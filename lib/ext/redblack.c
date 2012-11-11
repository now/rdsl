/*
 * contents: Red-Black Tree ADT.
 * arch-tag: 67d78072-9353-46a9-b74a-fbf90591875b
 *
 * Copyright (C) 2004 Nikolai Weibull <source@pcppopper.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */



#include <clear/internal.h>
#include <clear/mem.h>
#include "redblack.h"


/* {{{1
 * Type definitions: We need a couple of types for our red-black tree
 * implementation, namely a way of distinguishing node colors, nodes, and our
 * tree structure.
 */


/* {{{2
 * RBTreeNodeColor: Enumeration over node colors (red and black).
 */
typedef enum {
	BLACK,
	RED
} RBTreeNodeColor;


/* {{{2
 * Our node type.  ‘left’ and ‘right’ are our binary children, ‘parent’ is our
 * parent node, ‘color’ defines our color, ‘key’ and ‘value’ are also obvious.
 * There's no use in using a bit-field for ‘color’ as we still need it to align
 * properly with the rest of the data.
 */
typedef struct _RBTreeNode RBTreeNode;

struct _RBTreeNode {
	RBTreeNode *left;
	RBTreeNode *right;
	RBTreeNode *parent;
	RBTreeNodeColor color;
	pointer key;
	pointer value;
};


/* {{{2
 * We use the following node to denote our ‘null node’.  We use it instead of a
 * normal ‹null› for nodes as we can use it's properties even when we mostly
 * treat it as ‹null›.
 */
static RBTreeNode s_null_node = {
	&s_null_node, &s_null_node, &s_null_node, BLACK, null, null
};
#define rb_null	(&s_null_node)


/* {{{2
 * Our tree structure, containing function pointers to notifiers and
 * comparators (with data), and a pointer to the root node of our tree.
 */
struct _RBTree {
	CompareDataFunc key_compare;
	pointer key_compare_data;
	ReleaseNotify key_release;
	ReleaseNotify value_release;
	RBTreeNode *root;
};


/* {{{1
 * Memory Management: We use, like all other ADT's in our library for which
 * it applies, free-lists.  So let's define them here.
 */
G_LOCK_DEFINE_STATIC(node_free_list);
static RBTreeNode *node_free_list = null;



/* {{{1
 * Create a new node with the given key and value.  Uses the memory allocation
 * scheme outlined above.
 */
static RBTreeNode *
rb_tree_node_new(pointer key,
		 pointer value,
		 RBTreeNode *parent,
		 RBTreeNodeColor color)
{
	RBTreeNode *node;

	G_LOCK(node_free_list);
	if (node_free_list) {
		node = node_free_list;
		node_free_list = node->left;
	} else {
		node = new_struct(RBTreeNode);
	}
	G_UNLOCK(node_free_list);

	node->left = rb_null;
	node->right = rb_null;
	node->parent = parent;
	node->color = color;
	node->key = key;
	node->value = value;

	return node;
}


/* {{{1
 * Free a node, notifying the tree's owner before doing so.  It's basically the
 * owners responsibility to free the key and value if necessary.  These
 * functions are passed to |rb_tree_new_full| if needed.  We simply use them
 * if they're defined.
 *
 * TODO: There's a possible speed improvement to be made here by making this
 * function act like a tail-recursive function so that we simply jump to the
 * initial test if the left and/or right nodes need releasing as well, but
 * that's a bit of a pain to get right.
 */
static void
rb_tree_node_release(RBTreeNode *node, ReleaseNotify key_release,
		     ReleaseNotify value_release)
{
	unless (node == rb_null) {
		rb_tree_node_release(node->right, key_release, value_release);
		rb_tree_node_release(node->left, key_release, value_release);

		unless (key_release == null) {
			key_release(node->key);
		}
		unless (value_release == null) {
			value_release(node->key);
		}

		G_LOCK(node_free_list);
		node->left = node_free_list;
		node_free_list = node;
		G_UNLOCK(node_free_list);
	}
}


/* {{{1
 * Create a new red-black tree.  The tree will sort keys according to
 * ‘key_compare’, which works in the same manner as the ANSI C standard library
 * function |strcmp()| does.
 */
