local cmd = root:find_component("cCommand")

local er = entity:find_child("menu_shading_solid"):find_component("cReceiver")
er:add_mouse_left_down_listener_s(get_slot(
	function()
		cmd:excute("view_shading_solid")
	end
))

local er = entity:find_child("menu_shading_wireframe"):find_component("cReceiver")
er:add_mouse_left_down_listener_s(get_slot(
	function()
		cmd:excute("view_shading_wireframe")
	end
))

local er = entity:find_child("menu_physics_visualization_on"):find_component("cReceiver")
er:add_mouse_left_down_listener_s(get_slot(
	function()
		cmd:excute("view_physics_on")
	end
))

local er = entity:find_child("menu_physics_visualization_off"):find_component("cReceiver")
er:add_mouse_left_down_listener_s(get_slot(
	function()
		cmd:excute("view_physics_off")
	end
))

local er = entity:find_child("menu_capture"):find_component("cReceiver")
er:add_mouse_left_down_listener_s(get_slot(
	function()
		cmd:excute("capture")
	end
))
