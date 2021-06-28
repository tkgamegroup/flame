function make_player(character)
	character.hovering = { p=nil }
	character.hovering_destroyed_lis = 0
	character.hovering_pos = vec2(0)
	character.mpos = vec2(-1000)

	character.on_die = function()
		scene_receiver.remove_mouse_move_listener(character.mouse_move_list)
		scene_receiver.remove_mouse_right_down_listener(character.mouse_rightdown_list)
	end
	
	local e_shading_flags = find_enum("ShadingFlags")
	local e_shading_material = e_shading_flags["Material"]
	local e_shading_outline = e_shading_flags["Outline"]
	character.on_event = function()
		local o = camera.node.get_global_pos()
		local d = normalize_3(camera.camera.screen_to_world(character.mpos) - o)
		local pe = flame_malloc(8)
		character.hovering_pos = s_physics.raycast(o, d, pe).to_flat()
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

		if hovering.p ~= character.hovering.p then
			if character.hovering.p then
				character.hovering.remove_message_listener(character.hovering_destroyed_lis)
				change_outline(character.hovering, e_shading_material)
			end
			if hovering.p then
				change_outline(hovering, e_shading_material + e_shading_outline)
				local hash_destroyed = flame_hash("destroyed")
				character.hovering_destroyed_lis = hovering.add_message_listener(function(m)
					if m == hash_destroyed then
						character.hovering.remove_message_listener(character.hovering_destroyed_lis)
						character.hovering = { p=nil }
					end
				end)
			end

			character.hovering = hovering
		end
	end
	
	character.mouse_move_list = scene_receiver.add_mouse_move_listener(function(disp, mpos)
		character.mpos = mpos
	end)

	character.mouse_rightdown_list = scene_receiver.add_mouse_right_down_listener(function(mpos)
		if not character.hovering.p then
			return
		end

		local name = character.hovering.get_name()
		if name == "terrain" then
			if not alt_pressing then
				character.change_state("move_to", character.hovering_pos)
			else
				character.change_state("attack_on_pos", character.hovering_pos)
			end
		else
			local char = characters[2][name]
			if char then
				character.change_state("attack_target", char)
			end
		end
	end)
end
