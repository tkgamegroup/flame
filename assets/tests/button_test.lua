local receiver = entity:get_component_n("cReceiver")
make_obj(receiver, "cReceiver")
receiver:add_mouse_click_listener_s(get_slot(
	function()
		print("Hello World")
	end
))
