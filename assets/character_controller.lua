local node = entity:get_component_n("cNode")
make_obj(node, "cNode")

local mesh = entity:find_child("mesh"):get_component_n("cMesh")
make_obj(mesh, "cMesh")

local controller = entity:get_component_n("cController")
make_obj(controller, "cController")

character = {
	speed = 0.2,
	node = node,
	mesh = mesh,
	controller = controller,
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

function character:update_dir()
	self.node:set_euler({ x=self.yaw, y=0, z=0 })
	self.dir1 = self.node:get_local_dir(2)
	self.dir2 = self.node:get_local_dir(0)
end

local root_event_receiver = root:get_component_n("cEventReceiver")
make_obj(root_event_receiver, "cEventReceiver")

root_event_receiver:add_key_down_listener_s(get_slot(
	function(k)
		if k == enums["flame::KeyboardKey"]["W"] then
			character.w = true
			character.mesh:set_animation_name("walk")
		end
		if k == enums["flame::KeyboardKey"]["S"] then
			character.s = true
		end
		if k == enums["flame::KeyboardKey"]["A"] then
			character.a = true
		end
		if k == enums["flame::KeyboardKey"]["D"] then
			character.d = true
		end
		if k == enums["flame::KeyboardKey"]["Q"] then
			character.q = true
		end
		if k == enums["flame::KeyboardKey"]["E"] then
			character.e = true
		end
	end
))

root_event_receiver:add_key_up_listener_s(get_slot(
	function(k)
		if k == enums["flame::KeyboardKey"]["W"] then
			character.w = false
			character.mesh:set_animation_name("")
		end
		if k == enums["flame::KeyboardKey"]["S"] then
			character.s = false
		end
		if k == enums["flame::KeyboardKey"]["A"] then
			character.a = false
		end
		if k == enums["flame::KeyboardKey"]["D"] then
			character.d = false
		end
		if k == enums["flame::KeyboardKey"]["Q"] then
			character.q = false
		end
		if k == enums["flame::KeyboardKey"]["E"] then
			character.e = false
		end
	end
))

entity:add_event_s(get_slot(
	function()
		if character.w then
			character.controller:move({x=character.dir1.x * character.speed, y=character.dir1.y * character.speed, z=character.dir1.z * character.speed})
		end
		if character.s then
			character.controller:move({x=-character.dir1.x * character.speed, y=-character.dir1.y * character.speed, z=-character.dir1.z * character.speed})
		end
		if character.q then
			character.controller:move({x=character.dir2.x * character.speed, y=character.dir2.y * character.speed, z=character.dir2.z * character.speed})
		end
		if character.e then
			character.controller:move({x=-character.dir2.x * character.speed, y=-character.dir2.y * character.speed, z=-character.dir2.z * character.speed})
		end
		if character.a then
			character.yaw = character.yaw + 1
			character:update_dir()
		end
		if character.d then
			character.yaw = character.yaw - 1
			character:update_dir()
		end
	end
))
