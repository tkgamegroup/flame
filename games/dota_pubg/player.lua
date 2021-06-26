function make_player(character)
	local player = {
		character = character,
		hovering = { p=nil },
		hovering_destroyed_lis = 0,
		hovering_pos = vec2(0),
		mpos = vec2(-1000)
	}
	
	local e_shading_flags = find_enum("ShadingFlags")
	local e_shading_material = e_shading_flags["Material"]
	local e_shading_outline = e_shading_flags["Outline"]
	player.character.entity.add_event(function()
		local o = camera.node.get_global_pos()
		local d = normalize_3(camera.camera.screen_to_world(player.mpos) - o)
		local pe = flame_malloc(8)
		local pos = s_physics.raycast(o, d, pe)
		player.hovering_pos = vec2(pos.x, pos.z)
		local p = flame_get(pe, 0, e_type_pointer, e_else_type, 1, 1)
		flame_free(pe)

		local hovering = nil
		if p then
			hovering = make_entity(p)
		else
			hovering = { p=nil }
		end

		function change_outline(e, f)
			for i=0, e.get_children_count() - 1, 1 do
				local mesh = e.get_child(i).find_component("cMesh")
				if mesh.p then
					mesh.set_shading_flags(f)
				end
			end
		end
		if hovering.p ~= player.hovering.p then
			if player.hovering.p then
				player.hovering.remove_message_listener(player.hovering_destroyed_lis)
				change_outline(player.hovering, e_shading_material)
			end
			if hovering.p then
				change_outline(hovering, e_shading_material + e_shading_outline)
				local hash_destroyed = flame_hash("destroyed")
				player.hovering_destroyed_lis = hovering.add_message_listener(function(m)
					if m == hash_destroyed then
						player.hovering.remove_message_listener(player.hovering_destroyed_lis)
						player.hovering = { p=nil }
					end
				end)
			end

			player.hovering = hovering
		end
	end, 0)

	local scene_receiver = scene.find_component("cReceiver")
	
	scene_receiver.add_mouse_move_listener(function(disp, mpos)
		player.mpos = mpos
	end)

	scene_receiver.add_mouse_right_down_listener(function(mpos)
		if player.hovering == nil then
			return
		end

		local name = player.hovering.get_name()
		if starts_with(name, "enemy_") then
			name = string.sub(name, 7)
			local enemy = enemies[name]
			if enemy then
				player.character.change_state("attack_target", enemy.character)
			end
		elseif name == "terrain" then
			player.character.change_state("move_to", player.hovering_pos)
		end
	end)

	return player
end