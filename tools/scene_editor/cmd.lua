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

function cmd_view_physics_visualization(checked)
    s_physics.set_visualization(checked)
end

function cmd_view_global_axes(checked)
    scene.find_child("hud_global_axes").set_visible(checked)
end

function cmd_view_crosshair(checked)
    scene.find_child("hud_crosshair").set_visible(checked)
end

function cmd_view_camera_fly()
    if camera then
        camera.destroy()
        camera = make_fly_camera(camera.entity)
    end
end

function cmd_view_camera_arcball()
    if camera then
        camera.destroy()
        camera = make_arcball_camera(camera.entity)
    end
end

ui_reflector = { p=nil }
function cmd_tools_ui_reflector(checked)
    if checked then
        if not ui_reflector.p then
            ui_reflector = create_entity("ui_reflector")
            ui.add_child(ui_reflector)
        end
    else
        if ui_reflector.p then
            ui.remove_child(ui_reflector)
            ui_reflector.p = nil
        end
    end
end

csm_debugger = { p=nil }
function cmd_tools_csm_debugger(checked)
    if checked then
        if not csm_debugger.p then
            csm_debugger = create_entity("csm_debugger")
            csm_debugger.find_driver("dWindow").add_close_listener(function()
                cmd_item.set_checked(false)
                ui.remove_child(csm_debugger)
                csm_debugger.p = nil
            end)
            ui.add_child(csm_debugger)
        end
    else
        if csm_debugger.p then
            ui.remove_child(csm_debugger)
            csm_debugger.p = nil
        end
    end
end

function cmd_settings_alwawys_update(checked)
    cmd_item.set_checked(checked)
end
