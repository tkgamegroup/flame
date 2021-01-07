function find_udt(n)
	local udt = udts[n]
	if (udt == nil) then
		udt = udts["flame::"..n]
		if (udt == nil) then
			print("script: cannot find udt "..n)
			return nil
		end
	end
	return udt
end

callbacks = {}

function get_callback_slot(f)
	local s = math.random(0, 10000)
	if (callbacks[s] == nil) then
		callbacks[s] = {}
		callbacks[s].f = f
		return s
	end
	return get_callback_slot(f)
end

function make_obj(o, n)
	local udt = find_udt(n)
	if (udt == nil) then
		return
	end
	for k, v in pairs(udt.functions) do
		o[k] = function(self, ...)
			return flame_call({...}, o.p, v)
		end
	end
	for k, v in pairs(udt.callback_interfaces) do
		o["add_"..k] = function(self, f, ...)
			n = get_callback_slot(f)
			parms = {...}
			parms[#parms + 1] = 0
			parms[#parms + 1] = n
			callbacks[n].c = flame_call(parms, o.p, v.add)
			return n
		end
		o["remove_"..k] = function(self, n, ...)
			parms = {...}
			parms[#parms + 1] = callbacks[n].c
			parms[#parms + 1] = n
			flame_call(parms, o.p, v.remove)
			callbacks[n] = nil
			return n
		end
	end
end
