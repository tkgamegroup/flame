local node = entity:get_component_n("cNode")
make_obj(node, "cNode")

camera = {
	node = node,
	pos = node:get_pos(),
	length = 5,
	yaw = 0,
	pitch = 0,
	dir1 = { x=0, y=0, z=1 },
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
			camera.dir1 = camera.node:get_global_dir(2)
			camera.node:set_pos({x=camera.dir1.x*camera.length, y=camera.dir1.y*camera.length, z=camera.dir1.z*camera.length})
			camera.pos = camera.node:get_global_pos()
		end
	end
))