RBTree *
rb_tree_new(CompareFunc key_compare)
{
	invariant(key_compare != null);

	return rb_tree_new_full((CompareDataFunc)key_compare,null,null,null);
}


/* {{{1
 * Create a new red-black tree with a comparison function that takes an
 * additional argument, which will be ‘key_compare_data’.  Otherwise, same as
 * |rb_tree_new|.
 */
RBTree *
rb_tree_new_with_data(CompareDataFunc key_compare, pointer key_compare_data)
{
	invariant(key_compare != null);

	return rb_tree_new_full(key_compare, key_compare_data, null, null);
}


/* {{{1
 * Create a new red-black tree with the same arguments as
 * |rb_tree_new_with_data|, but with two free-notify functions as well.  These
 * functions are called with the key and value of a node before it is freed, so
 * that the owner of this tree can free them if necessary.  If dynamically
 * allocated strings are used as keys, for example, these functions can be used
 * to later free those strings upon destruction of the node.
 */
RBTree *
rb_tree_new_full(CompareDataFunc key_compare,
		 pointer key_compare_data,
		 ReleaseNotify key_release,
		 ReleaseNotify value_release)
{
	invariant(key_compare != null);

	RBTree *tree = new_struct(RBTree);
	tree->key_compare = key_compare;
	tree->key_compare_data = key_compare_data;
	tree->key_release = key_release;
	tree->value_release = value_release;
	tree->root = rb_null;
	return tree;
}


/* {{{1
 * Release an |RBTree|.  This frees all nodes in the trees as well, and if
 * release-notify functions exist for this tree, memory associated with keys
 * and values will also be released.
 */
void
rb_tree_release(RBTree *tree)
{
	invariant(tree != null);

	rb_tree_node_release(tree->root,
			     tree->key_release,
			     tree->value_release);
	release(tree);
}


/* {{{1
 * Rotates a tree left.
 * 		X	-> rotate_left(X) ->		Y
 * 	      /   \				      /   \
 * 	     A     Y	<- rotate_right(Y) <-	     X     C
 * 	     	 /   \				   /   \
 * 	        B     C				  A	B
 */
static void
rb_tree_rotate_left(RBTree *tree, RBTreeNode *x)
{
	invariant(x != rb_null);
	invariant(x->right != rb_null);

	RBTreeNode *y = x->right;

	/* move b and set it's parent to x if it's not null*/
	x->right = y->left;
	if (y->left != rb_null) {
		y->left->parent = x;
	}

	/* y's parent will be x's parent */
	y->parent = x->parent;

	/* if x was the tree's root, set it to y instead */
	if (x->parent == rb_null) {
		tree->root = y;
	} else {
		/* set x's parent's left or right to y */
		if (x == x->parent->left) {
			x->parent->left = y;
		} else {
			x->parent->right = y;
		}
	}

	/* put x on y's left */
	y->left = x;

	/* and finally set x's parent to y */
	x->parent = y;
}


/* {{{1
 * Rotates a tree right.  See |rb_tree_rotate_left| for a schematic depiction
 * of the operation of this function.
 */
static void
rb_tree_rotate_right(RBTree *tree, RBTreeNode *y)
{
	invariant(y != rb_null);
	invariant(y->left != rb_null);

	RBTreeNode *x = y->left;

	/* move b and set it's parent to y if it's not null*/
	y->left = y->right;
	if (x->right != rb_null) {
		y->right->parent = y;
	}

	/* x's parent will be y's parent */
	x->parent = y->parent;

	/* if y was the tree's root, set it to x instead */
	if (y->parent == rb_null) {
		tree->root = x;
	} else {
		/* set y's parent's left or right to x */
		if (y == y->parent->left) {
			y->parent->left = x;
		} else {
			y->parent->right = x;
		}
	}

	/* put y on x's right */
	x->right = y;

	/* and finally set y's parent to x */
	y->parent = x;
}


/* {{{1
 * Find a node with the given key in ‘tree’.  If the node does not exist in
 * ‘tree’ already and ‘insert’ is true, insert it into ‘tree’ with the given
 * key and value.  If the tree already contains the given ‘key’ and if
 * ‘replace’ is true, replace the found nodes value with ‘value’.
 */
