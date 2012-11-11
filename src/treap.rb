require 'sortedassociation'

module RDSL

# The Treap class represents a Treap ADT.  For a full description of Treaps,
# please refer to http://www.nada.kth.se/~snilsson/public/papers/treap/.  The
# interface is stringently kept the same as that of Ruby Hashes.  This means
# that a Treap can be used as a drop in for a Hash when the data needs to be in
# a sorted order.
class Treap
	include SortedAssociation

	# The default value for non-existent keys.
	attr_accessor :default

	# The number of key-value pairs; same value returned by Treap#length.
	attr_reader :size

	# Creates a +Treap+.  A default value may also be specified.
	def initialize(default = nil)
		@tree, @default, @size = nil, default, 0
	end

	#def clone
	#	treap = Treap.new
	#	each do |pair| treap.store pair end
	#	return pair
	#end

	# Creates a +Treap+. Each successive pair is treated as a key-value
	# pair.
	def Treap.[](*ary)
		raise ArgumentError, "odd number args to Treap" if
						ary.size % 2 != 0
		treap = Treap.new
		0.step(ary.size - 2, 2) do |i|
			treap.store ary[i], ary[i + 1]
		end
		return treap
	end

	# Associates _key_ with _value_.  The value of this expression is
	# _value_.
	def store(key, value)
		int_max = 2 ** (1.size * 8) # OMG!
		@tree = insert @tree, Node.new(key, value, rand(int_max))
		@size += 1		# OMG: check it out
		return value
	end

	# Returns the value associated with _key_.  If the _key_ isn't present
	# in the Treap, the value of the block is returned.  If no block is
	# specified and _ifnone_ is, _ifnone_ is returned.  Else an IndexError
	# is raised.
	def fetch(key, ifnone = nil)
		node = @tree
		# TODO: this can be made into a each-block
		while node != nil
			return node.value if key == node.key
			node = key < node.key ? node.left : node.right
		end
		if block_given?
			yield key
		elsif ifnone != nil
			return ifnone
		else
			raise IndexError, "key not found"
		end
	end

	# Deletes all key-value pairs stored in the Treap.
	def clear
		@tree = nil
		@size = 0
		return self
	end

	# Deletes a key-value pair with key _key_.  The stored value is
	# returned.
	def delete(key)
		old, @tree = remove @tree, key
		return old
	end

	# Executes the block once for each key-value pair.  Pairs are given in
	# the order specified by _order_, relative to the keys.
	def each(with_key = true, with_value = true, order = ASCENDING)
		stack = []
		node = @tree
		build_order = (order == ASCENDING) ? :left : :right
		stack_order = (order == ASCENDING) ? :right : :left

		while node != nil
			stack << node
			node = node.send build_order
		end

		while !stack.empty?
			node = stack.last
			if node.send(stack_order) == nil
				tmp = stack.pop
				while !stack.empty? and
				      stack.last.send(stack_order) == tmp
					tmp = stack.pop
				end
			else
				tmp = node.send stack_order
				while tmp != nil
					stack << tmp
					tmp = tmp.send build_order
				end
			end
			
			values = []
			values << node.key if with_key
			values << node.value if with_value
			yield values	# XXX: faster with *values ???
		end
	end

	# Returns the number of key-value pairs in the Treap.
	def length
		@size
	end

	# Returns a string representation of the Treap.
	def to_s
		@tree.to_s
	end

private

	# Represents a node in a tree.  Can probably be useful outside of the
	# Treap class; only time will tell.
	class Node
		def initialize(key, value, priority)
			@key, @value, @priority = key, value, priority
			@right, @left = nil, nil
		end

		# Rotate the tree to the right.  Returns the rotated tree.
		def rotate_right
			tmp, @left, tmp.right = @left, @left.right, self
			return tmp
		end

		# Rotate the tree to the left.  Returns the rotated tree.
		def rotate_left
			tmp, @right, tmp.left = @right, @right.left, self
			return tmp
		end

		# Delete the root node of the tree.  Returns the updated tree.
		def pop_root
			return @right if @left == nil
			return @left if @right == nil
			tmp = nil
			if @left.priority >= @right.priority
				tmp = rotate_left
				tmp.left = pop_root
			else
				tmp = rotate_right
				tmp.right = pop_root
			end
			return tmp
		end

		def to_s
			to_s_impl 0
		end

		attr_accessor :key, :value, :left, :right
		attr_reader :priority

		def to_s_impl(level)
			str = ""
			str << @right.to_s_impl(level + 1) if @right != nil
			str << " " * level
			str << "#{@key} = #{@value} (#{@priority})\n"
			str << @left.to_s_impl(level + 1) if @left != nil
			return str
		end
	end

	def insert(tree, node)
		return node if tree == nil
		if node.key < tree.key
			tree.left = insert(tree.left, node)
			if tree.priority > tree.left.priority
				tree = tree.rotate_right
			end
		elsif node.key > tree.key
			tree.right = insert(tree.right, node)
			if tree.priority > tree.right.priority
				tree = tree.rotate_left
			end
		else
			tree.value = node.value
			@size -= 1	# OMG: should this be forbidden?!
		end
		return tree
	end

	def remove(tree, key)
		return [nil, nil] if tree == nil
		old = nil
		if key < tree.key
			old, tree.left = remove(tree.left, key)
		elsif key > tree.key
			old, tree.right = remove(tree.right, key)
		else
			old = tree.value
			tree = tree.pop_root
			@size -= 1
		end
		return [old, tree]
	end
end

end # module

if __FILE__ == $0 or defined? RDSL::DEBUG
	# TODO: how do we share this with RDSL::Skiplist?
	require 'test/unit'

	class TreapTest < Test::Unit::TestCase
		def set_up
			@treap = RDSL::Treap[1935, "Elvis Presley",
						1926, "Chuck Berry",
						1941, "Bob Dylan",
						1936, "Roy Orbison",
						1915, "Muddy Waters"]
		end

		def test_create
			assert_not_nil @treap, "Treap#[] didn't create a Treap"
		end
		
		def test_size
			assert_equal 5, @treap.size
		end

		def test_length_and_size
			assert_equal @treap.size, @treap.length
		end

		def test_max_and_min
			assert_equal 1941, @treap.max.first
			assert_equal 1915, @treap.min.first
		end

		def test_fetch
			assert_equal "Elvis Presley", @treap.fetch(1935)
			assert_equal @treap.fetch(1935), @treap[1935]
		end
	end

	time = Time.now
	treap = RDSL::Treap.new
	s = "a"
	10000.downto 0 do |i|
		treap[i] = s
		s = s.succ
	end
	puts "10000 insertions took #{Time.now - time}"
end
