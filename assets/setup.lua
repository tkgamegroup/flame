function make_obj(o, n)
	local tbl = udts[n];
	if (tbl == nil) then
		tbl = udts["flame::"..n];
		if (tbl == nil) then
			print("script: cannot find udt "..n)
			return
		end
	end
	for k, v in pairs(tbl) do
		o[k] = function(self, ...)
			return flame_call({...}, o.p, v)
		end
	end
end