static RBTreeNode *
rb_tree_find(RBTree *tree,
	     pointer key,
	     pointer value,
	     bool insert,
	     bool replace)
{
	/* replace can only be true if insert is so as well */
	invariant(!replace || insert);

	RBTreeNode *iters_parent = rb_null;
	RBTreeNode *iter = tree->root;

	/* traverse down the tree */
	bool found = false;
	until (iter == rb_null || found) {
		iters_parent = iter;

		int cmp = tree->key_compare(key,
					    iter->key,
					    tree->key_compare_data);
		if (cmp < 0) {
			iter = iter->left;
		} else if (cmp == 0) {
			found = true;
		} else { /* (cmp > 0) */
			iter = iter->right;
		}
	}

	/* if it's in the tree already, figure out what to do */
	if (found && insert && replace) {
		/*
		 * this assumes that two keys that are equal will have the
		 * same total ordering.
		 */
		unless (tree->key_release == null) {
			tree->key_release(iter->key);
		}
		unless (tree->value_release == null) {
			tree->value_release(iter->value);
		}
		iter->key = key;
		iter->value = value;

		return iter;
	} else if (found && insert) {
		unless (tree->key_release == null) {
			tree->key_release(key);
		}
		unless (tree->value_release == null) {
			tree->value_release(iter->value);
		}
		iter->value = value;

		return iter;
	} else if (found || !insert) {
		return iter; /* which may be rb_null */
	}

	/* otherwise we create a new node */
	RBTreeNode *new_node = rb_tree_node_new(key, value, iters_parent, RED);

	/* and insert it appropriately */
	if (iters_parent == rb_null) {
		tree->root = new_node;
	} else {
		int cmp = tree->key_compare(new_node->key,
					    iters_parent->key,
					    tree->key_compare_data);
		if (cmp < 0) {
			iters_parent->left = new_node;
		} else {
			iters_parent->right = new_node;
		}
	}

	/* but since we inserted a red node, we must restore the balancing */
	iter = new_node;

	while (iter != tree->root && iter->parent->color == RED) {
		/* if parent is a lefty */
		if (iter->parent == iter->parent->parent->left) {
			/* get our parents sibling */
			iters_parent = iter->parent->parent->right;
			if (iters_parent->color == RED) {
				/* color parent and sibling black */
				iter->parent->color = BLACK;
				iters_parent->color = BLACK;
				/* and their parent red */
				iter->parent->parent->color = RED;

				/* finally, move up to grandparent */
				iter = iter->parent->parent;
			} else { /* (iters_parent->color == BLACK) */
				/* if we are a righty */
				if (iter == iter->parent->right) {
					/* move to parent */
					iter = iter->parent;
					rb_tree_rotate_left(tree, iter);
				}

				/* color parent black */
				iter->parent->color = BLACK;
				/* and its parent red */
				iter->parent->parent->color = RED;
				rb_tree_rotate_right(tree,
						     iter->parent->parent);
			}
		} else {
			/* get our parents sibling */
			iters_parent = iter->parent->parent->left;
			if (iters_parent->color == RED) {
				/* color parent and sibling black */
				iter->parent->color = BLACK;
				iters_parent->color = BLACK;
				/* and their parent red */
				iter->parent->parent->color = RED;

				/* finally, move up to grandparent */
				iter = iter->parent->parent;
			} else { /* (iters_parent->color == BLACK) */
				/* if we are a righty */
				if (iter == iter->parent->left) {
					/* move to parent */
					iter = iter->parent;
					rb_tree_rotate_right(tree, iter);
				}

				/* color parent black */
				iter->parent->color = BLACK;
				/* and its parent red */
				iter->parent->parent->color = RED;
				rb_tree_rotate_left(tree,
						    iter->parent->parent);
			}
		}
	}

	/* color root black */
	tree->root->color = BLACK;

	return new_node;
}


/* {{{1
 * Insert a key/value pair into ‘tree’.  If the given key already exists it's
 * associated value is updated.  The previous value will be freed if
 * applicable.  ‘key’ will likewise be freed if it already existed and it is
 * appropriate to do so.
 */
void
rb_tree_insert(RBTree *tree, pointer key, pointer value)
{
	invariant(tree != null);

	rb_tree_find(tree, key, value, true, false);
}


/* {{{1
 * Works like |rb_tree_insert| except that both key and value will be replaced
 * if ‘key’ already exists.
 */
