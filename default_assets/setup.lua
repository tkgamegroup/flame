function starts_with(s1, s2)
   return string.sub(s1, 1, string.len(s2))==s2
end

function dump(o)
   if type(o) == 'table' then
      local s = '{ '
      for k,v in pairs(o) do
         if type(k) ~= 'number' then k = '"'..k..'"' end
         s = s .. '['..k..'] = ' .. dump(v) .. ','
      end
      return s .. '} '
   else
      return tostring(o)
   end
end

function find_enum(n)
	local enum = enums[n]
	if (enum == nil) then
		enum = enums["flame::"..n]
		if (enum == nil) then
			return nil
		end
	end
	return enum
end

function find_udt(n)
	local udt = udts[n]
	if (udt == nil) then
		udt = udts["flame::"..n]
		if (udt == nil) then
			return nil
		end
	end
	return udt
end

callbacks = {}

function get_callback_slot(f)
	local s = math.random(0, 10000)
	if (callbacks[s] == nil) then
		callbacks[s] = { f=f }
		return s
	end
	return get_callback_slot(f)
end

function make_obj(o, n)
	local udt = nil
	if type(n) == "string" then
		udt = find_udt(n)
	else
		udt = n
	end
	if (udt == nil) then
		print("script: cannot find udt "..n)
		return
	end
	if udt.base ~= "" then
		make_obj(o, udt.base)
	end
	if not o.p then return end
	for k, vi in pairs(udt.variables) do
		local ti = types[vi.type_name]
		local v = flame_get(o.p, vi.offset, ti.tag, ti.basic, ti.vec_size, ti.col_size)
		if ti.is_object_type then
			local vv = { p=v }
			make_obj(vv, ti.name)
			v = vv
		end
		o[k] = v
	end
	for k, fi in pairs(udt.functions) do
		local ti = types[fi.ret_type_name]
		if ti.is_object_type then
			o[k] = function(...)
				__type__ = ti.name
				local ret = {}
				ret.p = flame_call(o.p, fi.f, {...})
				make_obj(ret, __type__)
				return ret
			end
		else
			o[k] = function(...)
				return flame_call(o.p, fi.f, {...})
			end
		end
	end
	for k, fi in pairs(udt.callbacks) do
		o[k] = function(f, ...)
			n = get_callback_slot(f)
			flame_call(o.p, fi, { 0, n, ... })
			callbacks[n] = nil
		end
	end
	for k, fi in pairs(udt.listeners) do
		o["add_"..k] = function(f, ...)
			n = get_callback_slot(f)
			callbacks[n].c = flame_call(o.p, fi.add, { 0, n, ... })
			return n
		end
		o["remove_"..k] = function(n, ...)
			flame_call(o.p, fi.remove, { callbacks[n].c, ...})
			callbacks[n] = nil
			return n
		end
	end
end

function array_find(arr, val)
	for _, v in ipairs(arr) do
        if v == val then
            return true
        end
    end
    return false
end

function vec2(x, y)
	if type(x) == "table" then
		y = x.y
		x = x.x
	elseif y == nil then
		y = x
	end
	local o = { x=x, y=y }
	o.push = function()
		return  o.x, o.y
	end
	setmetatable(o, {
		__unm = function(a)
			return vec2(-a.x, -a.y)
		end,
		__add = function(a, b)
			return vec2(a.x + b.x, a.y + b.y)
		end,
		__sub = function(a, b)
			return vec2(a.x - b.x, a.y - b.y)
		end,
		__mul = function(a, b)
			if type(b) == "table" then
				return vec2(a.x * b.x, a.y * b.y)
			end
			return vec2(a.x * b, a.y * b)
		end
	})
	return o
end

