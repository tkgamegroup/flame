function cmd_file_open()
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

function cmd_file_save_as()
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

function cmd_shading_wireframe()
    s_renderer.set_shading(find_enum("ShadingType")["Wireframe"])
end

function cmd_shading_combined()
    s_renderer.set_shading(find_enum("ShadingType")["Combined"])
end

function cmd_shading_normal_data()
    s_renderer.set_shading(find_enum("ShadingType")["NormalData"])
end

function cmd_show_physics_visualization()
    local cmd_item = entity.find_driver("dMenuItem")
    local checked = cmd_item.get_checked()
    checked = not checked
    cmd_item.set_checked(checked)
    s_physics.set_visualization(checked)
end

function cmd_show_ui_reflector()
    local cmd_item = entity.find_driver("dMenuItem")
    local checked = cmd_item.get_checked()
    checked = not checked
    cmd_item.set_checked(checked)
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

function cmd_show_global_axes()
    local cmd_item = entity.find_driver("dMenuItem")
    local checked = cmd_item.get_checked()
    checked = not checked
    cmd_item.set_checked(checked)
    scene.find_child("hud_global_axes").set_visible(checked)
end

function cmd_show_crosshair()
    local cmd_item = entity.find_driver("dMenuItem")
    local checked = cmd_item.get_checked()
    checked = not checked
    cmd_item.set_checked(checked)
    scene.find_child("hud_crosshair").set_visible(checked)
end

function cmd_tools_scatter_vegetations()
    local e_terrain = scene.find_child("terrain")
    if not e_terrain.p then return end

    local node = e_terrain.find_component("cNode")
    local pos = node.get_pos();
    local scale = node.get_scale()

    local terrain = e_terrain.find_component("cTerrain")
    local blocks = terrain.get_blocks()
    local height_texture = terrain.get_height_texture()

    local num = 5000
              
    local e_grass_root = create_entity("prefabs/octree")
    e_grass_root.set_name("grass_root")
    e_grass_root.find_component("cNode").set_pos(pos + vec3(scale.x * 0.5, 0.0, scale.z * 0.5))
    local octree = e_grass_root.find_component("cOctree")
    octree.set_length(math.max(scale.x, scale.y))
    
    local ptr_uvs = malloc_vec2(num)
    local ptr_samples = malloc_vec4(num)
            
    for i=0,num-1,1 do
        local uv = vec2(math.random(), math.random())
        uv = uv
        set_vec2(ptr_uvs, i, uv)
    end

    height_texture.get_samples(num, ptr_uvs, ptr_samples)
              
    for i=0,num-1,1 do
        local uv = get_vec2(ptr_uvs, i)
        local sample = get_vec4(ptr_samples, i)
        local e = create_entity("D:\\assets\\grass\\02_d.prefab")
        local node = e.find_component("cNode")
        node.set_pos(vec3((uv.x - 0.5) * scale.x, sample.x * scale.y, (uv.y - 0.5) * scale.z))
        node.set_euler(vec3(math.random() * 360, 0, 0))
        node.set_scale(vec3(0.8 + math.random() * 0.4))
        e_grass_root.add_child(e)
    end
              
    flame_free(ptr_uvs)
    flame_free(ptr_samples)

    e_terrain.get_parent().add_child(e_grass_root)
end

function cmd_settings_alwawys_update()
    local cmd_item = entity.find_driver("dMenuItem")
    local checked = cmd_item.get_checked()
    checked = not checked
    cmd_item.set_checked(checked)
end