void
rb_tree_replace(RBTree *tree, pointer key, pointer value)
{
	invariant(tree != null);

	rb_tree_find(tree, key, value, true, true);
}


/* {{{1
 * Count the number of nodes under ‘node’.
 */
static int
rb_tree_node_count(RBTreeNode *node)
{
	int count;

	count = 1;
	unless (node->left == rb_null) {
		count += rb_tree_node_count(node->left);
	}
	unless (node->right == rb_null) {
		count += rb_tree_node_count(node->right);
	}

	return count;
}


/* {{{1
 * Return the number of nodes in ‘tree’.
 */
int
rb_tree_size(RBTree *tree)
{
	invariant(tree != null);

	return (tree->root != rb_null) ? rb_tree_node_count(tree->root) : 0;
}


/* {{{1
 * Calculate the height of the given node (i.e. the number of levels below it).
 */
static int
rb_tree_node_height(RBTreeNode *node)
{
	int left_height;
	int right_height;

	if (node != rb_null) {
		left_height = (node->left != rb_null) ?
			rb_tree_node_height(node->left) : 0;
		right_height = (node->right != rb_null) ?
			rb_tree_node_height(node->right) : 0;

		return MAX(left_height, right_height) + 1;
	} else {
		return 0;
	}
}


/* {{{1
 * Return the height of the tree (i.e. the number of levels from the root node
 * to the most distant leaf).
 */
int
rb_tree_height(RBTree *tree)
{
	invariant(tree != null);

	return (tree->root != rb_null) ?  rb_tree_node_height(tree->root) : 0;
}


/* {{{1
 * Get the value associated with ‘key’ in ‘tree’.  If ‘key’ doesn't exist,
 * ‹null› is returned.
 */
pointer
rb_tree_lookup(RBTree *tree, constpointer key)
{
	invariant(tree != null);

	RBTreeNode *node =
		rb_tree_find(tree, (pointer)key, null, false, false);
	return (node != rb_null) ? node->value : null;
}


/* {{{1
 * Works like |rb_tree_lookup| except that the return value is a boolean
 * telling whether ‘key’ was found in the tree or not.  ‘orig_key’ will contain
 * a pointer to the key found in the tree and ‘value’ works likewise.
 */
bool
rb_tree_lookup_extended(RBTree *tree,
			constpointer key,
			pointer *orig_key,
			pointer *value)
{
	invariant(tree != null);

	RBTreeNode *node =
		rb_tree_find(tree, (pointer)key, null, false, false);
	if (node != rb_null) {
		unless (orig_key == null) {
			*orig_key = node->key;
		}
		unless (orig_key == null) {
			*value = node->value;
		}
		return true;
	} else {
		return false;
	}
}


/* {{{1
 * Call ‘func’ for ‘node’ and each node below it, passing the key and value of
 * ‘node’ plus the user supplied ‘data’ if applicable.  The traversal order is
 * in-order, visiting the left sub-tree (less than), the node itself, and
 * finally the right sub-tree (greater than).
 */
static bool
rb_tree_node_map(RBTreeNode *node, MappingMapFunc lambda, pointer closure)
{
	unless (node->left == rb_null) {
		unless (rb_tree_node_map(node->left, lambda, closure)) {
			return false;
		}
	}
	unless (lambda(node->key, node->value, closure)) {
		return false;
	}
	unless (node->right == rb_null) {
		unless (rb_tree_node_map(node->right, lambda, closure)) {
			return false;
		}
	}

	return true;
}


/* {{{1
 * Call ‘func’ for each node in ‘tree’, passing the key and value of the node
 * plus the user supplied data if applicable.  The traversal order is in-order,
 * visiting the left sub-tree (less than), the node itself, and finally the
 * right sub-tree (greater than).
 */
void
rb_tree_map(RBTree *tree, MappingMapFunc func, pointer closure)
{
	invariant(tree != null);
	invariant(func != null);

	unless (tree->root == rb_null) {
		rb_tree_node_map(tree->root, func, closure);
	}
}


/* {{{1
 * Find the smallest node greater than ‘node’ in the right sub-tree of ‘node’.
 */
