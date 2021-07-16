scene = entity

prefab = { p=nil }
selected = { p=nil }

local e_light = find_enum("LightType")
local e_directional = e_light["Directional"]
function load_scene(filename)
    if prefab.p then
        if prefab.wrapper then
            scene.remove_child(prefab.wrapper)
        else
            scene.remove_child(prefab)
        end
    end

    local ok
    prefab, ok = create_entity(filename)

    if prefab.find_component("cNode").p and prefab.find_child("camera").p == nil then
        prefab.wrapper = create_entity("prefabs/node")
        local e = create_entity("prefabs/cube")
        local node = e.find_component("cNode")
        node.set_pos(vec3(0, -1, 0))
        node.set_scale(vec3(10, 0.1, 10))
        e.find_component("cRigid").set_dynamic(false)
        prefab.wrapper.add_child(e)
        
        --[[
        local e = create_entity("prefabs/light")
        e.find_component("cNode").set_euler(vec3(0, 70, 0))
        local light = e.find_component("cLight")
        light.set_type(e_directional)
        light.set_color(vec3(1.7))
        light.set_cast_shadow(true)
        prefab.wrapper.add_child(e)
        ]]

        local e = create_entity("D:\\assets\\sky_test\\sky")
        prefab.wrapper.add_child(e)

        local e = create_entity("prefabs/camera")
        e.find_component("cNode").set_pos(vec3(0, 0, 5))
        e.find_component("cCamera").set_current(true)
        prefab.wrapper.add_child(e)
        make_third_camera(e)
        
        prefab.wrapper.add_child(prefab)
        scene.add_child(prefab.wrapper)
    else
        scene.add_child(prefab, 0)
    end
              
    update_hierachy()
    return ok
end
            
function save_scene(filename)
    if not prefab.p then return end

    prefab.save(filename)
end
      
function select(e, from_hierarchy)
    if selected.p then
        selected.set_state(find_enum("StateFlags")["None"])
    end
    selected = e
    if selected.p then
        selected.set_state(find_enum("StateFlags")["Selected"])
    end
    s_renderer.mark_dirty()
    if not from_hierarchy then
        local o = {}
        if selected.p then
            o = selected.get_userdata()
        else
            o = { p=nil }
        end
        local tree = hierarchy.find_driver("dTree")
        tree.set_selected(o)
        tree.expand_to_selected()
    end
    update_inspector()
end
            
local last_mpos = vec2(0, 0)
local receiver = scene.find_component("cReceiver")

receiver.add_mouse_left_down_listener(function(p)
    last_mpos.x = p.x
    last_mpos.y = p.y
end)

receiver.add_mouse_left_up_listener(function(p)
    if p.x == last_mpos.x and p.y == last_mpos.y then
        --[[ TODO
        local e = {}
        e.p = s_renderer.pickup(last_mpos)
        make_obj(e, "flame::Entity")
        select(e, false)
        ]]
    end
end)
