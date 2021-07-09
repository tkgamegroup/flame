function make_fly_camera(entity)
	local camera = {
		speed = 0.5,
		node = entity.find_component("cNode"),
		yaw = 0,
		pitch = 0,
		dir1 = vec3(0, 0, 1),
		dir2 = vec3(1, 0, 0),
		dragging = false,
		w = false,
		s = false,
		a = false,
		d = false,
		sp = false,
		sh = false
	}

	camera.move = function(dir, v)
		camera.pos = camera.pos + dir * v
		camera.node.set_pos(camera.pos)
	end

	camera.update_dir = function()
		camera.node.set_euler(vec3(camera.yaw, camera.pitch, 0))
		camera.dir1 = camera.node.get_local_dir(2)
		camera.dir2 = camera.node.get_local_dir(0)
	end

	local scene_receiver = scene.find_component("cReceiver")

	local e_keyboardkey = find_enum("KeyboardKey")
	local e_key_w = e_keyboardkey["W"]
	local e_key_s = e_keyboardkey["S"]
	local e_key_a = e_keyboardkey["A"]
	local e_key_d = e_keyboardkey["D"]
	local e_key_sp = e_keyboardkey["Space"]
	local e_key_shift = e_keyboardkey["Shift"]
	scene_receiver.add_key_down_listener(function(k)
		if k == e_key_w then
			camera.w = true
		elseif k == e_key_s then
			camera.s = true
		elseif k == e_key_a then
			camera.a = true
		elseif k == e_key_d then
			camera.d = true
		elseif k == e_key_sp then
			camera.sp = true
		elseif k == e_key_shift then
			camera.sh = true
		end
	end)

	scene_receiver.add_key_up_listener(function(k)
		if k == e_key_w then
			camera.w = false
		elseif k == e_key_s then
			camera.s = false
		elseif k == e_key_a then
			camera.a = false
		elseif k == e_key_d then
			camera.d = false
		elseif k == e_key_sp then
			camera.sp = false
		elseif k == e_key_shift then
			camera.sh = false
		end
	end)

	scene_receiver.add_mouse_left_down_listener(function()
		camera.dragging = true
	end)

	scene_receiver.add_mouse_left_up_listener(function()
		camera.dragging = false
	end)

	scene_receiver.add_mouse_move_listener(function(disp)
		if camera.dragging then
			camera.yaw = camera.yaw - disp.x
			camera.pitch = camera.pitch - disp.y
			camera.update_dir()
		end
	end)

	entity.add_event(function()
		local moved = false
		if camera.w then
			camera.pos = camera.pos - camera.dir * camera.speed
			moved = true
		end
		if camera.s then
			camera.pos = camera.pos + camera.dir1 * camera.speed
			moved = true
		end
		if camera.a then
			camera.pos = camera.pos - dir2 * camera.speed
			moved = true
		end
		if camera.d then
			camera.pos = camera.pos + camera.dir2 * camera.speed
			moved = true
		end
		if camera.sp then
			camera.pos.y = camera.pos.y + camera.speed
			moved = true
		end
		if camera.sh then
			camera.pos.y = camera.pos.y - camera.speed
			moved = true
		end
		if moved then
			camera.node.set_pos(camera.pos)
		end
	end)

	camera.node.set_euler(vec3(camera.yaw, camera.pitch, 0))
	camera.pos = camera.node.get_pos()

	return camera
end

function make_third_camera(entity)
	local camera = {
		node = entity.find_component("cNode"),
		length = 5,
		yaw = 0,
		pitch = -90,
		dragging = false,
		infront_occluder = false
	}
	
	local pnode = entity.get_parent().find_component("cNode")
	camera.set_pos = function()
		local off = vec3(0, 3, 0)
		local d = 0
		if infront_occluder and s_physics.p then
			local o = pnode.get_global_pos() + off
			local p = s_physics.raycast(o, camera.node.get_global_dir(2))
			d = distance_3(o, p)
			d = d < camera.length and d - 1 or camera.length
		else
			d = camera.length
		end
		if d < 0.1 then d = 0.1 end
		camera.node.set_pos(camera.node.get_local_dir(2) * d + off)
	end

	entity.add_event(function()
		camera.set_pos()
	end, 0.5)

	local scene_receiver = scene.find_component("cReceiver")

	scene_receiver.add_mouse_left_down_listener(function()
		camera.dragging = true
	end)

	scene_receiver.add_mouse_left_up_listener(function()
		camera.dragging = false
	end)

	scene_receiver.add_mouse_scroll_listener(function(scroll)
		if scroll > 0 then
			camera.length = camera.length * 0.9 - 0.1
		else
			camera.length = camera.length * 1.1 + 0.1
		end
		camera.set_pos()
	end)

	scene_receiver.add_mouse_move_listener(function(disp)
		if camera.dragging then
			camera.yaw = camera.yaw - disp.x
			camera.pitch = camera.pitch - disp.y
			camera.node.set_euler(vec3(camera.yaw, camera.pitch, 0))
			camera.set_pos()
		end
	end)

	camera.node.set_euler(vec3(camera.yaw, camera.pitch, 0))
	camera.set_pos()

	return camera
end
