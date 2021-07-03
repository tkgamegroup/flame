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

	player.skills = {}
	for i=1, SKILL_SLOTS_COUNT, 1 do
		player.skills[i] = nil
	end

	player.equipments = {}
	for i=1, EQUIPMENT_SLOTS_COUNT, 1 do
		player.equipments[i] = 0
	end

	player.items = {}
	for i=1, ITEM_SLOTS_COUNT, 1 do
		player.items[i] = nil
	end
	
	player.on_change_extra_state = function(s, t)
		if s == "attack_on_pos" then
			player.target_pos = t
		elseif s == "pick_up" then
			player.target = t
			player.attacking = false
		elseif s == "pick_up_on_pos" then
			player.target_pos = t
		end
	end

	player.on_process_extra_state = function()
		if player.state == "pick_up" then
			local item_obj = player.target
			if not player.move_to_pos(item_obj.pos.to_flat(), 0.2) then
				if player.receive_item(item_obj.id, item_obj.num) == 0 then
					item_obj.die()
				end
				player.change_state("idle")
			end
		elseif player.state == "attack_on_pos" then
			if not player.attacking then
				player.target = player.find_closest_obj(player.group == 1 and TAG_CHARACTER_G2 or TAG_CHARACTER_G1, 5)
			end
			
			if player.target and not player.target.dead then
				player.attack_target()
			else
				player.target = nil
				if not player.move_to_pos(player.target_pos.to_flat(), 0) then 
					player.change_state("idle")
				end
			end
		elseif player.state == "pick_up_on_pos" then
			
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

	player.receive_item = function(id, num)
		local item_type = ITEM_LIST[id]
		local max_num = item_type.stack_num
		local ori_num = num
		while num > 0 do
			for i=1, ITEM_SLOTS_COUNT, 1 do
				local slot = player.items[i]
				if slot and slot.id == id then
					local n = max_num - slot.num
					if n >= num then
						slot.num = slot.num + num
						num = 0
						break
					else
						num = num - n
						slot.num = slot.num + n
					end
				end
			end
			if num == 0 then break end
			for i=1, ITEM_SLOTS_COUNT, 1 do
				local slot = player.items[i]
				if not slot then
					slot = { id=id, num=0 }
					player.items[i] = slot
					ui_item_slots[i].image.set_tile_name(item_type.name)
					if max_num >= num then
						slot.num = max_num
						num = 0
						break
					else
						num = num - max_num
						slot.num = max_num
					end
					break
				end
			end
		end
		
		if player == main_player and num ~= ori_num then
			update_ui_item_slots()
		end
		return num
	end

	player.use_item = function(idx)
		local slot = player.items[idx]
		if slot then
			local item_type = ITEM_LIST[slot.id]
			if item_type.type == "EQUIPMENT" then
				local euip_slot = item_type.data.slot

				local ori_id = player.equipments[euip_slot]
				player.equipments[euip_slot] = slot.id
				ui_equipment_slots[euip_slot].image.set_tile_name(item_type.name)

				player.items[idx] = nil
				ui_item_slots[idx].image.set_tile_name("")

				if ori_id ~= 0 then
					player.receive_item(ori_id, 1)
				end
				player.calc_stats()

				if player == main_player then
					update_ui_equipment_slots()
				end
			end
		end
	end

	player.use_equipment = function(idx)
		local equipment = player.equipments[idx]
		if equipment ~= 0 then
			if player.receive_item(equipment, 1) == 0 then
				player.equipments[idx] = 0
				player.calc_stats()

				if player == main_player then
					update_ui_equipment_slots()
				end
			end
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
