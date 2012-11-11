class String
	alias _old_index index

	def index(x, pos = 0)
		if not x.is_a? String
			_old_index x, pos
		else
			bmh_index x, pos
		end
	end

	def bmh_index(x, pos = 0)
		n = length
		m = x.length

		# skip preprocessing if unnecessary
		if pos > n - m
			return nil
		end

		# FIXME: bad to use ASCII limit here.
		bad_char = Array.new 256, m

		i = 0
		x[0..-2].each_byte do |byte|
			bad_char[byte] = m - i - 1
			i += 1
		end

		while pos <= n - m
			c = self[pos + m - 1]
			if x[m - 1] == c and x == self[pos, m]
				return pos
			end
			pos += bad_char[c]
		end

		return nil
	end

	def kmp_index(x, pos = 0)
		n = length
		m = x.length
		off = Array.new m, 0

		i = 0
		j = -1
		off[0] = -1
		while i < m
			while j > -1 and x[i] != x[j]
				j = off[j]
			end
			i += 1
			j += 1
			if x[i] == x[j]
				off[i] = off[j]
			else
				off[i] = j
			end
		end

		i = 0
		j = pos
		while j < n
			while i > -1 and x[i] != self[j]
				i = off[i]
			end
			i += 1
			j += 1
			if i >= m
				return j - i
			end
		end

		return nil
	end

	def brute(x, pos = 0)
		m = x.length

		pos.upto length - m do |j|
			if x == self[j, m]
				return j
			end
		end

		return nil
	end

end

if __FILE__ == $0
	s = ""
	open "../test/searchspace" do |file|
		s = file.read
	end

	x = "GCAGAGAG"

	pos = 0
	i = 0
	time = Time.new
	while pos = s._old_index(x, pos)
		pos += 1
		i += 1
	end
	puts "found #{x} #{i} times in searchspace. time: #{Time.new - time}"
	
	pos = 0
	i = 0
	time = Time.new
	while pos = s.index(x, pos)
		pos += 1
		i += 1
	end
	puts "found #{x} #{i} times in searchspace. time: #{Time.new - time}"
end
