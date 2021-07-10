function cmd_file_open()
    local l = create_entity("prefabs/layer")

    local d = create_entity("prefabs/file_selector")

    d.find_driver("dWindow").set_title("Open")
    local file_selector = d.find_driver("dFileSelector")
    file_selector.add_callback(function(ok, text)
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
    local l = create_entity("prefabs/layer")
    local d = create_entity("prefabs/input_dialog")
              
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

local e_rendertype = find_enum("RenderType")
local e_wireframe = e_rendertype["Wireframe"]
local e_shaded = e_rendertype["Shaded"]
local e_normaldata = e_rendertype["NormalData"]

function cmd_shading_wireframe()
    s_renderer.set_render_type(e_wireframe)
end

function cmd_shading_shaded()
    s_renderer.set_render_type(e_shaded)
end

function cmd_shading_normal_data()
    s_renderer.set_render_type(e_normaldata)
end

function cmd_show_physics_visualization(entity)
    local cmd_item = entity.find_driver("dMenuItem")
    local checked = cmd_item.get_checked()
    checked = not checked
    cmd_item.set_checked(checked)
    s_physics.set_visualization(checked)
end

ui_reflector = { p=nil }
function cmd_show_ui_reflector(entity)
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

function cmd_show_global_axes(entity)
    local cmd_item = entity.find_driver("dMenuItem")
    local checked = cmd_item.get_checked()
    checked = not checked
    cmd_item.set_checked(checked)
    scene.find_child("hud_global_axes").set_visible(checked)
end

function cmd_show_crosshair(entity)
    local cmd_item = entity.find_driver("dMenuItem")
    local checked = cmd_item.get_checked()
    checked = not checked
    cmd_item.set_checked(checked)
    scene.find_child("hud_crosshair").set_visible(checked)
end

function cmd_settings_alwawys_update(entity)
    local cmd_item = entity.find_driver("dMenuItem")
    local checked = cmd_item.get_checked()
    checked = not checked
    cmd_item.set_checked(checked)
end
