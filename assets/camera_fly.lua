local node = entity:get_component_n("cNode")
make_obj(node, "cNode")

camera = {
	speed = 0.5,
	node = node,
	pos = node:get_pos(),
	yaw = 0,
	pitch = -90,
	dir1 = { x=0, y=0, z=1 },
	dir2 = { x=1, y=0, z=0 },
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

function camera:move(dir, v)
	self.pos.x = self.pos.x + dir.x * v
	self.pos.y = self.pos.y + dir.y * v
	self.pos.z = self.pos.z + dir.z * v
	self.node:set_pos(self.pos)
end

function camera:update_dir()
	self.node:set_euler({ x=self.yaw, y=self.pitch, z=0 })
	self.dir1 = self.node:get_dir(2)
	self.dir2 = self.node:get_dir(0)
end

local root_event_receiver = root:get_component_n("cEventReceiver")
make_obj(root_event_receiver, "cEventReceiver")

root_event_receiver:add_key_down_listener_s(get_slot(
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

root_event_receiver:add_key_up_listener_s(get_slot(
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

root_event_receiver:add_mouse_left_down_listener_s(get_slot(
	function()
		camera.dragging = true
	end
))

root_event_receiver:add_mouse_left_up_listener_s(get_slot(
	function()
		camera.dragging = false
	end
))

root_event_receiver:add_mouse_move_listener_s(get_slot(
	function(disp)
		if camera.dragging then
			camera.yaw = camera.yaw - disp.x
			camera.pitch = camera.pitch - disp.y
			camera:update_dir()
		end
	end
))

entity:add_event_s(get_slot(
	function()
		if camera.w then
			camera:move(camera.dir1, -camera.speed)
		end
		if camera.s then
			camera:move(camera.dir1, camera.speed)
		end
		if camera.a then
			camera:move(camera.dir2, -camera.speed)
		end
		if camera.d then
			camera:move(camera.dir2, camera.speed)
		end
		if camera.sp then
			camera.pos.y = camera.pos.y + camera.speed
			camera.node:set_pos(camera.pos)
		end
		if camera.sh then
			camera.pos.y = camera.pos.y - camera.speed
			camera.node:set_pos(camera.pos)
		end
	end
))
