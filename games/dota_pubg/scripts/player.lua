EXP_NEXT_LIST = {
	200,
	300,
	400,
	500,
	600,
	700,
	800,
	900
}

function make_player(e)
	local player = make_character(e, TAG_CHARACTER_G1)
	player.LV = 1
	player.EXP = 0
	player.EXP_NEXT = EXP_NEXT_LIST[1]
	player.GOLD = 0
	player.STA = new_attribute(10) -- stamina, increase hp max and hp recover
	player.SPI = new_attribute(10) -- spirit, increase mp max and mp recover
	player.LUK = new_attribute(10) -- luck,
	player.STR = new_attribute(10) -- strength, increase physical damage by percentage
	player.AGI = new_attribute(10) -- agile, increase attack speed
	player.INT = new_attribute(10) -- intelligence, increase physical damage by percentage
	player.attribute_points = 0
	
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
			player.STA.b = player.STA.b + diff
			player.SPI.b = player.SPI.b + diff
			player.LUK.b = player.LUK.b + diff
			player.STR.b = player.STR.b + diff
			player.AGI.b = player.AGI.b + diff
			player.INT.b = player.INT.b + diff
			player.calc_stats()
			player.attribute_points = player.attribute_points + diff * 5
		end
	end
	
	local character_calc_stats = player.calc_stats
	player.calc_stats = function()
		player.ATK_TYPE = "Physical" 
		local id = player.equipments[EQUIPMENT_SLOT_MAIN_HAND]
		if id then
			player.ATK_TYPE = ITEM_LIST[id].attributes.ATK_TYPE
		end
		
		player.collect_attribute("STA")
		player.STA.calc()
		player.collect_attribute("SPI")
		player.SPI.calc()
		player.collect_attribute("LUK")
		player.LUK.calc()
		player.collect_attribute("STR")
		player.STR.calc()
		player.collect_attribute("AGI")
		player.AGI.calc()
		player.collect_attribute("INT")
		player.INT.calc()

		player.HP_MAX.b = 1000 + player.STA.t * 100
		player.MP_MAX.b = 1000 + player.SPI.t * 100
		player.HP_REC.b = player.STA.t
		player.MP_REC.b = player.SPI.t

		player.MOV_SP.d = 0.06

		player.PHY_INC.b = player.STR.t
		player.ATK_SP.d = 100
		player.ATK_SP.b = player.AGI.t
		player.MAG_INC.b = player.INT.t

		character_calc_stats()
	end

	player.calc_stats()

	return player
end
