local node = entity.find_component("cNode")

controller = {
	speed = 0.5,
	node = node,
	pos = node.get_pos(),
	yaw = 0,
	pitch = 0,
	_0 = false,
	_1 = false,
	_2 = false,
	_3 = false,
	_4 = false,
	_5 = false,
	_6 = false,
	_7 = false,
	_8 = false,
	_9 = false
}

local scene_receiver = scene.find_component("cReceiver")

scene_receiver.add_key_down_listener_s(get_slot(
	function(k)
		if k == enums["flame::KeyboardKey"]["Numpad0"] then
			controller._0 = true
		end
		if k == enums["flame::KeyboardKey"]["Numpad1"] then
			controller._1 = true
		end
		if k == enums["flame::KeyboardKey"]["Numpad2"] then
			controller._2 = true
		end
		if k == enums["flame::KeyboardKey"]["Numpad3"] then
			controller._3 = true
		end
		if k == enums["flame::KeyboardKey"]["Numpad4"] then
			controller._4 = true
		end
		if k == enums["flame::KeyboardKey"]["Numpad5"] then
			controller._5 = true
		end
		if k == enums["flame::KeyboardKey"]["Numpad6"] then
			controller._6 = true
		end
		if k == enums["flame::KeyboardKey"]["Numpad7"] then
			controller._7 = true
		end
		if k == enums["flame::KeyboardKey"]["Numpad8"] then
			controller._8 = true
		end
		if k == enums["flame::KeyboardKey"]["Numpad9"] then
			controller._9 = true
		end
	end
))

scene_receiver.add_key_up_listener_s(get_slot(
	function(k)
		if k == enums["flame::KeyboardKey"]["Numpad0"] then
			controller._0 = false
		end
		if k == enums["flame::KeyboardKey"]["Numpad1"] then
			controller._1 = false
		end
		if k == enums["flame::KeyboardKey"]["Numpad2"] then
			controller._2 = false
		end
		if k == enums["flame::KeyboardKey"]["Numpad3"] then
			controller._3 = false
		end
		if k == enums["flame::KeyboardKey"]["Numpad4"] then
			controller._4 = false
		end
		if k == enums["flame::KeyboardKey"]["Numpad5"] then
			controller._5 = false
		end
		if k == enums["flame::KeyboardKey"]["Numpad6"] then
			controller._6 = false
		end
		if k == enums["flame::KeyboardKey"]["Numpad7"] then
			controller._7 = false
		end
		if k == enums["flame::KeyboardKey"]["Numpad8"] then
			controller._8 = false
		end
		if k == enums["flame::KeyboardKey"]["Numpad9"] then
			controller._9 = false
		end
	end
))

entity.add_event(function()
		if controller._8 then
			controller.pos.x = controller.pos.x + controller.speed
			controller.node.set_pos(controller.pos)
		end
		if controller._5 then
			controller.pos.x = controller.pos.x - controller.speed
			controller.node.set_pos(controller.pos)
		end
		if controller._4 then
			controller.pos.z = controller.pos.z - controller.speed
			controller.node.set_pos(controller.pos)
		end
		if controller._6 then
			controller.pos.z = controller.pos.z + controller.speed
			controller.node.set_pos(controller.pos)
		end
		if controller._7 then
			controller.pos.y = controller.pos.y - controller.speed
			controller.node.set_pos(controller.pos)
		end
		if controller._9 then
			controller.pos.y = controller.pos.y + controller.speed
			controller.node.set_pos(controller.pos)
		end
		if controller._0 then
			controller.yaw = controller.yaw + 1
			controller.node.set_euler(vec3(controller.yaw, controller.pitch, 0))
		end
		if controller._1 then
			controller.yaw = controller.yaw - 1
			controller.node.set_euler(vec3(controller.yaw, controller.pitch, 0))
		end
		if controller._2 then
			controller.pitch = controller.pitch + 1
			controller.node.set_euler(vec3(controller.yaw, controller.pitch, 0))
		end
		if controller._3 then
			controller.pitch = controller.pitch - 1
			controller.node.set_euler(vec3(controller.yaw, controller.pitch, 0))
		end
end)