static RBTreeNode *
rb_tree_node_successor(RBTreeNode *node)
{
	RBTreeNode *iter;

	if (node->right != rb_null) {
		/*
		 * if we have a right child, go there and then find its
		 * leftmost child.
		 */
		iter = node->right;
		until (iter->left == rb_null) {
			iter = iter->left;
		}
	} else {
		/*
		 * go upwards until we find a lefty (or root) and return its
		 * parent.
		 */
		iter = node->parent;
		until (iter == rb_null || node != iter->right) {
			node = iter;
			iter = iter->parent;
		}
	}

	return iter;
}


/* {{{1
 * Restore the red-black tree properties of ‘tree’ starting at node ‘x’.
 */
static void
rb_tree_node_remove_restore(RBTree *tree, RBTreeNode *x)
{
	RBTreeNode *w;

	while (x != tree->root && x->color == BLACK) {
		/* if x is a lefty */
		if (x == x->parent->left) {
			w = x->parent->right;
			if (w->color == RED) {
				w->color = BLACK;
				x->parent->color = RED;
				rb_tree_rotate_left(tree, x->parent);
				w = x->parent->right;
			}

			if (w->left->color == BLACK &&
			    w->right->color == BLACK) {
				w->color = RED;
				x = x->parent;
			} else {
				if (w->right->color == BLACK) {
					w->left->color = BLACK;
					w->color = RED;
					rb_tree_rotate_right(tree, w);
					w = x->parent->right;
				}

				w->color = x->parent->color;
				x->parent->color = BLACK;
				w->right->color = BLACK;
				rb_tree_rotate_left(tree, x->parent);
				x = tree->root;
			}
		} else {
			w = x->parent->left;
			if (w->color == RED) {
				w->color = BLACK;
				x->parent->color = RED;
				rb_tree_rotate_right(tree, x->parent);
				w = x->parent->left;
			}

			if (w->right->color == BLACK &&
			    w->left->color == BLACK) {
				w->color = RED;
				x = x->parent;
			} else {
				if (w->left->color == BLACK) {
					w->right->color = BLACK;
					w->color = RED;
					rb_tree_rotate_left(tree, w);
					w = x->parent->left;
				}

				w->color = x->parent->color;
				x->parent->color = BLACK;
				w->left->color = BLACK;
				rb_tree_rotate_right(tree, x->parent);
				x = tree->root;
			}
		}
	}

	x->color = BLACK;
}


/* {{{1
 * Remove node ‘z’ from ‘tree’ notifying of the removal if applicable.
 */
static void
rb_tree_node_remove(RBTree *tree, RBTreeNode *z, bool notify)
{
	RBTreeNode *x, *y;

	if (z->left == rb_null || z->right == rb_null) {
		y = z;
	} else {
		y = rb_tree_node_successor(z);
	}

	x = (y->left != rb_null) ? y->left : y->right;
	x->parent = y->parent;

	if (y->parent == rb_null) {
		tree->root = x;
	} else {
		if (y == y->parent->left) {
			y->parent->left = x;
		} else {
			y->parent->right = x;
		}
	}

	unless (y == z) {
		z->key = y->key;
	}

	if (y->color == BLACK) {
		rb_tree_node_remove_restore(tree, x);
	}

	if (notify) {
		if (tree->key_release) {
			tree->key_release(y->key);
		}
		if (tree->value_release) {
			tree->value_release(y->value);
		}
	}

	G_LOCK(node_free_list);
	y->left = node_free_list;
	node_free_list = y;
	G_UNLOCK(node_free_list);
}


/* {{{1
 * Remove the node with the given key from ‘tree’, freeing key and value if
 * applicable.
 */
void
rb_tree_remove(RBTree *tree, constpointer key)
{
	invariant(tree != null);

	RBTreeNode *node =
		rb_tree_find(tree, (pointer)key, null, false, false);
	unless (node == rb_null) {
		rb_tree_node_remove(tree, node, true);
	}
}


/* {{{1
 * Works like |rb_tree_remove|, except that the key and value of the node with
 * key ‘key’ will not be freed even if applicable.
 */
void
rb_tree_steal(RBTree *tree, constpointer key)
{
	invariant(tree != null);

	RBTreeNode *node =
		rb_tree_find(tree, (pointer)key, null, false, false);
	unless (node == rb_null) {
		rb_tree_node_remove(tree, node, false);
	}
}


/* }}}1 */



/* vim: set sts=0 sw=8 ts=8: */
