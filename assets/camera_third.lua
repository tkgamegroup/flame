local node = entity:get_component_n("cNode")
make_obj(node, "cNode")

camera = {
	node = node,
	length = 5,
	yaw = 0,
	pitch = -90,
	dragging = false,
}

local root_event_receiver = root:get_component_n("cEventReceiver")
make_obj(root_event_receiver, "cEventReceiver")

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
			camera.node:set_euler({ x=camera.yaw, y=camera.pitch, z=0 })
			local dir = camera.node:get_dir(2)
			camera.node:set_pos({ x = dir.x * camera.length, y = dir.y * camera.length, z = dir.z * camera.length})
		end
	end
))
