scene.find_component("cReceiver").add_key_down_listener(function(k)
	if (k == enums["flame::KeyboardKey"]["F"]) then
		local e = create_entity("prefabs/sphere")
		e.find_component("cNode").set_pos(character.node.get_global_pos() + vec3(0, 3, 0))
		e.find_component("cRigid").add_impulse(character.node.get_global_dir(2) * 20)
		node_root.add_child(e.p)

		local t = 60 * 30
		e.add_event(function()
				t = t - 1
				if t == 0 then
					object_root.remove_child(e.p)
				end
		end)
	end
end)
