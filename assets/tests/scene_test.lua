local cmd = root:get_component_n("cCommand")
make_obj(cmd, "cCommand")

local er = entity:find_child("menu_view_shading_solid"):get_component_n("cEventReceiver")
make_obj(er, "cEventReceiver")
er:add_mouse_left_down_listener_s(get_slot(
	function()
		cmd:excute("view_shading_solid")
	end
))

local er = entity:find_child("menu_view_shading_wireframe"):get_component_n("cEventReceiver")
make_obj(er, "cEventReceiver")
er:add_mouse_left_down_listener_s(get_slot(
	function()
		cmd:excute("view_shading_wireframe")
	end
))

local er = entity:find_child("menu_view_physics_on"):get_component_n("cEventReceiver")
make_obj(er, "cEventReceiver")
er:add_mouse_left_down_listener_s(get_slot(
	function()
		cmd:excute("view_physics_on")
	end
))

local er = entity:find_child("menu_view_physics_off"):get_component_n("cEventReceiver")
make_obj(er, "cEventReceiver")
er:add_mouse_left_down_listener_s(get_slot(
	function()
		cmd:excute("view_physics_off")
	end
))
