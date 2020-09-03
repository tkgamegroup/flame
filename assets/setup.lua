function find_udt(n)
	local udt = udts[n];
	if (udt == nil) then
		udt = udts["flame::"..n];
		if (udt == nil) then
			if not n == "flame::Component" then
				print("script: cannot find udt "..n)
			end
			return nil
		end
	end
	return udt
end

function make_obj(o, n)
	local udt = find_udt(n)
	if (udt == nil) then
		return
	end
	for k, v in pairs(udt) do
		o[k] = function(self, ...)
			return flame_call({...}, o.p, v)
		end
	end
end

slots = {}

function get_slot(o)
	local s = math.random(0, 10000)
	if (slots[s] == nil) then
		slots[s] = o
		return s
	end
	return get_slot(o)
end

function release_slot(n)
	slots[n] = nil
end