function vec3(x, y, z)
	if type(x) == "table" then
		y = x.y
		z = x.z
		x = x.x
	elseif y == nil and z == nil then
		y = x
		z = x
	end
	local o = { x=x, y=y, z=z }
	o.push = function()
		return  o.x, o.y, o.z
	end
	o.to_flat = function()
		return  vec2(o.x, o.z)
	end
	setmetatable(o, {
		__unm = function(a)
			return vec3(-a.x, -a.y, -a.z)
		end,
		__add = function(a, b)
			return vec3(a.x + b.x, a.y + b.y, a.z + b.z)
		end,
		__sub = function(a, b)
			return vec3(a.x - b.x, a.y - b.y, a.z - b.z)
		end,
		__mul = function(a, b)
			if type(b) == "table" then
				return vec3(a.x * b.x, a.y * b.y, a.z * b.z)
			end
			return vec3(a.x * b, a.y * b, a.z * b)
		end
	})
	return o
end

function vec4(x, y, z, w)
	if type(x) == "table" then
		y = x.y
		z = x.z
		w = x.w
		x = x.x
	elseif y == nil and z == nil and w == nil then
		y = x
		z = x
		w = x
	end
	local o = { x=x, y=y, z=z, w=w }
	o.push = function()
		return  o.x, o.y, o.z, o.w
	end
	setmetatable(o, {
		__unm = function(a)
			return vec4(-a.x, -a.y, -a.z, -a.w)
		end,
		__add = function(a, b)
			return vec4(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w)
		end,
		__sub = function(a, b)
			return vec4(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w)
		end,
		__mul = function(a, b)
			if type(b) == "table" then
				return vec4(a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w)
			end
			return vec4(a.x * b, a.y * b, a.z * b, a.w * b)
		end
	})
	return o
end

function AABB(a, b)
	local o = {
		a=a, b=b
	}
	o.get_points = function(dst)
		flame_set_v3(dst, 0 * 12, vec3(o.a.x, o.a.y, o.a.z))
		flame_set_v3(dst, 1 * 12, vec3(o.b.x, o.a.y, o.a.z))
		flame_set_v3(dst, 2 * 12, vec3(o.b.x, o.a.y, o.b.z))
		flame_set_v3(dst, 3 * 12, vec3(o.a.x, o.a.y, o.b.z))
		flame_set_v3(dst, 4 * 12, vec3(o.a.x, o.b.y, o.a.z))
		flame_set_v3(dst, 5 * 12, vec3(o.b.x, o.b.y, o.a.z))
		flame_set_v3(dst, 6 * 12, vec3(o.b.x, o.b.y, o.b.z))
		flame_set_v3(dst, 7 * 12, vec3(o.a.x, o.b.y, o.b.z))
	end
	return o
end

function mat2x3(a1, a2, a3, b1, b2, b3)
	local o = {
		a1=a1, a2=a2, a3=a3,
		b1=b1, b2=b2, b3=b3
	}
	o.push = function()
		return
			o.a1, o.a2, o.a3,
			o.b1, o.b2, o.b3
	end
	return o
end

function mat4(a1, a2, a3, a4, b1, b2, b3, b4, c1, c2, c3, c4, d1, d2, d3, d4)
	local o = {
		a1=a1, a2=a2, a3=a3, a4=a4,
		b1=b1, b2=b2, b3=b3, b4=b4,
		c1=c1, c2=c2, c3=c3, c4=c4,
		d1=d1, d2=d2, d3=d3, d4=d4
	}
	o.push = function()
		return  
			o.a1, o.a2, o.a3, o.a4,
			o.b1, o.b2, o.b3, o.b4,
			o.c1, o.c2, o.c3, o.c4,
			o.d1, o.d2, o.d3, o.d4
	end
	return o
end

