local root_event_receiver = root:get_component_n("cEventReceiver")
make_obj(root_event_receiver, "cEventReceiver")
root_event_receiver:add_mouse_click_listener_s(get_slot(
	function()
		local e = flame_call({}, root.p, find_udt("Entity")["create"])
		e:load("assets/ball")
		root:add_child(e.p)
	end
))
