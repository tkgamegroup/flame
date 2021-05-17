function menu_file_open()
    local d = create_entity("prefabs/input_dialog")
    local l = create_entity("prefabs/layer")
              
    d.find_driver("dWindow").set_title("Open")
    local input_dialog = d.find_driver("dInputDialog")
    input_dialog.set_text(last_open)
    input_dialog.add_callback(function(ok, text)
        if ok then
            if load_scene(text) then 
                last_open = text
            end
        end
        l.get_parent().remove_child(l)
    end)
    l.add_child(d)
    ui.add_child(l)
end

function menu_file_save_as()
    local d = create_entity("prefabs/input_dialog")
    local l = create_entity("prefabs/layer")
              
    d.find_driver("dWindow").set_title("Save As")
    local input_dialog = d.find_driver("dInputDialog")
    input_dialog.set_text(last_save)
    input_dialog.add_callback(function(ok, text)
        if ok then 
            if save_scene(text) then
            last_save = text
            end
        end
        l.get_parent().remove_child(l)
    end)
    l.add_child(d)
    ui.add_child(l)
end

function menu_shading_solid()
    s_renderer.set_shade_wireframe(false)
    entity.find_driver("dMenuItem").set_single_checked()
end

function menu_shading_wireframe()
    s_renderer.set_shade_wireframe(true)
    entity.find_driver("dMenuItem").set_single_checked()
end

function menu_show_physics_visualization()
    local menu_item = entity.find_driver("dMenuItem")
    local checked = menu_item.get_checked()
    checked = not checked
    menu_item.set_checked(checked)
    s_physics.set_visualization(checked)
end

function menu_show_ui_reflector()
    local menu_item = entity.find_driver("dMenuItem")
    local checked = menu_item.get_checked()
    checked = not checked
    menu_item.set_checked(checked)
    if checked then
        if not ui_reflector.p then
            ui_reflector = create_entity("ui_reflector")
            ui.add_child(ui_reflector)
        end
    else
        if ui_reflector.p then
            ui_reflector.get_parent().remove_child(ui_reflector)
            ui_reflector.p = nil
        end
    end
end

function menu_show_global_axes()
    local menu_item = entity.find_driver("dMenuItem")
    local checked = menu_item.get_checked()
    checked = not checked
    menu_item.set_checked(checked)
    scene.find_child("hud_global_axes").set_visible(checked)
end

function menu_show_crosshair()
    local menu_item = entity.find_driver("dMenuItem")
    local checked = menu_item.get_checked()
    checked = not checked
    menu_item.set_checked(checked)
    scene.find_child("hud_crosshair").set_visible(checked)
end

function menu_tools_scatter_vegetations()
    local e_terrain = scene.find_child("terrain")
    if not e_terrain.p then return end

    local node = e_terrain.find_component("cNode")
    local terrain = e_terrain.find_component("cTerrain")
              
    local blocks = terrain.get_blocks()
    local scale = node.get_scale()
    local num = 10000

    local height_texture = terrain.get_height_texture()
              
    local e_grass_root = create_entity("prefabs/node")
    e_grass_root.set_name("grass_root")
    
    for x=0,--[[blocks.x-1]]0,1 do
        for y=0,--[[blocks.y-1]]0,1 do
            local ptr_uvs = malloc_vec2(num)
            local ptr_samples = malloc_vec4(num)
            
            local uv_off = vec2(x / blocks.x, y / blocks.y)
            for i=0,num-1,1 do
                local uv = vec2(math.random() / blocks.x, math.random() / blocks.y)
                uv = uv_off + uv
                set_vec2(ptr_uvs, i, uv)
            end

            height_texture.get_samples(num, ptr_uvs, ptr_samples)
              
            for i=0,num-1,1 do
                local uv = get_vec2(ptr_uvs, i)
                local sample = get_vec4(ptr_samples, i)
                local e = create_entity("D:\\assets\\grass\\02_d.prefab")
                local node = e.find_component("cNode")
                local pos = vec3(uv.x * blocks.x * scale.x, sample.x * scale.y, uv.y * blocks.y * scale.z)
                node.set_pos(pos)
                e_grass_root.add_child(e)
            end
              
            flame_free(ptr_uvs)
            flame_free(ptr_samples)
        end
    end

    e_terrain.get_parent().add_child(e_grass_root)
end

function menu_settings_alwawys_update()
    local menu_item = entity.find_driver("dMenuItem")
    local checked = menu_item.get_checked()
    checked = not checked
    menu_item.set_checked(checked)
end
