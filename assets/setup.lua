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
	for k, v in pairs(udt.normal_functions) do
		o[k] = function(self, ...)
			return flame_call({...}, o.p, v)
		end
	end
	for k, v in pairs(udt.type_needed_functions) do
		o[k] = function(self, ...)
			__type__ = v.type
			local ret = {}
			ret.p = flame_call({...}, o.p, v.func)
			make_obj(ret, __type__)
			return ret
		end
	end
	for k, v in pairs(udt.callback_interfaces) do
		o["add_"..k] = function(self, f, ...)
			n = get_callback_slot(f)
			callbacks[n].c = flame_call({ 0, n, ... }, o.p, v.add)
			return n
		end
		o["remove_"..k] = function(self, n, ...)
			flame_call({ callbacks[n].c, ...}, o.p, v.remove)
			callbacks[n] = nil
			return n
		end
	end
end

function v2_neg(a)
	return { 
		x = -a.x, 
		y = -a.y
	}
end

function v2_add(a, b)
	return { 
		x = a.x + b.x, 
		y = a.y + b.y
	}
end

function v2_sub(a, b)
	return { 
		x = a.x - b.x, 
		y = a.y - b.y
	}
end

function v2_mul(a, b)
	if type(b) == "table" then
		return { 
			x = a.x * b.x, 
			y = a.y * b.x
		}
	end
	return { 
		x = a.x * b, 
		y = a.y * b
	}
end

function v3_neg(a)
	return { 
		x = -a.x, 
		y = -a.y,
		z = -a.z
	}
end

function v3_add(a, b)
	return { 
		x = a.x + b.x, 
		y = a.y + b.y,
		z = a.z + b.z
	}
end

function v3_sub(a, b)
	return { 
		x = a.x - b.x, 
		y = a.y - b.y,
		z = a.z - b.z
	}
end

function v3_mul(a, b)
	if type(b) == "table" then
		return { 
			x = a.x * b.x, 
			y = a.y * b.x,
			z = a.z * b.z
		}
	end
	return { 
		x = a.x * b, 
		y = a.y * b,
		z = a.z * b
	}
end

function v3_distance(a, b)
	local x = a.x - b.x
	local y = a.y - b.y
	local z = a.z - b.z
	return math.sqrt(x * x + y * y + z * z)
end
