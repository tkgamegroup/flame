local node = entity.find_component("cNode")
local pnode = entity.get_parent().find_component("cNode")

camera = {
	node = node,
	length = 5,
	yaw = 0,
	pitch = -90,
	dragging = false
}

camera.set_pos = function ()
	local off = vec3(0, 3, 0)
	local d = 0
	if s_physics.p then
		local o = pnode.get_global_pos() + off
		local p = s_physics.raycast(o, camera.node.get_global_dir(2))
		d = v3_distance(o, p)
		if d < camera.length then
			d = d - 1
		else
			d = camera.length
		end
	else
		d = camera.length
	end
	if d < 1 then d = 1 end
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
		if camera.length > 5 then
			camera.length = camera.length - 1
			camera.set_pos()
		end
	else
		if camera.length < 100 then
			camera.length = camera.length + 1
			camera.set_pos()
		end
	end
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
