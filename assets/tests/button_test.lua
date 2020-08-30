local event_receiver = entity:get_component_n("cEventReceiver")
make_obj(event_receiver, "cEventReceiver")

event_receiver:add_mouse_click_listener_s(get_slot(
	function()
		print("Hello World")
	end
))
