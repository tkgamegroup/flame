local node = entity:get_component_n("cNode")
make_obj(node, "cNode")
local camera = {
	node = node,
	pos = node:get_pos(),
	yaw = 0,
	pitch = 0,
	dir1 = { x=0, y=0, z=1 },
	dir2 = { x=1, y=0, z=0 },
	w = false,
	s = false,
	a = false,
	d = false,
	q = false,
	e = false,
	z = false,
	x = false,
	sp = false,
	sh = false
}
local root_event_receiver = entity:get_world():get_root():get_component_n("cEventReceiver")
make_obj(root_event_receiver, "cEventReceiver")
root_event_receiver:add_key_down_listener_s(get_slot(
	function(k)
		if (k == enums["flame::KeyboardKey"]["W"]) then
			camera.w = true
		end
		if (k == enums["flame::KeyboardKey"]["S"]) then
			camera.s = true
		end
		if (k == enums["flame::KeyboardKey"]["A"]) then
			camera.a = true
		end
		if (k == enums["flame::KeyboardKey"]["D"]) then
			camera.d = true
		end
		if (k == enums["flame::KeyboardKey"]["Q"]) then
			camera.q = true
		end
		if (k == enums["flame::KeyboardKey"]["E"]) then
			camera.e = true
		end
		if (k == enums["flame::KeyboardKey"]["Z"]) then
			camera.z = true
		end
		if (k == enums["flame::KeyboardKey"]["X"]) then
			camera.x = true
		end
		if (k == enums["flame::KeyboardKey"]["Space"]) then
			camera.sp = true
		end
		if (k == enums["flame::KeyboardKey"]["Shift"]) then
			camera.sh = true
		end
	end
))
root_event_receiver:add_key_up_listener_s(get_slot(
	function(k)
		if (k == enums["flame::KeyboardKey"]["W"]) then
			camera.w = false
		end
		if (k == enums["flame::KeyboardKey"]["S"]) then
			camera.s = false
		end
		if (k == enums["flame::KeyboardKey"]["A"]) then
			camera.a = false
		end
		if (k == enums["flame::KeyboardKey"]["D"]) then
			camera.d = false
		end
		if (k == enums["flame::KeyboardKey"]["Q"]) then
			camera.q = false
		end
		if (k == enums["flame::KeyboardKey"]["E"]) then
			camera.e = false
		end
		if (k == enums["flame::KeyboardKey"]["Z"]) then
			camera.z = false
		end
		if (k == enums["flame::KeyboardKey"]["X"]) then
			camera.x = false
		end
		if (k == enums["flame::KeyboardKey"]["Space"]) then
			camera.sp = false
		end
		if (k == enums["flame::KeyboardKey"]["Shift"]) then
			camera.sh = false
		end
	end
))
entity:add_event_s(get_slot(
	function()
		if camera.w then
			camera.pos.x = camera.pos.x - camera.dir1.x * 0.2
			camera.pos.y = camera.pos.y - camera.dir1.y * 0.2
			camera.pos.z = camera.pos.z - camera.dir1.z * 0.2
			camera.node:set_pos(camera.pos)
		end
		if camera.s then
			camera.pos.x = camera.pos.x + camera.dir1.x * 0.2
			camera.pos.y = camera.pos.y + camera.dir1.y * 0.2
			camera.pos.z = camera.pos.z + camera.dir1.z * 0.2
			camera.node:set_pos(camera.pos)
		end
		if camera.q then
			camera.pos.x = camera.pos.x - camera.dir2.x * 0.2
			camera.pos.y = camera.pos.y - camera.dir2.y * 0.2
			camera.pos.z = camera.pos.z - camera.dir2.z * 0.2
			camera.node:set_pos(camera.pos)
		end
		if camera.e then
			camera.pos.x = camera.pos.x + camera.dir2.x * 0.2
			camera.pos.y = camera.pos.y + camera.dir2.y * 0.2
			camera.pos.z = camera.pos.z + camera.dir2.z * 0.2
			camera.node:set_pos(camera.pos)
		end
		if camera.a then
			camera.yaw = camera.yaw + 1
			local euler = { x=camera.yaw, y=camera.pitch, z=0 }
			camera.node:set_euler(euler)
			camera.dir1 = camera.node:get_dir(2)
			camera.dir2 = camera.node:get_dir(0)
		end
		if camera.d then
			camera.yaw = camera.yaw - 1
			local euler = { x=camera.yaw, y=camera.pitch, z=0 }
			camera.node:set_euler(euler)
			camera.dir1 = camera.node:get_dir(2)
			camera.dir2 = camera.node:get_dir(0)
		end
		if camera.z then
			camera.pitch = camera.pitch - 1
			local euler = { x=camera.yaw, y=camera.pitch, z=0 }
			camera.node:set_euler(euler)
			camera.dir1 = camera.node:get_dir(2)
			camera.dir2 = camera.node:get_dir(0)
		end
		if camera.x then
			camera.pitch = camera.pitch + 1
			local euler = { x=camera.yaw, y=camera.pitch, z=0 }
			camera.node:set_euler(euler)
			camera.dir1 = camera.node:get_dir(2)
			camera.dir2 = camera.node:get_dir(0)
		end
		if camera.sp then
			camera.pos.y = camera.pos.y + 0.2
			camera.node:set_pos(camera.pos)
		end
		if camera.sh then
			camera.pos.y = camera.pos.y - 0.2
			camera.node:set_pos(camera.pos)
		end
	end
))