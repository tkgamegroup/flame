inspector = entity

local last_target = nil
function update_inspector()
    if last_target then
        if selected.p == last_target.e.p then return end
        last_target.e.remove_message_listener(last_target.message_listener)
        for i = 1, #last_target.data_listeners, 1 do
            local lis = last_target.data_listeners[i]
            last_target.e.remove_component_data_listener(lis.p, lis.comp)
        end
    end

    inspector.remove_all_children()
                
    if not selected.p then 
        last_target = nil
        return 
    end

    local target = selected

    last_target = {}
    last_target.e = selected

    local message_callbacks = {}
    last_target.message_listener = target.add_message_listener(function(msg)
        
    end)

    last_target.data_listeners = {}
                  
    local ens, evs
                  
    function add_group(name)
        local e = create_entity("attribute_group")
        e.find_driver("dTreeNode").set_title(name)
        inspector.add_child(e)
        ens = e.find_child("names") 
        evs = e.find_child("values")
    end

    function add_name(name)
        local en = create_entity("prefabs/text")
        en.find_component("cText").set_text(name)
        ens.add_child(en)
    end
                  
    function add_text_attribute(name, value)
        add_name(name)

        local ev = create_entity("prefabs/text")
        ev.find_component("cText").set_text(value)
        evs.add_child(ev)
    end
                  
    function add_bool_attribute(name, getter, setter, listen_changes)
        add_name(name)

        local ev = create_entity("prefabs/checkbox")
        local checkbox = ev.find_driver("dCheckbox")
        checkbox.set_checked(getter())

        local tracker = {}
        tracker.changing = false

        local callback = nil
        if listen_changes then
            local hash = flame_hash(name)
            callback = function(h)
                if h == hash then
                    tracker.changing = true
                    checkbox.set_checked(getter())
                    tracker.changing = false
                    return true
                end
                return false
            end
        end

        local hash_checked = flame_hash("checked")
        ev.add_driver_data_listener(function(h)
            if h == hash_checked then
                if not tracker.changing then
                    setter(checkbox.get_checked())
                end
            end
        end, checkbox.p)

        evs.add_child(ev)

        return callback
    end

    function add_float_attribute(name, getter, setter, listen_changes)
        add_name(name)
        
        local ev = create_entity("prefabs/drag_edit")
        local drag_edit = ev.find_driver("dDragEdit")
        drag_edit.set_float(getter())

        local tracker = {}
        tracker.changing = false

        local callback = nil
        if listen_changes then
            local hash = flame_hash(name)
            callback = function(h)
                if h == hash then
                    tracker.changing = true
                    drag_edit.set_float(getter())
                    tracker.changing = false
                end
                return false
            end
        end

        local hash_value = flame_hash("value")
        ev.add_driver_data_listener(function(h)
            if h == hash_value then
                if not tracker.changing then
                    setter(drag_edit.get_float())
                end
            end
        end, drag_edit.p)

        evs.add_child(ev)

        return callback
    end

    function add_vec2_attribute(name, getter, setter, listen_changes)
        add_name(name)
        
        local ev = create_entity("prefabs/drag_edit_2")
        local ev1 = ev.get_child(0)
        local ev2 = ev.get_child(1)
        local drag_edit_1 = ev1.find_driver("dDragEdit")
        local drag_edit_2 = ev2.find_driver("dDragEdit")
        local v = getter()
        drag_edit_1.set_float(v.x)
        drag_edit_2.set_float(v.y)

        local tracker = {}
        tracker.changing = false

        local callback = nil
        if listen_changes then
            local hash = flame_hash(name)
            callback = function(h)
                if h == hash then
                    tracker.changing = true
                    local v = getter()
                    drag_edit_1.set_float(v.x)
                    drag_edit_2.set_float(v.y)
                    tracker.changing = false
                end
                return false
            end
        end

        local hash_value = flame_hash("value")
        ev1.add_driver_data_listener(function(h)
            if h == hash_value then
                if not tracker.changing then
                    setter(vec2(drag_edit_1.get_float(), drag_edit_2.get_float()))
                end
            end
        end, drag_edit_1.p)
        ev2.add_driver_data_listener(function(h)
            if h == hash_value then
                if not tracker.changing then
                    setter(vec2(drag_edit_1.get_float(), drag_edit_2.get_float()))
                end
            end
        end, drag_edit_2.p)

        evs.add_child(ev)

        return callback
    end

    function add_vec3_attribute(name, getter, setter, listen_changes)
        add_name(name)
        
        local ev = create_entity("prefabs/drag_edit_3")
        local ev1 = ev.get_child(0)
        local ev2 = ev.get_child(1)
        local ev3 = ev.get_child(2)
        local drag_edit_1 = ev1.find_driver("dDragEdit")
        local drag_edit_2 = ev2.find_driver("dDragEdit")
        local drag_edit_3 = ev3.find_driver("dDragEdit")
        local v = getter()
        drag_edit_1.set_float(v.x)
        drag_edit_2.set_float(v.y)
        drag_edit_3.set_float(v.z)

        local tracker = {}
        tracker.changing = false

        local callback = nil
        if listen_changes then
            local hash = flame_hash(name)
            callback = function(h)
                if h == hash then
                    tracker.changing = true
                    local v = getter()
                    drag_edit_1.set_float(v.x)
                    drag_edit_2.set_float(v.y)
                    drag_edit_3.set_float(v.z)
                    tracker.changing = false
                end
                return false
            end
        end

        local hash_value = flame_hash("value")
        ev1.add_driver_data_listener(function(h)
            if h == hash_value then
                if not tracker.changing then
                    setter(vec3(drag_edit_1.get_float(), drag_edit_2.get_float(), drag_edit_3.get_float()))
                end
            end
        end, drag_edit_1.p)
        ev2.add_driver_data_listener(function(h)
            if h == hash_value then
                if not tracker.changing then
                    setter(vec3(drag_edit_1.get_float(), drag_edit_2.get_float(), drag_edit_3.get_float()))
                end
            end
        end, drag_edit_2.p)
        ev3.add_driver_data_listener(function(h)
            if h == hash_value then
                if not tracker.changing then
                    setter(vec3(drag_edit_1.get_float(), drag_edit_2.get_float(), drag_edit_3.get_float()))
                end
            end
        end, drag_edit_3.p)

        evs.add_child(ev)

        return callback
    end

    function add_vec4_attribute(name, getter, setter, listen_changes)
        add_name(name)
        
        local ev = create_entity("prefabs/drag_edit_4")
        local ev1 = ev.get_child(0)
        local ev2 = ev.get_child(1)
        local ev3 = ev.get_child(2)
        local ev4 = ev.get_child(3)
        local drag_edit_1 = ev1.find_driver("dDragEdit")
        local drag_edit_2 = ev2.find_driver("dDragEdit")
        local drag_edit_3 = ev3.find_driver("dDragEdit")
        local drag_edit_4 = ev4.find_driver("dDragEdit")
        local v = getter()
        drag_edit_1.set_float(v.x)
        drag_edit_2.set_float(v.y)
        drag_edit_3.set_float(v.z)
        drag_edit_4.set_float(v.w)

        local tracker = {}
        tracker.changing = false
        
        local callback = nil
        if listen_changes then
            local hash = flame_hash(name)
            callback = function(h)
                if h == hash then
                    tracker.changing = true
                    local v = getter()
                    drag_edit_1.set_float(v.x)
                    drag_edit_2.set_float(v.y)
                    drag_edit_3.set_float(v.z)
                    drag_edit_4.set_float(v.w)
                    tracker.changing = false
                end
                return false
            end
        end

        local hash_value = flame_hash("value")
        ev1.add_driver_data_listener(function(h)
            if h == hash_value then
                if not tracker.changing then
                    setter(vec4(drag_edit_1.get_float(), drag_edit_2.get_float(), drag_edit_3.get_float(), drag_edit_4.get_float()))
                end
            end
        end, drag_edit_1.p)
        ev2.add_driver_data_listener(function(h)
            if h == hash_value then
                if not tracker.changing then
                    setter(vec4(drag_edit_1.get_float(), drag_edit_2.get_float(), drag_edit_3.get_float(), drag_edit_4.get_float()))
                end
            end
        end, drag_edit_2.p)
        ev3.add_driver_data_listener(function(h)
            if h == hash_value then
                if not tracker.changing then
                    setter(vec4(drag_edit_1.get_float(), drag_edit_2.get_float(), drag_edit_3.get_float(), drag_edit_4.get_float()))
                end
            end
        end, drag_edit_3.p)
        ev4.add_driver_data_listener(function(h)
            if h == hash_value then
                if not tracker.changing then
                    setter(vec4(drag_edit_1.get_float(), drag_edit_2.get_float(), drag_edit_3.get_float(), drag_edit_4.get_float()))
                end
            end
        end, drag_edit_4.p)

        evs.add_child(ev)

        return callback
    end
                  
    add_group("entity")
    add_text_attribute("Name: ", target.get_name())
    add_text_attribute("Srcs: ", target.get_srcs())
    add_bool_attribute("Visible: ", target.get_visible, target.set_visible)
                  
    local components = {}
    target.get_components(function(comp)
        table.insert(components, comp)
    end)
    for i = 1, #components, 1 do
        local comp = components[i]
        local name = comp.type_name

        add_group(name)

        local udt = find_udt(name)

        local data_callbacks = {}
        table.insert(last_target.data_listeners, { p = target.add_component_data_listener(function(h)
            for i = 1, #data_callbacks, 1 do
                if data_callbacks[i](h) then return end
            end
        end, comp.p), comp = comp.p })
                  
        for j = 1, #udt.attributes, 1 do
            local attr = udt.attributes[j]

            local type = types[attr.get.ret_type_name]

            if type.basic == e_boolean_type then
                table.insert(data_callbacks, add_bool_attribute(attr.name, function()
                    return flame_call(comp.p, attr.get.f, {})
                end,
                function(c)
                    flame_call(comp.p, attr.set.f, { c })
                end, true))
            elseif type.basic == e_floating_type then
                if type.vec_size == 1 then
                    table.insert(data_callbacks, add_float_attribute(attr.name, function()
                        return flame_call(comp.p, attr.get.f, {})
                    end,
                    function(c)
                        flame_call(comp.p, attr.set.f, { c })
                    end, true))
                elseif type.vec_size == 2 then
                    table.insert(data_callbacks, add_vec2_attribute(attr.name, function()
                        return flame_call(comp.p, attr.get.f, {})
                    end,
                    function(c)
                        flame_call(comp.p, attr.set.f, { c })
                    end, true))
                elseif type.vec_size == 3 then
                    table.insert(data_callbacks, add_vec3_attribute(attr.name, function()
                        return flame_call(comp.p, attr.get.f, {})
                    end,
                    function(c)
                        flame_call(comp.p, attr.set.f, { c })
                    end, true))
                elseif type.vec_size == 4 then
                    table.insert(data_callbacks, add_vec4_attribute(attr.name, function()
                        return flame_call(comp.p, attr.get.f, {})
                    end,
                    function(c)
                        flame_call(comp.p, attr.set.f, { c })
                    end, true))
                end
            end
        end
    end
end
