local e_type_data = find_enum("TypeTag")["Data"]
local e_floating_type = find_enum("BasicType")["FloatingType"]

function malloc_float(n)
	return flame_malloc(4 * n)
end

function malloc_vec2(n)
	return flame_malloc(8 * n)
end

function malloc_vec3(n)
	return flame_malloc(12 * n)
end

function malloc_vec4(n)
	return flame_malloc(16 * n)
end

function set_float(p, i, v)
    flame_set(p, 4 * i, e_type_data, e_floating_type, 1, 1, v)
end

function set_vec2(p, i, v)
    flame_set(p, 8 * i, e_type_data, e_floating_type, 2, 1, v)
end

function set_vec3(p, i, v)
    flame_set(p, 12 * i, e_type_data, e_floating_type, 3, 1, v)
end

function set_vec4(p, i, v)
    flame_set(p, 16 * i, e_type_data, e_floating_type, 4, 1, v)
end

function get_float(p, i)
    return flame_get(p, 4 * i, e_type_data, e_floating_type, 1, 1)
end

function get_vec2(p, i)
    return flame_get(p, 8 * i, e_type_data, e_floating_type, 2, 1)
end

function get_vec3(p, i)
    return flame_get(p, 12 * i, e_type_data, e_floating_type, 3, 1)
end

function get_vec4(p, i)
    return flame_get(p, 16 * i, e_type_data, e_floating_type, 4, 1)
end

for k, udt in pairs(udts) do
	udt.static_functions = {}
	for k, fi in pairs(udt.functions) do
		if fi.static then
			local ti = types[fi.ret_type_name]
			if ti.is_object_type then
				udt.static_functions[k] = function(...)
					__type__ = ti.name
					local ret = {}
					ret.p = flame_call(nil, fi.f, {...})
					make_obj(ret, __type__)
					return ret
				end
			else
				udt.static_functions[k] = function(...)
					return flame_call(nil, fi.f, {...})
				end
			end
		end
	end
	udt.attributes = {}
	for k, fi in pairs(udt.functions) do
		if starts_with(k, "get_") then
			local n = string.sub(k, 5)
			local func2 = udt.functions["set_"..n]
            if func2 ~= nil then
				udt.attributes[n] = { get=fi, set=func2 }
			end
        end
	end
end
