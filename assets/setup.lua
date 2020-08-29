function make_obj(o, n)
	local tbl = udts[n];
	if (tbl == nil) then
		tbl = udts["flame::"..n];
		if (tbl == nil) then
			if not n == "flame::Component" then
				print("script: cannot find udt "..n)
			end
			return
		end
	end
	for k, v in pairs(tbl) do
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
