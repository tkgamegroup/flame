function make_player(character)
	local player = {
		character = character,
		hovered = { p=nil },
		hovered_pos = vec2(0),
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
		player.hovered_pos = vec2(pos.x, pos.z)
		local p = flame_get(pe, 0, e_type_pointer, e_else_type, 1, 1)
		flame_free(pe)

		local hovered = nil
		if p then
			hovered = make_entity(p)
		else
			hovered = { p=nil }
		end
		if hovered.p ~= player.hovered.p then
			if player.hovered.p then
				local e = player.hovered
				for i=0, e.get_children_count() - 1, 1 do
					local mesh = e.get_child(i).find_component("cMesh")
					if mesh.p then
						mesh.set_shading_flags(e_shading_material)
					end
				end
			end
			if hovered.p then
				local e = hovered
				for i=0, e.get_children_count() - 1, 1 do
					local mesh = e.get_child(i).find_component("cMesh")
					if mesh.p then
						mesh.set_shading_flags(e_shading_material + e_shading_outline)
					end
				end
			end
		end
		player.hovered = hovered
	end, 0)

	local scene_receiver = scene.find_component("cReceiver")
	
	scene_receiver.add_mouse_move_listener(function(disp, mpos)
		player.mpos = mpos
	end)

	scene_receiver.add_mouse_right_down_listener(function(mpos)
		if player.hovered == nil then
			return
		end

		local name = player.hovered.get_name()
		if name == "enemy" then
			player.character.change_state("attack_target", enemies[1].character)
		elseif name == "terrain" then
			player.character.change_state("move_to", player.hovered_pos)
		end
	end)

	return player
end
