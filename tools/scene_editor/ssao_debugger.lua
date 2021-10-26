local v_radius = flame_float_var()
local v_bias = flame_float_var()
s_renderer.get_ssao_props(v_radius.p, v_bias.p)

local e_radius_drag = entity.find_child("radius_drag")
local radius_drag = e_radius_drag.find_component("cDragEdit")
radius_drag.set_float(v_radius.get_())
local e_bias_drag = entity.find_child("bias_drag")
local bias_drag = e_bias_drag.find_component("cDragEdit")
bias_drag.set_float(v_bias.get_())

e_radius_drag.add_component_data_listener(function(h)
	s_renderer.set_ssao_props(radius_drag.get_float(), bias_drag.get_float())
end, radius_drag)

e_bias_drag.add_component_data_listener(function(h)
	s_renderer.set_ssao_props(radius_drag.get_float(), bias_drag.get_float())
end, bias_drag)
