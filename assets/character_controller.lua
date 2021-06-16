local node = entity.find_component("cNode")

local find_first_dfs_component
find_first_dfs_component = function(e, n)
	local ret = e.find_component(n)
	if ret.p then return ret end
    for i=0, e.get_children_count()-1, 1 do
		local ret = find_first_dfs_component(e.get_child(i), n)
		if ret.p then return ret end
	end
	return { p=nil }
end

local mesh = find_first_dfs_component(entity, "cMesh")

local controller = entity.find_component("cController")

character = {
	speed = 0.1,
	node = node,
	mesh = mesh,
	controller = controller,
	yaw = 0,
	dir1 = vec3(0, 0, 1),
	dir2 = vec3(1, 0, 0),
	w = false,
	s = false,
	a = false,
	d = false,
	q = false,
	e = false
}

character.update_dir = function()
	character.node.set_euler(vec3(character.yaw, 0, 0))
	character.dir1 = character.node.get_local_dir(2)
	character.dir2 = character.node.get_local_dir(0)
end

local h_animation = flame_hash("animation")
character.mesh.entity.add_component_data_listener(function(h)
	if h == h_animation then
		if character.mesh.get_animation() == "" then
			character.mesh.set_animation("stand.fani", true)
		end
	end
end, character.mesh)

character.mesh.set_animation("stand.fani", true)

local scene_receiver = scene.find_component("cReceiver")

scene_receiver.add_key_down_listener(function(k)
	if k == enums["flame::KeyboardKey"]["W"] then
		character.w = true
		character.mesh.set_animation("run.fani", true)
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

scene_receiver.add_key_up_listener(function(k)
	if k == enums["flame::KeyboardKey"]["W"] then
		character.w = false
		character.mesh.set_animation("stand.fani", true)
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

scene_receiver.add_mouse_left_up_listener(function()
	character.mesh.set_animation("attack.fani", false)
end)

entity.add_event(function()
	yaw = 0
	if character.a then
		yaw = yaw + 1
	end
	if character.d then
		yaw = yaw - 1
	end
	if yaw ~= 0 then
		character.yaw = character.yaw + yaw
		character.update_dir()
	end
	disp = vec3(0, 0, 0)
	if character.w then
		disp = disp + character.dir1 * character.speed
	end
	if character.s then
		disp = disp + character.dir1 * -character.speed
	end
	if character.q then
		disp = disp + character.dir2 * character.speed
	end
	if character.e then
		disp = disp + character.dir2 * -character.speed
	end
	if disp.x ~= 0 or disp.y ~= 0 or disp.z ~= 0 then
		character.controller.move(disp)
	end
end, 0)
