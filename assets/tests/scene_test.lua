local cmd = root:get_component_n("cCommand")
make_obj(cmd, "cCommand")

local er = entity:find_child("menu_shading_solid"):get_component_n("cReceiver")
make_obj(er, "cReceiver")
er:add_mouse_left_down_listener_s(get_slot(
	function()
		cmd:excute("view_shading_solid")
	end
))

local er = entity:find_child("menu_shading_wireframe"):get_component_n("cReceiver")
make_obj(er, "cReceiver")
er:add_mouse_left_down_listener_s(get_slot(
	function()
		cmd:excute("view_shading_wireframe")
	end
))

local er = entity:find_child("menu_physics_visualization_on"):get_component_n("cReceiver")
make_obj(er, "cReceiver")
er:add_mouse_left_down_listener_s(get_slot(
	function()
		cmd:excute("view_physics_on")
	end
))

local er = entity:find_child("menu_physics_visualization_off"):get_component_n("cReceiver")
make_obj(er, "cReceiver")
er:add_mouse_left_down_listener_s(get_slot(
	function()
		cmd:excute("view_physics_off")
	end
))

local er = entity:find_child("menu_capture"):get_component_n("cReceiver")
make_obj(er, "cReceiver")
er:add_mouse_left_down_listener_s(get_slot(
	function()
		cmd:excute("capture")
	end
))