function hexahedron_draw_lines(dst, points, col)
	flame_cpy(dst, 32 * 0 + 16 * 0 + 0, points, 0, 12)
	flame_cpy(dst, 32 * 0 + 16 * 0 + 12, col, 0, 4)
	flame_cpy(dst, 32 * 0 + 16 * 1 + 0, points, 12, 12)
	flame_cpy(dst, 32 * 0 + 16 * 1 + 12, col, 0, 4)

	flame_cpy(dst, 32 * 1 + 16 * 0 + 0, points, 12, 12)
	flame_cpy(dst, 32 * 1 + 16 * 0 + 12, col, 0, 4)
	flame_cpy(dst, 32 * 1 + 16 * 1 + 0, points, 24, 12)
	flame_cpy(dst, 32 * 1 + 16 * 1 + 12, col, 0, 4)

	flame_cpy(dst, 32 * 2 + 16 * 0 + 0, points, 24, 12)
	flame_cpy(dst, 32 * 2 + 16 * 0 + 12, col, 0, 4)
	flame_cpy(dst, 32 * 2 + 16 * 1 + 0, points, 36, 12)
	flame_cpy(dst, 32 * 2 + 16 * 1 + 12, col, 0, 4)

	flame_cpy(dst, 32 * 3 + 16 * 0 + 0, points, 36, 12)
	flame_cpy(dst, 32 * 3 + 16 * 0 + 12, col, 0, 4)
	flame_cpy(dst, 32 * 3 + 16 * 1 + 0, points, 0, 12)
	flame_cpy(dst, 32 * 3 + 16 * 1 + 12, col, 0, 4)

	flame_cpy(dst, 32 * 4 + 16 * 0 + 0, points, 48, 12)
	flame_cpy(dst, 32 * 4 + 16 * 0 + 12, col, 0, 4)
	flame_cpy(dst, 32 * 4 + 16 * 1 + 0, points, 60, 12)
	flame_cpy(dst, 32 * 4 + 16 * 1 + 12, col, 0, 4)

	flame_cpy(dst, 32 * 5 + 16 * 0 + 0, points, 60, 12)
	flame_cpy(dst, 32 * 5 + 16 * 0 + 12, col, 0, 4)
	flame_cpy(dst, 32 * 5 + 16 * 1 + 0, points, 72, 12)
	flame_cpy(dst, 32 * 5 + 16 * 1 + 12, col, 0, 4)

	flame_cpy(dst, 32 * 6 + 16 * 0 + 0, points, 72, 12)
	flame_cpy(dst, 32 * 6 + 16 * 0 + 12, col, 0, 4)
	flame_cpy(dst, 32 * 6 + 16 * 1 + 0, points, 84, 12)
	flame_cpy(dst, 32 * 6 + 16 * 1 + 12, col, 0, 4)

	flame_cpy(dst, 32 * 7 + 16 * 0 + 0, points, 84, 12)
	flame_cpy(dst, 32 * 7 + 16 * 0 + 12, col, 0, 4)
	flame_cpy(dst, 32 * 7 + 16 * 1 + 0, points, 48, 12)
	flame_cpy(dst, 32 * 7 + 16 * 1 + 12, col, 0, 4)

	flame_cpy(dst, 32 * 8 + 16 * 0 + 0, points, 0, 12)
	flame_cpy(dst, 32 * 8 + 16 * 0 + 12, col, 0, 4)
	flame_cpy(dst, 32 * 8 + 16 * 1 + 0, points, 48, 12)
	flame_cpy(dst, 32 * 8 + 16 * 1 + 12, col, 0, 4)

	flame_cpy(dst, 32 * 9 + 16 * 0 + 0, points, 12, 12)
	flame_cpy(dst, 32 * 9 + 16 * 0 + 12, col, 0, 4)
	flame_cpy(dst, 32 * 9 + 16 * 1 + 0, points, 60, 12)
	flame_cpy(dst, 32 * 9 + 16 * 1 + 12, col, 0, 4)

	flame_cpy(dst, 32 * 10 + 16 * 0 + 0, points, 24, 12)
	flame_cpy(dst, 32 * 10 + 16 * 0 + 12, col, 0, 4)
	flame_cpy(dst, 32 * 10 + 16 * 1 + 0, points, 72, 12)
	flame_cpy(dst, 32 * 10 + 16 * 1 + 12, col, 0, 4)

	flame_cpy(dst, 32 * 11 + 16 * 0 + 0, points, 36, 12)
	flame_cpy(dst, 32 * 11 + 16 * 0 + 12, col, 0, 4)
	flame_cpy(dst, 32 * 11 + 16 * 1 + 0, points, 84, 12)
	flame_cpy(dst, 32 * 11 + 16 * 1 + 12, col, 0, 4)
