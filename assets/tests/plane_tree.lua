local scene = entity
root.find_component("cReceiver").add_key_down_listener(function(k)
	if (k == enums["flame::KeyboardKey"]["T"]) then
		local e = find_udt("Entity").static_functions.create()
		e.load("D:/tree02/tree")

		local pos = character.node.get_global_pos()
		pos.y = -100
		pos = s_physics.raycast(pos, vec3(0, 1, 0))

		e.find_component("cNode").set_pos(pos)
		scene.add_child(e.p)
	end
end)
