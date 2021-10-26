local layer = root.find_child("debug_layer")
if layer.p then
	local debugger = {
		ppoints = nil,
		camera_lines = { nil, nil, nil, nil },
		light_lines = { nil, nil, nil, nil },

		drawer = nil
	}

	local element = layer.find_component("cElement")
	debugger.drawer = find_udt("ElementScriptDrawer").static_functions.create()
	debugger.drawer.set_callback(get_callback_slot(function(l, r)
		for i=1, 4, 1 do
			if debugger.camera_lines[i] then
				r.draw_lines(12, debugger.camera_lines[i])
			end
		end
		for i=1, 4, 1 do
			if debugger.light_lines[i] then
				r.draw_lines(12, debugger.light_lines[i])
			end
		end
	end))
	element.add_drawer(debugger.drawer)
	entity.find_component("cWindow").add_close_listener(function()
		element.remove_drawer(debugger.drawer)
		debugger.drawer.release()
	end)

	entity.find_child("capture_btn").find_component("cReceiver").add_mouse_click_listener(function()
		local camera = s_renderer.get_camera()
		if not camera.p then return end

		local plvs = flame_malloc(4)
		local pdist = flame_malloc(4)
		s_renderer.get_shadow_props(plvs, pdist, nil)
		local lvs = flame_get_i(plvs, 0)
		local dist = flame_get_F(pdist, 0)
		flame_free(plvs)
		flame_free(pdist)

		if lvs > 0 then
			local points = flame_malloc(8 * 12)

			local col = flame_malloc(4)
			flame_set(col, 0, e_type_data, e_char_type, 4, 1, vec4(0, 255, 0, 255))

			for i=1, lvs, 1 do
				if not debugger.camera_lines[i] then 
					debugger.camera_lines[i] = flame_malloc(12 * 32)
				end

				local n = (i - 1) / lvs
				n = n * n
				local f = i / lvs
				f = f * f
				camera.get_points(points, n * dist, f * dist)
				hexahedron_draw_lines(debugger.camera_lines[i], points, col)
			end

			flame_free(col)

			flame_free(points)
		end
		
		local sun = root.find_child("sun")
		if sun.p then
			local points = flame_malloc(8 * 12)

			local col = flame_malloc(4)
			flame_set(col, 0, e_type_data, e_char_type, 4, 1, vec4(0, 0, 255, 255))

			local light = sun.find_component("cLight")
			for i=0, lvs-1, 1 do
				if not debugger.light_lines[i + 1] then 
					debugger.light_lines[i + 1] = flame_malloc(12 * 32)
				end
				local m = flame_mat4_inverse(light.get_shadow_mat(i))
				flame_set(points, 12 * 0, e_type_data, e_floating_type, 3, 1, vec3(flame_mat4_transform(m, vec4(-1, 1, 0, 1))))
				flame_set(points, 12 * 1, e_type_data, e_floating_type, 3, 1, vec3(flame_mat4_transform(m, vec4(1, 1, 0, 1))))
				flame_set(points, 12 * 2, e_type_data, e_floating_type, 3, 1, vec3(flame_mat4_transform(m, vec4(1, -1, 0, 1))))
				flame_set(points, 12 * 3, e_type_data, e_floating_type, 3, 1, vec3(flame_mat4_transform(m, vec4(-1, -1, 0, 1))))
				flame_set(points, 12 * 4, e_type_data, e_floating_type, 3, 1, vec3(flame_mat4_transform(m, vec4(-1, 1, 1, 1))))
				flame_set(points, 12 * 5, e_type_data, e_floating_type, 3, 1, vec3(flame_mat4_transform(m, vec4(1, 1, 1, 1))))
				flame_set(points, 12 * 6, e_type_data, e_floating_type, 3, 1, vec3(flame_mat4_transform(m, vec4(1, -1, 1, 1))))
				flame_set(points, 12 * 7, e_type_data, e_floating_type, 3, 1, vec3(flame_mat4_transform(m, vec4(-1, -1, 1, 1))))
				hexahedron_draw_lines(debugger.light_lines[i + 1], points, col)
			end

			flame_free(col)

			flame_free(points)
		end
	end)

	entity.find_child("clear_btn").find_component("cReceiver").add_mouse_click_listener(function()
		for i=1, 4, 1 do
			if debugger.camera_lines[i] then
				flame_free(debugger.camera_lines[i])
				debugger.camera_lines[i] = nil
			end
		end
		for i=1, 4, 1 do
			if debugger.light_lines[i] then
				flame_free(debugger.light_lines[i])
				debugger.light_lines[i] = nil
			end
		end
	end)
end
