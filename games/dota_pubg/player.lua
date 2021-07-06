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
	
	player.on_change_extra_state = function(s, t)
		if s == "attack_on_pos" then
			player.target_pos = t
		elseif s == "pick_up_on_pos" then
			player.target_pos = t
			player.attacking = false
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

	player.on_process_extra_state = function()
		if player.state == "attack_on_pos" then
			if not player.attacking then
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

		local weapon = player.equipments[1]
		if weapon ~= 0 then
			local data = ITEM_LIST[weapon].data
			player.ATK_TYPE = data.ATK_TYPE
			if player.ATK_TYPE == "PHY" then
				player.ATK_DMG = (data.ATK + player.PHY_DMG) * 10
			else
				player.ATK_DMG = (data.ATK + player.MAG_DMG) * 10
			end
		else
			player.ATK_TYPE = "PHY"
			player.ATK_DMG = player.PHY_DMG * 10
		end
	end

	player.calc_stats()

	return player
end
