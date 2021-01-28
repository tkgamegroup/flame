local scene = entity
root.find_component("cReceiver").add_key_down_listener(function(k)
	if (k == enums["flame::KeyboardKey"]["F"]) then
		local e = find_udt("Entity").static_functions.create()
		e.load("prefabs/sphere")
			
		local node = e.find_component("cNode")
		node.set_pos(camera.node.get_global_pos())
			
		local rigid = e.find_component("cRigid")
		local dir = camera.node.get_global_dir(2)
		rigid.add_impulse(vec3(-dir.x*5, -dir.y*5, -dir.z*5))
			
		scene.add_child(e.p)
			
		local t = 60 * 30
		e.add_event(function()
				t = t - 1
				if t == 0 then
					scene.remove_child(e.p)
				end
		end)
	end
end)
