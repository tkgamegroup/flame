function make_obj(o, tbl)
	for k, v in pairs(tbl) do
		o[k] = function(self, ...)
			return flame_call(..., o.p, v)
		end
	end
end
