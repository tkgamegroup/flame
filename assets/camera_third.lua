local node = entity:get_component_n("cNode")
make_obj(node, "cNode")

camera = {
	node = node,
	length = 5,
	yaw = 0,
	pitch = 0,
	dragging = false,
}

function camera:set_pos()
	local dir = self.node:get_local_dir(2)
	self.node:set_pos({x=dir.x*self.length, y=dir.y*self.length, z=dir.z*self.length})
end

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

root_event_receiver:add_mouse_scroll_listener_s(get_slot(
	function(scroll)
		if scroll > 0 then
			if camera.length > 5 then
				camera.length = camera.length - 1
				camera:set_pos()
			end
		else
			if camera.length < 40 then
				camera.length = camera.length + 1
				camera:set_pos()
			end
		end
	end
))

root_event_receiver:add_mouse_move_listener_s(get_slot(
	function(disp)
		if camera.dragging then
			camera.yaw = camera.yaw - disp.x
			camera.pitch = camera.pitch - disp.y
			camera.node:set_euler({ x=camera.yaw, y=camera.pitch, z=0 })
			camera:set_pos()
		end
	end
))
