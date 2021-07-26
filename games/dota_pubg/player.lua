EXP_NEXT_LIST = {
	100,
	200
}

function make_player(e)
	local player = make_character(e, 1)
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
	
	local character_change_state = player.change_state
	player.change_state = function(s, t, d)
		if s == "attack_on_pos" then
			player.target_pos = t
			player.state = s
		elseif s == "pick_up_on_pos" then
			player.target_pos = t
			player.state = s
		else
			character_change_state(s, t, d)
		end
	end

	player.pick_up_target = function()
		local item_obj = player.target
		if player.move_to_pos(item_obj.pos.to_flat(), 0.2) then
			if player.receive_item(item_obj.id, item_obj.num) == 0 then
				item_obj.die()
			end
			return true
		end
		return false
	end

	local character_process_state = player.process_state
	player.process_state = function()
		if player.state == "attack_on_pos" then
			if player.curr_anim ~= 2 then
				player.target = player.find_closest_obj(player.group == 1 and TAG_CHARACTER_G2 or TAG_CHARACTER_G1, 5)
			end
			
			if player.target and not player.target.dead then
				player.attack_target()
			else
				player.target = nil
				if player.move_to_pos(player.target_pos.to_flat(), 0) then 
					player.change_state("idle")
				end
			end
		elseif player.state == "pick_up_on_pos" then
			player.target = player.find_closest_obj(TAG_ITEM_OBJ, 5)
			
			if player.target and not player.target.dead then
				player.pick_up_target()
			else
				player.target = nil
				if player.move_to_pos(player.target_pos.to_flat(), 0) then 
					player.change_state("idle")
				end
			end
		else
			character_process_state()
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
		player.ATK = 0
		player.MOV_SP = 100

		for i=1, EQUIPMENT_SLOTS_COUNT, 1 do
			local id = player.equipments[i]
			if id ~= 0 then
				local item_type = ITEM_LIST[id]
				for k, v in pairs(item_type.attributes) do
					player[k] = player[k] + v
				end
			end
		end

		local pre_hp_ratio = player.HP_MAX > 0 and player.HP / player.HP_MAX or 1
		local pre_mp_ratio = player.MP_MAX > 0 and player.MP / player.MP_MAX or 1
		player.HP_MAX = 1000 + player.STA * 100
		player.MP_MAX = 1000 + player.SPI * 100
		player.HP = pre_hp_ratio * player.HP_MAX
		player.MP = pre_mp_ratio * player.MP_MAX
		player.HP_RECOVER = player.STA
		player.MP_RECOVER = player.SPI
		player.PHY_DMG = player.STR
		player.MAG_DMG = player.INT

		player.ATK = (player.ATK + player.PHY_DMG) * 10
	end

	player.calc_stats()

	return player
end
