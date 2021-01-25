local scene = entity:find_child("scene")
root:find_component("cReceiver"):add_key_down_listener(function(k)
--[[
	if (k == enums["flame::KeyboardKey"]["F"]) then
		local e = flame_call(nil, find_udt("Entity")["create"])
		e:load("sphere")
			
		local node = e:find_component("cNode")
		node:set_pos(camera.node:get_global_pos())
			
		local rigid = e:find_component("cRigid")
		local dir = camera.node:get_global_dir(2)
		rigid:add_impulse({x=-dir.x*5, y=-dir.y*5, z=-dir.z*5})
			
		scene:add_child(e.p)
			
		local t = 60 * 30
		e:add_event(function()
				t = t - 1
				if t == 0 then
					scene:remove_child(e.p)
				end
		end)
	end
--]]
end)
