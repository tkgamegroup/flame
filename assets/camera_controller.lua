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
	e = false
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
	end
))
entity:add_event_s(get_slot(
	function()
		if camera.w then
			camera.pos.z = camera.pos.z + 0.5
			camera.node:set_pos(camera.pos)
		end
		if camera.s then
			camera.pos.z = camera.pos.z - 0.5
			camera.node:set_pos(camera.pos)
		end
	end
))