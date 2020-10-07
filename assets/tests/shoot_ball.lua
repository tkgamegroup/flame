local scene = entity:find_child("scene")
local er = root:get_component_n("cEventReceiver")
make_obj(er, "cEventReceiver")
er:add_key_down_listener_s(get_slot(
	function(k)
		if (k == enums["flame::KeyboardKey"]["F"]) then
			local e = flame_call(nil, find_udt("Entity")["create"])
			e:load("sphere")
			
			local node = e:get_component_n("cNode")
			make_obj(node, "cNode")
			node:set_pos(camera.pos)
			
			local rigid = e:get_component_n("cRigid")
			make_obj(rigid, "cRigid")
			rigid:add_impulse({x=-camera.dir1.x*5, y=-camera.dir1.y*5, z=-camera.dir1.z*5})
			
			scene:add_child(e.p)
			
			local t = 60 * 30
			e:add_event_s(get_slot(
				function()
					t = t - 1
					if t == 0 then
						scene:remove_child(e.p)
					end
				end
			))
		end
	end
))
