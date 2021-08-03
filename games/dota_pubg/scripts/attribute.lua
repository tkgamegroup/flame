function new_attribute(b)
	local a = {
		b = b and b or 0,
		a = 0,
		p = 100,
		t = 0
	}

	a.calc = function()
		a.t = (a.b + a.a)
		if a.p then
			a.t = a.t * (1.0 + a.p / 100.0)
		end
	end

	return a
end
