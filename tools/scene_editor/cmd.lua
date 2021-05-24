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

function cmd_settings_alwawys_update()
    local cmd_item = entity.find_driver("dMenuItem")
    local checked = cmd_item.get_checked()
    checked = not checked
    cmd_item.set_checked(checked)
end
