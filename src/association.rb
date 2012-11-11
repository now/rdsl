require 'container'

module RDSL

# The Association module assumes that the including class has an +each+ method,
# a +fetch+ method, a +store+, and a +delete+ method.  Their interfaces should
# be the same as for Ruby Hashes.
module Association
	include Container

	# Returns the value associated with _key_.
	def [](key)
		# TODO: document necessity of default.
		fetch key do |k| default end
	end

	# Associates key with value.
	def []=(key, value)
		store key, value
	end

	# Executes the block once for each key-value pair.
	def each_pair
		each do |key, value| yield key, value end
	end

	# Executes the block once for each key.
	def each_key
		each true, false do |key| yield key end
	end

	# Executes the block once for each value.
	def each_value
		each false, true do |value| yield value end
	end

	# Returns the key for _value_, or +nil+ if it isn't present.
	def index(value)
		each do |k, v|
			return k if v == value
		end
		return nil
	end

	# Returns an array of values associated with the specified keys.
	def indices(*keys)
		values = []
		keys.each do |key| values << fetch(key) end
		return values
	end

	# Returns a Association containing the current values as keys and the
	# keys as values.  If more than one key has the same value, an
	# arbitrary key is chosen.
	def invert
		assoc = self.class.new
		each do |key, value| assoc.store value, key end
		return assoc
	end

	# Returns +true+ if _key_ is present in the Association.
	def key?(key)
		each_key do |k| return true if k == key end
		return false
	end

	# Returns an array of all keys in the Association.
	def keys
		keys = []
		each_key do |key| keys << key end
		return keys
	end

	# Deletes key-value pairs where the value of block is +true+.  Returns
	# a copy with the above deletions made to it.
	def reject(&block)
		# FIXME: we need a deep copy for this. damn.
		assoc = self.clone
		assoc.reject! do |key, value| block.call key, value end
	end

	# Deletes key-value pairs where the value of block is +true+.
	def reject!
		rejects = []
		each do |key, value|
			rejects << key if yield key, value
		end
		rejects.each do |key| delete key end
		return self
	end

	# Replaces the contents of the Association with that of _assoc_.
	def replace(assoc)
		clear
		update assoc
	end

	# Removes a key-value pair and returns it.
	# FIXME: may be rather useless.
	def shift
		return if empty?
		key = nil
		value = nil
		each do |k, v|
			key, value = k, v
			break
		end
		delete key
		return [key, value]
	end

	# Returns the Association itself.  Every object that has a
	# Association#to_assoc method is treated as if it is an association by
	# Association#replace and Association#update.
	def to_assoc
		self
	end

	# Updates the Association with the contents of the specified _assoc_.
	# If duplicate keys exist, the associated value of _assoc_ takes
	# precedence and overwrites that of the receiving Association.
	def update(assoc)
		assoc.to_assoc.each do |k, v| store k, v end
		return self
	end

	# Returns +true+ if value is present in the Container.
	def value?(value)
		each_value do |v| return true if v == value end
		return false
	end

	# Returns an array of all values in the Container.
	def values
		values = []
		each_value do |v| values << v end
		return values
	end

	# See Association#value?
	alias has_value? value? 

	# See Association#key?
	alias has_key? key?

	# Ree Association#key?
	alias include? key?

	# See Treap#indices.
	alias indexes indices

	# See Association#key?.
	alias member? key?
end

end
