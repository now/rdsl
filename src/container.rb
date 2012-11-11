module RDSL

# The Container module assumes that the including class has an +each+ and a
# +length+ method.
# You can add the following methods to a class that provides +each+, by just
# including this module.
module Container
	include Enumerable

	# Returns +true+ if the Container is empty.
	def empty?
		length == 0
	end
end

end
