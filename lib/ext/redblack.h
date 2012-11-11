/*
 * contents: Red-Black Tree ADT.
 * arch-tag: 6aecc304-4a22-44cc-ad04-b9d02df9b72f
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



#ifndef REDBLACK_H
#define REDBLACK_H


typedef struct _RBTree RBTree;


RBTree *rb_tree_new(CompareFunc key_compare);
RBTree *rb_tree_new_with_data(CompareDataFunc key_compare,
			      pointer key_compare_data);
RBTree *rb_tree_new_full(CompareDataFunc key_compare,
			 pointer key_compare_data,
			 ReleaseNotify key_release,
			 ReleaseNotify value_release);
void rb_tree_release(RBTree *tree);

void rb_tree_insert(RBTree *tree, pointer key, pointer value);
void rb_tree_replace(RBTree *tree, pointer key, pointer value);
int rb_tree_size(RBTree *tree);
int rb_tree_height(RBTree *tree);
pointer rb_tree_lookup(RBTree *tree, constpointer key);
bool rb_tree_lookup_extended(RBTree *tree,
			     constpointer key,
			     pointer *orig_key,
			     pointer *value);
void rb_tree_map(RBTree *tree, MappingMapFunc lambda, pointer closure);
void rb_tree_remove(RBTree *tree, constpointer key);
void rb_tree_steal(RBTree *tree, constpointer key);


#endif /* REDBLACK_H */



/* vim: set sts=0 sw=8 ts=8: */
