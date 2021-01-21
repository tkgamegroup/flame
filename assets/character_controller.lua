local node = entity:find_component("cNode")

local mesh = entity:find_child("mesh"):find_component("cMesh")

local controller = entity:find_component("cController")

character = {
	speed = 0.1,
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

local root_receiver = root:find_component("cReceiver")

root_receiver:add_key_down_listener(function(k)
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
end)

root_receiver:add_key_up_listener(function(k)
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
end)

entity:add_event(function()
	yaw = 0
	if character.a then
		yaw = yaw + 1
	end
	if character.d then
		yaw = yaw - 1
	end
	if yaw ~= 0 then
		character.yaw = character.yaw + yaw
		character:update_dir()
	end
	disp = {x=0, y=0, z=0}
	if character.w then
		disp.x = disp.x + character.dir1.x * character.speed
		disp.y = disp.y + character.dir1.y * character.speed
		disp.z = disp.z + character.dir1.z * character.speed
	end
	if character.s then
		disp.x = disp.x - character.dir1.x * character.speed
		disp.y = disp.y - character.dir1.y * character.speed
		disp.z = disp.z - character.dir1.z * character.speed
	end
	if character.q then
		disp.x = disp.x + character.dir2.x * character.speed
		disp.y = disp.y + character.dir2.y * character.speed
		disp.z = disp.z + character.dir2.z * character.speed
	end
	if character.e then
		disp.x = disp.x - character.dir2.x * character.speed
		disp.y = disp.y - character.dir2.y * character.speed
		disp.z = disp.z - character.dir2.z * character.speed
	end
	if disp.x ~= 0 or disp.y ~= 0 or disp.z ~= 0 then
		character.controller:move(disp)
	end
end)
