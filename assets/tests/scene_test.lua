local er = entity:find_child("menu_view_terrain_normal"):get_component_n("cEventReceiver")
make_obj(er, "cEventReceiver")
er:add_mouse_left_down_listener_s(get_slot(
	function()
		print("123")
	end
))
