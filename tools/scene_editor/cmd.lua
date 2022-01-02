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

function cmd_view_bounds(checked)
    scene.find_child("debug_bounds").set_visible(checked)
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
cmd_tools_ui_reflector = {
    f = function (checked)
        if checked then
            if not ui_reflector.p then
                ui_reflector = create_entity("ui_reflector")
                ui_reflector.find_component("cWindow").add_close_listener(function()
                    cmd_tools_ui_reflector.dmi.set_checked(false)
                    ui_reflector.p = nil
                end)
                ui.add_child(ui_reflector)
            end
        else
            if ui_reflector.p then
                ui.remove_child(ui_reflector)
                ui_reflector.p = nil
            end
        end
    end
}

csm_debugger = { p=nil }
cmd_tools_csm_debugger = {
    f = function(checked)
        if checked then
            if not csm_debugger.p then
                csm_debugger = create_entity("csm_debugger")
                csm_debugger.find_component("cWindow").add_close_listener(function()
                    cmd_tools_csm_debugger.dmi.set_checked(false)
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
}

ssao_debugger = { p=nil }
cmd_tools_ssao_debugger = {
    f = function (checked)
        if checked then
            if not ssao_debugger.p then
                ssao_debugger = create_entity("ssao_debugger")
                ssao_debugger.find_component("cWindow").add_close_listener(function()
                    cmd_tools_ssao_debugger.dmi.set_checked(false)
                    ssao_debugger.p = nil
                end)
                ui.add_child(ssao_debugger)
            end
        else
            if ssao_debugger.p then
                ui.remove_child(ssao_debugger)
                ssao_debugger.p = nil
            end
        end
    end
}
