local e_tag = find_enum("TypeTag")
e_type_data = e_tag["Data"]
e_type_pointer = e_tag["Pointer"]
local e_basic = find_enum("BasicType")
e_boolean_type = e_basic["BooleanType"]
e_integer_type = e_basic["IntegerType"]
e_floating_type = e_basic["FloatingType"]
e_char_type = e_basic["CharType"]
e_else_type = e_basic["ElseType"]

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
				table.insert(udt.attributes, { name=n, get=fi, set=func2 })
			end
        end
	end
    table.sort(udt.attributes, function(a, b)
        return a.get.index < b.get.index
    end)
end
