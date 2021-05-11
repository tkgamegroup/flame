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
    scatter_vegetations()
end

function menu_settings_alwawys_update()
    local menu_item = entity.find_driver("dMenuItem")
    local checked = menu_item.get_checked()
    checked = not checked
    menu_item.set_checked(checked)
end
