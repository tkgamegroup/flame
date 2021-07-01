function make_player(e)
	local player = make_character(e, 1, 0, 0, 0, 0, 0, "PHY")
	player.LV = 1
	player.EXP = 0
	player.EXP_NEXT = EXP_NEXT_LIST[1]
	player.STA = 10
	player.SPI = 10
	player.LUK = 10
	player.STR = 10
	player.AGI = 10
	player.INT = 10
	player.PHY_DMG = 10
	player.MAG_DMG = 10
	player.attribute_points = 0

	player.hovering = { p=nil }
	player.hovering_destroyed_lis = 0
	player.hovering_pos = vec2(0)
	player.mpos = vec2(-1000)

	player.on_die = function()
		scene_receiver.remove_mouse_move_listener(player.mouse_move_list)
		scene_receiver.remove_mouse_right_down_listener(player.mouse_rightdown_list)
	end
	
	local e_shading_flags = find_enum("ShadingFlags")
	local e_shading_material = e_shading_flags["Material"]
	local e_shading_outline = e_shading_flags["Outline"]
	player.on_event = function()
		local o = camera.node.get_global_pos()
		local d = normalize_3(camera.camera.screen_to_world(player.mpos) - o)
		local pe = flame_malloc(8)
		player.hovering_pos = s_physics.raycast(o, d, pe).to_flat()
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
	end

	player.on_reward = function(gold, exp)
		player.EXP = player.EXP + exp
		local lv = player.LV
		while player.EXP >= player.EXP_NEXT do
			player.EXP = player.EXP - player.EXP_NEXT
			player.LV = player.LV + 1
			if player.LV <= #EXP_NEXT_LIST then
				player.EXP_NEXT = EXP_NEXT_LIST[player.LV]
			else
				player.EXP = player.EXP_NEXT
				break
			end
		end
		if player.LV > lv then
			local diff = player.LV - lv
			player.STA = player.STA + diff
			player.SPI = player.SPI + diff
			player.LUK = player.LUK + diff
			player.STR = player.STR + diff
			player.AGI = player.AGI + diff
			player.INT = player.INT + diff
			player.calc_stats()
			player.attribute_points = player.attribute_points + diff * 5
		end
	end

	player.calc_stats = function()
		local pre_hp_max = player.HP_MAX
		local pre_mp_max = player.MP_MAX
		player.HP_MAX = 1000 + player.STA * 100
		player.MP_MAX = 1000 + player.SPI * 100
		if player.HP_MAX > pre_hp_max then player.receive_heal(player, player.HP_MAX - pre_hp_max) end
		if player.MP_MAX > pre_mp_max then player.MP = player.MP + (player.MP_MAX - pre_mp_max) end
		player.HP_RECOVER = player.STA
		player.MP_RECOVER = player.SPI
		player.PHY_DMG = player.STR
		player.MAG_DMG = player.INT
		if player.ATK_TYPE == "PHY" then
			player.ATK_DMG = player.PHY_DMG * 10
		else
			player.ATK_DMG = player.MAG_DMG * 10
		end
	end
	
	player.mouse_move_list = scene_receiver.add_mouse_move_listener(function(disp, mpos)
		player.mpos = mpos
	end)

	player.mouse_rightdown_list = scene_receiver.add_mouse_right_down_listener(function(mpos)
		if not player.hovering.p then
			return
		end

		local name = player.hovering.get_name()
		if name == "terrain" then
			if not alt_pressing then
				player.change_state("move_to", player.hovering_pos)
			else
				player.change_state("attack_on_pos", player.hovering_pos)
			end
		else
			local char = characters[2][name]
			if char then
				player.change_state("attack_target", char)
			end
		end
	end)

	player.calc_stats()

	return player
end
