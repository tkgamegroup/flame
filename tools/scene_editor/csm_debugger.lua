local e = entity

local layer = root.find_child("debug_layer")
if layer.p then
	local debugger = {
		ppoints = nil,
		camera_lines = { nil, nil, nil, nil },
		light_lines = { 
			{ nil, nil, nil, nil },
			{ nil, nil, nil, nil },
			{ nil, nil, nil, nil },
			{ nil, nil, nil, nil }
		},

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
			local lines = debugger.light_lines[i]
			for j=1, 4, 1 do
				if lines[j] then
					r.draw_lines(12, lines[j])
				end
			end
		end
	end))
	element.add_drawer(debugger.drawer)
	e.find_driver("dWindow").add_close_listener(function()
		element.remove_drawer(debugger.drawer)
		debugger.drawer.release()
	end)

	e.find_child("capture_btn").find_component("cReceiver").add_mouse_click_listener(function()
		local camera = s_renderer.get_camera()
		if not camera.p then return end

		local plvs = flame_malloc(4)
		local pdist = flame_malloc(4)
		s_renderer.get_shadow_props(plvs, pdist, nil)
		local lvs = flame_get(plvs, 0, e_type_data, e_integer_type, 1, 1)
		local dist = flame_get(pdist, 0, e_type_data, e_floating_type, 1, 1)
		flame_free(plvs)
		flame_free(pdist)

		function set_lines(dst, points, col)
			flame_cpy(dst, 32 * 0 + 16 * 0 + 0, points, 0, 12)
			flame_cpy(dst, 32 * 0 + 16 * 0 + 12, col, 0, 4)
			flame_cpy(dst, 32 * 0 + 16 * 1 + 0, points, 12, 12)
			flame_cpy(dst, 32 * 0 + 16 * 1 + 12, col, 0, 4)

			flame_cpy(dst, 32 * 1 + 16 * 0 + 0, points, 12, 12)
			flame_cpy(dst, 32 * 1 + 16 * 0 + 12, col, 0, 4)
			flame_cpy(dst, 32 * 1 + 16 * 1 + 0, points, 24, 12)
			flame_cpy(dst, 32 * 1 + 16 * 1 + 12, col, 0, 4)

			flame_cpy(dst, 32 * 2 + 16 * 0 + 0, points, 24, 12)
			flame_cpy(dst, 32 * 2 + 16 * 0 + 12, col, 0, 4)
			flame_cpy(dst, 32 * 2 + 16 * 1 + 0, points, 36, 12)
			flame_cpy(dst, 32 * 2 + 16 * 1 + 12, col, 0, 4)

			flame_cpy(dst, 32 * 3 + 16 * 0 + 0, points, 36, 12)
			flame_cpy(dst, 32 * 3 + 16 * 0 + 12, col, 0, 4)
			flame_cpy(dst, 32 * 3 + 16 * 1 + 0, points, 0, 12)
			flame_cpy(dst, 32 * 3 + 16 * 1 + 12, col, 0, 4)

			flame_cpy(dst, 32 * 4 + 16 * 0 + 0, points, 48, 12)
			flame_cpy(dst, 32 * 4 + 16 * 0 + 12, col, 0, 4)
			flame_cpy(dst, 32 * 4 + 16 * 1 + 0, points, 60, 12)
			flame_cpy(dst, 32 * 4 + 16 * 1 + 12, col, 0, 4)

			flame_cpy(dst, 32 * 5 + 16 * 0 + 0, points, 60, 12)
			flame_cpy(dst, 32 * 5 + 16 * 0 + 12, col, 0, 4)
			flame_cpy(dst, 32 * 5 + 16 * 1 + 0, points, 72, 12)
			flame_cpy(dst, 32 * 5 + 16 * 1 + 12, col, 0, 4)

			flame_cpy(dst, 32 * 6 + 16 * 0 + 0, points, 72, 12)
			flame_cpy(dst, 32 * 6 + 16 * 0 + 12, col, 0, 4)
			flame_cpy(dst, 32 * 6 + 16 * 1 + 0, points, 84, 12)
			flame_cpy(dst, 32 * 6 + 16 * 1 + 12, col, 0, 4)

			flame_cpy(dst, 32 * 7 + 16 * 0 + 0, points, 84, 12)
			flame_cpy(dst, 32 * 7 + 16 * 0 + 12, col, 0, 4)
			flame_cpy(dst, 32 * 7 + 16 * 1 + 0, points, 48, 12)
			flame_cpy(dst, 32 * 7 + 16 * 1 + 12, col, 0, 4)

			flame_cpy(dst, 32 * 8 + 16 * 0 + 0, points, 0, 12)
			flame_cpy(dst, 32 * 8 + 16 * 0 + 12, col, 0, 4)
			flame_cpy(dst, 32 * 8 + 16 * 1 + 0, points, 48, 12)
			flame_cpy(dst, 32 * 8 + 16 * 1 + 12, col, 0, 4)

			flame_cpy(dst, 32 * 9 + 16 * 0 + 0, points, 12, 12)
			flame_cpy(dst, 32 * 9 + 16 * 0 + 12, col, 0, 4)
			flame_cpy(dst, 32 * 9 + 16 * 1 + 0, points, 60, 12)
			flame_cpy(dst, 32 * 9 + 16 * 1 + 12, col, 0, 4)

			flame_cpy(dst, 32 * 10 + 16 * 0 + 0, points, 24, 12)
			flame_cpy(dst, 32 * 10 + 16 * 0 + 12, col, 0, 4)
			flame_cpy(dst, 32 * 10 + 16 * 1 + 0, points, 72, 12)
			flame_cpy(dst, 32 * 10 + 16 * 1 + 12, col, 0, 4)

			flame_cpy(dst, 32 * 11 + 16 * 0 + 0, points, 36, 12)
			flame_cpy(dst, 32 * 11 + 16 * 0 + 12, col, 0, 4)
			flame_cpy(dst, 32 * 11 + 16 * 1 + 0, points, 84, 12)
			flame_cpy(dst, 32 * 11 + 16 * 1 + 12, col, 0, 4)
		end

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
				set_lines(debugger.camera_lines[i], points, col)
			end

			flame_free(col)

			flame_free(points)
		end
		
		local e_dir = find_enum("LightType")["Directional"]
		local n = s_renderer.get_shadow_count(e_dir)
		if n > 0 then
			local points = flame_malloc(8 * 12)

			local col = flame_malloc(4)
			flame_set(col, 0, e_type_data, e_char_type, 4, 1, vec4(255, 255, 0, 255))

			local matrices = flame_malloc(4 * 64)
			for i=0, n-1, 1 do
				s_renderer.get_shadow_matrices(e_dir, i, matrices)

				local lines = debugger.light_lines[i + 1]
				for j=0, lvs-1, 1 do
					if not lines[j + 1] then 
						lines[j + 1] = flame_malloc(12 * 32)
					end
					local m = flame_mat4_inverse(flame_get(matrices, j * 64, e_type_data, e_floating_type, 4, 4))
					flame_set(points, 12 * 0, e_type_data, e_floating_type, 3, 1, vec3(flame_mat4_transform(m, vec4(-1, 1, 0, 1))))
					flame_set(points, 12 * 1, e_type_data, e_floating_type, 3, 1, vec3(flame_mat4_transform(m, vec4(1, 1, 0, 1))))
					flame_set(points, 12 * 2, e_type_data, e_floating_type, 3, 1, vec3(flame_mat4_transform(m, vec4(1, -1, 0, 1))))
					flame_set(points, 12 * 3, e_type_data, e_floating_type, 3, 1, vec3(flame_mat4_transform(m, vec4(-1, -1, 0, 1))))
					flame_set(points, 12 * 4, e_type_data, e_floating_type, 3, 1, vec3(flame_mat4_transform(m, vec4(-1, 1, 1, 1))))
					flame_set(points, 12 * 5, e_type_data, e_floating_type, 3, 1, vec3(flame_mat4_transform(m, vec4(1, 1, 1, 1))))
					flame_set(points, 12 * 6, e_type_data, e_floating_type, 3, 1, vec3(flame_mat4_transform(m, vec4(1, -1, 1, 1))))
					flame_set(points, 12 * 7, e_type_data, e_floating_type, 3, 1, vec3(flame_mat4_transform(m, vec4(-1, -1, 1, 1))))
					set_lines(lines[j + 1], points, col)
				end
			end
			flame_free(matrices)

			flame_free(col)

			flame_free(points)
		end
	end)

	e.find_child("clear_btn").find_component("cReceiver").add_mouse_click_listener(function()
		for i=1, 4, 1 do
			if debugger.camera_lines[i] then
				flame_free(debugger.camera_lines[i])
				debugger.camera_lines[i] = nil
			end
		end
		for i=1, 4, 1 do
			local lines = debugger.light_lines[i]
			for j=1, 4, 1 do
				if lines[j] then
					flame_free(lines[j])
					lines[j] = nil
				end
			end
		end
	end)
end
