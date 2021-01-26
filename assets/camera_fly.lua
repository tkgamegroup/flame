local node = entity.find_component("cNode")

camera = {
	speed = 0.5,
	node = node,
	pos = node.get_pos(),
	yaw = 0,
	pitch = 0,
	dir1 = vec3(0, 0, 1),
	dir2 = vec3(1, 0, 0),
	dragging = false,
	w = false,
	s = false,
	a = false,
	d = false,
	z = false,
	x = false,
	sp = false,
	sh = false
}

camera.move = function (dir, v)
	camera.pos.x = camera.pos.x + dir.x * v
	camera.pos.y = camera.pos.y + dir.y * v
	camera.pos.z = camera.pos.z + dir.z * v
	camera.node.set_pos(camera.pos)
end

camera.update_dir = function ()
	camera.node.set_euler(vec3(camera.yaw, camera.pitch, 0))
	camera.dir1 = camera.node.get_local_dir(2)
	camera.dir2 = camera.node.get_local_dir(0)
end

local root_receiver = root.find_component("cReceiver")

root_receiver.add_key_down_listener_s(get_slot(
	function(k)
		if k == enums["flame::KeyboardKey"]["W"] then
			camera.w = true
		end
		if k == enums["flame::KeyboardKey"]["S"] then
			camera.s = true
		end
		if k == enums["flame::KeyboardKey"]["A"] then
			camera.a = true
		end
		if k == enums["flame::KeyboardKey"]["D"] then
			camera.d = true
		end
		if k == enums["flame::KeyboardKey"]["Space"] then
			camera.sp = true
		end
		if k == enums["flame::KeyboardKey"]["Shift"] then
			camera.sh = true
		end
	end
))

root_receiver.add_key_up_listener_s(get_slot(
	function(k)
		if k == enums["flame::KeyboardKey"]["W"] then
			camera.w = false
		end
		if k == enums["flame::KeyboardKey"]["S"] then
			camera.s = false
		end
		if k == enums["flame::KeyboardKey"]["A"] then
			camera.a = false
		end
		if k == enums["flame::KeyboardKey"]["D"] then
			camera.d = false
		end
		if k == enums["flame::KeyboardKey"]["Space"] then
			camera.sp = false
		end
		if k == enums["flame::KeyboardKey"]["Shift"] then
			camera.sh = false
		end
	end
))

root_receiver.add_mouse_left_down_listener_s(get_slot(
	function()
		camera.dragging = true
	end
))

root_receiver.add_mouse_left_up_listener_s(get_slot(
	function()
		camera.dragging = false
	end
))

root_receiver.add_mouse_move_listener_s(get_slot(
	function(disp)
		if camera.dragging then
			camera.yaw = camera.yaw - disp.x
			camera.pitch = camera.pitch - disp.y
			camera.update_dir()
		end
	end
))

entity.add_event(function()
		if camera.w then
			camera.move(camera.dir1, -camera.speed)
		end
		if camera.s then
			camera.move(camera.dir1, camera.speed)
		end
		if camera.a then
			camera.move(camera.dir2, -camera.speed)
		end
		if camera.d then
			camera.move(camera.dir2, camera.speed)
		end
		if camera.sp then
			camera.pos.y = camera.pos.y + camera.speed
			camera.node.set_pos(camera.pos)
		end
		if camera.sh then
			camera.pos.y = camera.pos.y - camera.speed
			camera.node.set_pos(camera.pos)
		end
end)
