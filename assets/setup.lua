function find_udt(n)
	local udt = udts[n];
	if (udt == nil) then
		udt = udts["flame::"..n];
		if (udt == nil) then
			print("script: cannot find udt "..n)
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
