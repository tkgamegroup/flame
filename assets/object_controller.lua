local node = entity:get_component_n("cNode")
make_obj(node, "cNode")

controller = {
	node = node,
	pos = node:get_pos(),
	_8 = false,
	_5 = false,
	_4 = false,
	_6 = false,
	_7 = false,
	_9 = false
}

local root_event_receiver = root:get_component_n("cEventReceiver")
make_obj(root_event_receiver, "cEventReceiver")

root_event_receiver:add_key_down_listener_s(get_slot(
	function(k)
		if k == enums["flame::KeyboardKey"]["Numpad8"] then
			controller._8 = true
		end
		if k == enums["flame::KeyboardKey"]["Numpad5"] then
			controller._5 = true
		end
		if k == enums["flame::KeyboardKey"]["Numpad4"] then
			controller._4 = true
		end
		if k == enums["flame::KeyboardKey"]["Numpad6"] then
			controller._6 = true
		end
		if k == enums["flame::KeyboardKey"]["Numpad7"] then
			controller._7 = true
		end
		if k == enums["flame::KeyboardKey"]["Numpad9"] then
			controller._9 = true
		end
	end
))

root_event_receiver:add_key_up_listener_s(get_slot(
	function(k)
		if k == enums["flame::KeyboardKey"]["Numpad8"] then
			controller._8 = false
		end
		if k == enums["flame::KeyboardKey"]["Numpad5"] then
			controller._5 = false
		end
		if k == enums["flame::KeyboardKey"]["Numpad4"] then
			controller._4 = false
		end
		if k == enums["flame::KeyboardKey"]["Numpad6"] then
			controller._6 = false
		end
		if k == enums["flame::KeyboardKey"]["Numpad7"] then
			controller._7 = false
		end
		if k == enums["flame::KeyboardKey"]["Numpad9"] then
			controller._9 = false
		end
	end
))

entity:add_event_s(get_slot(
	function()
		if controller._8 then
			controller.pos.x = controller.pos.x + 0.1
			controller.node:set_pos(controller.pos)
		end
		if controller._5 then
			controller.pos.x = controller.pos.x - 0.1
			controller.node:set_pos(controller.pos)
		end
		if controller._4 then
			controller.pos.z = controller.pos.z - 0.1
			controller.node:set_pos(controller.pos)
		end
		if controller._6 then
			controller.pos.z = controller.pos.z + 0.1
			controller.node:set_pos(controller.pos)
		end
		if controller._7 then
			controller.pos.y = controller.pos.y - 0.1
			controller.node:set_pos(controller.pos)
		end
		if controller._9 then
			controller.pos.y = controller.pos.y + 0.1
			controller.node:set_pos(controller.pos)
		end
	end
))
