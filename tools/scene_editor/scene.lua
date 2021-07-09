scene = entity

prefab = { p=nil }
selected = { p=nil }
            
function load_scene(filename)
    if prefab.p then 
        scene.remove_child(prefab) 
    end

    local ok
    prefab, ok = create_entity(filename)
    scene.add_child(prefab, 0)

    if prefab.find_component("cNode").p and prefab.find_child("camera").p == nil then
        local e = create_entity("prefabs/camera")
        e.find_component("cNode").set_pos(vec3(0, 0, 5))
        e.find_component("cCamera").set_current(true)
        scene.add_child(e)
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
