scene.find_component("cReceiver").add_key_down_listener(function(k)
	if (k == enums["flame::KeyboardKey"]["T"]) then
		local e = create_entity("D:/tree02/tree")

		local pos = character.node.get_global_pos()
		pos.y = -100
		pos = s_physics.raycast(pos, vec3(0, 1, 0))

		local node = e.find_component("cNode")
		node.set_pos(pos)
		--node.set_quat(character.node.get_quat())
		node_root.add_child(e.p)
	end
end)