end

function length_2(v)
	return math.sqrt(v.x * v.x + v.y * v.y)
end

function distance_2(a, b)
	return length_2(a - b)
end

function normalize_2(v)
	local l = length_2(v)
	return vec2(v.x / l, v.y / l)
end

function dot_2(a, b)
	return a.x * b.x + a.y * b.y
end

function length_and_dir_2(v)
	local l = length_2(v)
	if l > 0 then
		return l, vec2(v.x / l, v.y / l)
	end
	return l, nil
end

function length_3(v)
	return math.sqrt(v.x * v.x + v.y * v.y + v.z * v.z)
end

function distance_3(a, b)
	return length_3(a - b)
end

function normalize_3(v)
	local l = length_3(v)
	return vec3(v.x / l, v.y / l, v.z / l)
end

function dot_3(a, b)
	return a.x * b.x + a.y * b.y + a.z * b.z
end

function length_and_dir_3(v)
	local l = length_3(v)
	if l > 0 then
		return l, vec3(v.x / l, v.y / l, v.z / l)
	end
	return l, nil
end

function rand2(min, max)
	return math.random() * (max - min) + min
end

function circle_rand(r)
	local rad = math.rad(math.random() * 360.0)
	return vec2(math.cos(rad) * r, math.sin(rad) * r)
end

function flame_get_i(p, o)
	return flame_get(p, o, e_type_data, e_integer_type, 1, 1)
end

function flame_get_f(p, o)
	return flame_get(p, o, e_type_data, e_floating_type, 1, 1)
end

function flame_get_p(p, o)
	return flame_get(p, o, e_type_pointer, e_else_type, 1, 1)
end

function flame_set_f(p, o, v)
	flame_set(p, o, e_type_data, e_floating_type, 1, 1, v)
end

function flame_set_c4(p, o, v)
	flame_set(p, o, e_type_data, e_char_type, 4, 1, v)
end

function flame_set_v3(p, o, v)
	flame_set(p, o, e_type_data, e_floating_type, 3, 1, v)
end

function flame_float_var(v)
	local ret = {}
	ret.p = flame_malloc(4)

	ret.get = function()
		return flame_get_f(ret.p, 0)
	end

	ret.set = function(v)
		flame_set_f(ret.p, 0, v)
	end

	ret.get_ = function()
		local v = ret.get()
		flame_free(ret.p)
		return v
	end
	
	if v then ret.set(v) end
	
	return ret
end

function split_by_newline(str)
	local lines = {}
	for s in str:gmatch("[^\r\n]+") do
		table.insert(lines, s)
	end
	return lines
end

function load_ini(fn)
	local file = flame_load_file(fn)
	local data = {}
	local section
	local lines = split_by_newline(file)
	for i, line in ipairs(lines) do
		local temp_section = line:match('^%[([%w]+)%]$')
		if temp_section then
			section = temp_section
			data[section] = {}
		else
			local param, value = line:match('^([%w|_]+)%s-=%s-(.+)$')
			if (param and value ~= nil) then
				if (tonumber(value)) then
					value = tonumber(value)
				elseif (value == 'true') then
					value = true
				elseif (value == 'false') then
					value = false
				end
				if (tonumber(param)) then
					param = tonumber(param)
				end
				data[section][param] = value
			end
		end
	end
	return data
end

function save_ini(fn, data)
	local contents = ""
	for section, param in pairs(data) do
		contents = contents .. ('[%s]\n'):format(section)
		for key, value in pairs(param) do
			contents = contents .. ('%s=%s\n'):format(key, tostring(value))
		end
		contents = contents .. '\n'
	end
	flame_save_file(fn, contents)
end
