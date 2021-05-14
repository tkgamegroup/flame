inspector = entity
            
function update_inspector()
    local count = inspector.get_children_count()
    if count > 0 then
        for i=0, count-1, 1 do
            inspector.remove_child(inspector.get_child(0))
        end
    end
                
    if not selected.p then return end
                  
    local e, ens, evs, en, ev
                  
    function add_group(name)
        e = create_entity("attribute_group")
        e.find_driver("dTreeNode").set_title(name)
        inspector.add_child(e)
        ens = e.find_child("names") 
        evs = e.find_child("values")
    end
                  
    function add_text_attribute(name, value)
        en = create_entity("prefabs/text")
        en.find_component("cText").set_text(name)
        ens.add_child(en)
        ev = create_entity("prefabs/text")
        ev.find_component("cText").set_text(value)
        evs.add_child(ev)
    end
                  
    function add_check_attribute(name, value, callback)
        en = create_entity("prefabs/text")
        en.find_component("cText").set_text(name)
        ens.add_child(en)
        ev = create_entity("prefabs/checkbox")
        local checkbox = ev.find_driver("dCheckbox")
        checkbox.set_checked(value)
        if callback then
            ev.add_driver_data_listener(function(h)
                if h == flame_hash("checked") then
                    callback(checkbox.get_checked())
                end
            end, checkbox)
        end
        evs.add_child(ev)
    end

    local target = selected
                  
    add_group("entity")
    add_text_attribute("Name: ", target.get_name())
    add_text_attribute("Srcs: ", target.get_srcs())
    add_check_attribute("Visible: ", target.get_visible(), function(c)
        target.set_visible(c)
    end)
                  
    local components = {}
    target.get_components(function(cmp)
        table.insert(components, cmp)
    end)
    for i=1, #components, 1 do
        local comp = components[i]
        local name = comp.type_name

        add_group(name)

        local udt = find_udt(name)
                  
        local attrs = {}
	    for k, attr in pairs(udt.attributes) do
            local temp = {}
            for kk, vv in pairs(attr) do
                temp[kk] = vv
            end
            temp.name = k
            table.insert(attrs, temp)
        end
        table.sort(attrs, function(a, b)
            return a.get.index < b.get.index
        end)
                  
        for j=1, #attrs, 1 do
        local attr = attrs[j]
        local v = flame_call(comp.p, attr.get.f, {})
        local t = type(v)
        if t ~= "nil" then
            if t == "table" then
                if v["x"] ~= nil and v["y"] ~= nil then
                    if v["z"] ~= nil then
                        if v["w"] ~= nil then
                            add_text_attribute(attr.name..": ", v.x..", "..v.y..", "..v.z..", "..v.w)
                        else
                            add_text_attribute(attr.name..": ", v.x..", "..v.y..", "..v.z)
                        end
                    else
                        add_text_attribute(attr.name..": ", v.x..", "..v.y)
                    end
                end
            elseif t == "boolean" then
                add_check_attribute(attr.name..": ", v, function(c)
                    flame_call(comp.p, attr.set.f, { c })
                end)
            else
                add_text_attribute(attr.name..": ", v)
            end
        end
        end
    end
end
