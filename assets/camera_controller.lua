local node = entity:get_component_n("cNode")
make_obj(node, "cNode")
local camera = {
	node = node,
	pos = node:get_pos(),
	yaw = 0,
	pitch = 0,
	w = false,
	s = false,
	a = false,
	d = false,
	q = false,
	e = false,
	dir = { x=0, y=0, z=1 }
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
	end
))
entity:add_event_s(get_slot(
	function()
		if camera.w then
			camera.pos.x = camera.pos.x - camera.dir.x * 0.2
			camera.pos.y = camera.pos.y - camera.dir.y * 0.2
			camera.pos.z = camera.pos.z - camera.dir.z * 0.2
			camera.node:set_pos(camera.pos)
		end
		if camera.s then
			camera.pos.x = camera.pos.x + camera.dir.x * 0.2
			camera.pos.y = camera.pos.y + camera.dir.y * 0.2
			camera.pos.z = camera.pos.z + camera.dir.z * 0.2
			camera.node:set_pos(camera.pos)
		end
		if camera.a then
			camera.yaw = camera.yaw + 1
			camera.node:set_euler(camera.yaw, camera.pitch, 0)
			local v = camera.node:get_dir(2)
			camera.dir.x = v.x
			camera.dir.y = v.y
			camera.dir.z = v.z
		end
		if camera.d then
			camera.yaw = camera.yaw - 1
			camera.node:set_euler(camera.yaw, camera.pitch, 0)
			local v = camera.node:get_dir(2)
			camera.dir.x = v.x
			camera.dir.y = v.y
			camera.dir.z = v.z
		end
		if camera.q then
			camera.pitch = camera.pitch - 1
			camera.node:set_euler(camera.yaw, camera.pitch, 0)
			local v = camera.node:get_dir(2)
			camera.dir.x = v.x
			camera.dir.y = v.y
			camera.dir.z = v.z
		end
		if camera.e then
			camera.pitch = camera.pitch + 1
			camera.node:set_euler(camera.yaw, camera.pitch, 0)
			local v = camera.node:get_dir(2)
			camera.dir.x = v.x
			camera.dir.y = v.y
			camera.dir.z = v.z
		end
	end
))