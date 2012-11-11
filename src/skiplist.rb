require 'sortedassociation'

module RDSL

class Skiplist
	include SortedAssociation

	# The default value for non-existent keys.
	attr_accessor :default

	# The number of key-value pairs; same value returned by Treap#length.
	attr_reader :size

	# Creates a +Skiplist+.  A default value may also be specified.
	def initialize(default = nil)
		@default, @size = default, 0

		@head = HeadNode.new self
		@tail = Node.new nil, nil, -1
		@tail.backward = @tail.pred = @head
		@head.forward[0] = @tail
	end

	# Creates a +Skiplist+. Each successive pair is treated as a key-value
	# pair.
	def Skiplist.[](*ary)
		raise ArgumentError, "odd number args to Treap" if
						ary.size % 2 != 0
		skiplist = Skiplist.new
		0.step(ary.size - 2, 2) do |i|
			skiplist.store ary[i], ary[i + 1]
		end
		return skiplist
	end

	# Associates _key_ with _value_.  The value of this expression is
	# _value_.
	def store(key, value)
		node = search_impl key
		if key != node.key or node == @tail
			# we are inserting a new node
			new_node = Node.new key, value, random_height
			node = node.pred
			new_node.pred = node
			node.forward[0].pred = new_node
			tmp = nil
			0.upto new_node.height do |i|
				node = node.backward while i > node.height

				tmp = node.forward[i]
				if node.height == MAX_HEIGHT and tmp.height < 0
					# at @head and @tail
					node.true_height = i + 1
					node.forward[i + 1] = tmp
				end

				new_node.forward[i] = tmp
				node.forward[i] = new_node

				tmp.backward = new_node if tmp.height == i
			end

			new_node.backward = node
			@size += 1
		else
			# simply update the stored value
			node.value = value
		end
		return value
	end

	# Returns the value associated with _key_.  If the _key_ isn't present
	# in the Skiplist, the value of the block is returned.  If no block is
	# specified and _ifnone_ is, _ifnone_ is returned.  Else an IndexError
	# is raised.
	def fetch(key, ifnone = nil)
		node = search_impl key
		return node.value if node != @tail
		if block_given?
			yield key
		elsif ifnone != nil
			return ifnone
		else
			raise IndexError, "key not found"
		end
	end

	# Deletes all key-value pairs stored in the Skiplist.
	def clear
		@head.true_height = 0
		@head.forward[0] = @tail
		@tail.backward = @tail.pred = @head
		return self
	end

	# Deletes a key-value pair with key _key_.  The stored value is
	# returned.
	def delete(key)
		rm_node = search_impl key
		return nil if rm_node == @tail
		node = rm_node.backward
		tmp = nil
		rm_node.height.downto 0 do |i|
			node = node.forward[i] while node.forward[i] != rm_node
			tmp = rm_node.forward[i]
			node.forward = tmp
			tmp.backward = node if tmp.height == i
		end
		tmp.pred = node
		@size -= 1
		return rm_node.value
	end

	# Executes the block once for each key-value pair.  Pairs are given in
	# the order specified by _order_, relative to the keys.
	def each(with_key = true, with_value = true, order = ASCENDING)
		# FIXME: make this prettier
		if order == ASCENDING
			node = @head.forward[0]
			while node != @tail
				info = []
				info << node.key if with_key
				info << node.value if with_value
				yield info
				node = node.forward[0]
			end
		else
			node = @tail.pred
			while node != @head
				info = []
				info << node.key if with_key
				info << node.value if with_value
				yield info
				node = node.pred
			end
		end
	end

	# Returns the number of key-value pairs in the Skiplist.
	def length
		@size
	end

	# Returns a string representation of the Skiplist.
	def to_s
		str = ""
		each do |key, value|
			str << "[ #{key}: #{value} ]->"
		end
		str[0..-3]
	end

private

	OPTIMAL_PROBABILITY 	= 0.25
	MAX_HEIGHT		= 32

	def search_impl(key)
		h = @head.true_height
		prev = @head
		node = @head.forward[h]

		return node if node.height >= 0 && key == node.key

		h -= 1
		@tail.key = key

		while h >= 0
			node = prev.forward[h]
			prev, node = node, node.forward[h] while key > node.key
			break if key == node.key && node != @tail
			h -= 1
		end

		return node
	end

	def random_height
		height = 0
		while rand < OPTIMAL_PROBABILITY && height < MAX_HEIGHT
			height += 1
		end
		return height
	end

	class Node
		def initialize(key, value, height)
			@key, @value, @height = key, value, height
			@pred = @backward = nil
			@forward = Array.new @height + 1
		end

		attr_accessor :key, :value, :height, :pred, :backward, :forward
	end

	class HeadNode < Node
		def initialize(list)
			super nil, nil, MAX_HEIGHT
			@list = list
			@true_height = 0
		end

		attr_accessor :true_height
	end
end

end

if __FILE__ == $0 or defined? RDSL::DEBUG
	require 'test/unit'

	class SkiplistTest < Test::Unit::TestCase
		def set_up
			@list = RDSL::Skiplist[1935, "Elvis Presley",
						1926, "Chuck Berry",
						1941, "Bob Dylan",
						1936, "Roy Orbison",
						1915, "Muddy Waters"]
		end

		def test_create
			assert_not_nil @list,
					"Skiplist#[] didn't create a Skiplist"
		end
		
		def test_size
			assert_equal 5, @list.size
		end

		def test_length_and_size
			assert_equal @list.size, @list.length
		end

		def test_max_and_min
			assert_equal 1941, @list.max.first
			assert_equal 1915, @list.min.first
		end

		def test_fetch
			assert_equal "Elvis Presley", @list.fetch(1935)
			assert_equal @list.fetch(1935), @list[1935]
		end
	end
end
