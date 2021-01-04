local NDF_s = entity:find_child("NDF_s")
local NDF_t = entity:find_child("NDF_t")
local NDF_s_s = NDF_s:find_component("cSlider")
local NDF_t_t = NDF_t:find_component("cText")

local hValue = flame_hash("value")
NDF_s:add_local_data_changed_listener_s(get_slot(
	function(c, h)
		if c == NDF_s_s.p and h == hValue then
			local roughness = NDF_s_s:get_value()
			local a = roughness * roughness
			local a2 = a * a
			local NdotH = 1.0
			local NdotH2 = NdotH * NdotH
			local nom = a2
			local denom = (NdotH2 * (a2 - 1.0) + 1.0)
			denom = math.pi * denom * denom
			local res = nom / denom
			NDF_t_t:set_text("NDF(ggx): "..tostring(res))
		end
	end
))
