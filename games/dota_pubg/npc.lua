NPC_LIST = {
	{
		stats = {
			HP_MAX = 500,
			MP_MAX = 500,
			HP_RECOVER = 5,
			MP_RECOVER = 5,
			ATK_DMG = 100,
			ATK_TYPE = "PHY"
		},
		drop_gold = 10,
		drop_exp = 40,
		drop_items = {
			{ prob=1, id=1, min_num=1, max_num=1 }
		}
	}
}

function make_npc(e, ID)
	local data = NPC_LIST[ID]
	local npc = make_character(e, 2, data.stats)
	npc.drop_gold = data.drop_gold
	npc.drop_exp = data.drop_exp
	npc.drop_items = data.drop_items

	npc.chase_start_pos = vec2(-1000)
	npc.chase_tick = 0

	npc.on_event = function()
		if npc.chase_tick > 0 then
			npc.chase_tick = npc.chase_tick - 1
		end
		
		if npc.chase_tick == 0 then
			local char = npc.find_closest_enemy(5)
			if char then
				npc.change_state("attack_target", char)
				npc.chase_start_pos = npc.pos
				npc.chase_tick = 600
			end
		end
		if npc.state == "attack_target" and distance_3(npc.pos, npc.chase_start_pos) > 20 then
			local p = npc.chase_start_pos
			npc.change_state("move_to", vec2(p.x, p.z))
			npc.chase_tick = 600
		end
		if npc.state == "idle" then
			if math.random() < 0.002 then
				local p = npc.pos
				p.x = p.x - 3 + math.random() * 6
				p.z = p.z - 3 + math.random() * 6
				npc.change_state("move_to", vec2(p.x, p.z))
			end
		end
	end

	npc.on_die = function()
		if npc.last_hit_character then
			if npc.last_hit_character.on_reward then
				npc.last_hit_character.on_reward(npc.drop_gold, npc.drop_exp)
				for i=1, #npc.drop_items, 1 do
					local item = npc.drop_items[i]
					if math.random() < item.prob then
						add_chest(npc.pos + vec3(0, 2, 0), item.id, math.random(item.min_num, item.max_num))
					end
				end
			end
		end
	end

	return npc
end
