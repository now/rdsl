require 'association'

module RDSL

# The Sorted Association module assumes that the including class has an +each+
# method which takes as an optional argument the order of the traversal.
module SortedAssociation
	include Association

	# Specifies traversal with the key-value pairs in an ascending order.
	ASCENDING = 0

	# Specifies traversal with the key-value pairs in a descending order.
	DESCENDING = 1

	# Executes the block once for each key-value pair.  The keys are given
	# in the order specified by _order_.
	def each_pair(order = ASCENDING)
		each true, true, order do |key, value| yield key, value end
	end

	# Executes the block once for each key.  The keys are given in the
	# order specified by _order_.
	def each_key(order = ASCENDING)
		each true, false, order do |key| yield key end
	end

	# Executes the block once for each value.  The values are given in the
	# order specified by _order_, relative to the keys.
	def each_value(order = ASCENDING)
		each false, true, order do |value| yield value end
	end

	# Returns +true+ if _key_ is present in the Association.
	def key?(key)
		each_key do |k|
			break if k > key
			return true if k == key
		end
		return false
	end

	# Returns the key-value pair in the SortedAssociation with the
	# maximium key.
	def max
		each_pair DESCENDING do |pair| return pair end
	end

	# Returns the key-value pair in the SortedAssociation with the minimum
	# key.
	def min
		each_pair ASCENDING do |pair| return pair end
	end
end

end
