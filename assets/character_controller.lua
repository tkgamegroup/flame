local node = entity:get_component_n("cNode")
make_obj(node, "cNode")

local cc = entity:get_component_n("cController")
make_obj(cc, "cController")

controller = {
	speed = 0.5,
	node = node,
	cc = cc,
	yaw = 0,
	dir1 = { x=0, y=0, z=1 },
	dir2 = { x=1, y=0, z=0 },
	w = false,
	s = false,
	a = false,
	d = false,
	q = false,
	e = false
}

local root_event_receiver = root:get_component_n("cEventReceiver")
make_obj(root_event_receiver, "cEventReceiver")

root_event_receiver:add_key_down_listener_s(get_slot(
	function(k)
		if k == enums["flame::KeyboardKey"]["W"] then
			controller.w = true
		end
		if k == enums["flame::KeyboardKey"]["S"] then
			controller.s = true
		end
		if k == enums["flame::KeyboardKey"]["A"] then
			controller.a = true
		end
		if k == enums["flame::KeyboardKey"]["D"] then
			controller.d = true
		end
		if k == enums["flame::KeyboardKey"]["Q"] then
			controller.q = true
		end
		if k == enums["flame::KeyboardKey"]["E"] then
			controller.e = true
		end
	end
))

root_event_receiver:add_key_up_listener_s(get_slot(
	function(k)
		if k == enums["flame::KeyboardKey"]["W"] then
			controller.w = false
		end
		if k == enums["flame::KeyboardKey"]["S"] then
			controller.s = false
		end
		if k == enums["flame::KeyboardKey"]["A"] then
			controller.a = false
		end
		if k == enums["flame::KeyboardKey"]["D"] then
			controller.d = false
		end
		if k == enums["flame::KeyboardKey"]["Q"] then
			controller.q = false
		end
		if k == enums["flame::KeyboardKey"]["E"] then
			controller.e = false
		end
	end
))

entity:add_event_s(get_slot(
	function()
		if controller.w then
			controller.cc:move({x=controller.dir1.x * controller.speed, y=controller.dir1.y * controller.speed, z=controller.dir1.z * controller.speed})
		end
		if controller.s then
			controller.cc:move({x=-controller.dir1.x * controller.speed, y=-controller.dir1.y * controller.speed, z=-controller.dir1.z * controller.speed})
		end
		if controller.q then
			controller.cc:move({x=controller.dir2.x * controller.speed, y=controller.dir2.y * controller.speed, z=controller.dir2.z * controller.speed})
		end
		if controller.e then
			controller.cc:move({x=-controller.dir2.x * controller.speed, y=-controller.dir2.y * controller.speed, z=-controller.dir2.z * controller.speed})
		end
		if controller.a then
			controller.yaw = controller.yaw + 1
			controller.node:set_euler({ x=controller.yaw, y=0, z=0 })
			controller.dir1 = controller.node:get_global_dir(2)
		end
		if controller.d then
			controller.yaw = controller.yaw - 1
			controller.node:set_euler({ x=controller.yaw, y=0, z=0 })
			controller.dir1 = controller.node:get_global_dir(2)
		end
	end
))
