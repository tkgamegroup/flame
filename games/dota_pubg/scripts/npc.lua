NPC_LIST = {
	{
		name = "zombie",
		display_name = "Zombie",
		stats = {
			HP_MAX = 500,
			MP_MAX = 500,
			HP_RECOVER = 5,
			MP_RECOVER = 5,
			MOV_SP = 100,
			ATK = 100,
			ARMOR = 2
		},
		drop_gold = 10,
		drop_exp = 40,
		drop_items = {
			{ prob=0.1, id="wooden_stick", min_num=1, max_num=1 },
			{ prob=0.1, id="wooden_shield", min_num=1, max_num=1 },
			{ prob=0.1, id="leather_hat", min_num=1, max_num=1 },
			{ prob=0.1, id="leather_clothes", min_num=1, max_num=1 },
			{ prob=0.1, id="leather_pants", min_num=1, max_num=1 },
			{ prob=0.1, id="leather_shoes", min_num=1, max_num=1 },
		}
	}
}

function make_npc(e, data)
	local npc = make_character(e, 2, data.stats)
	npc.drop_gold = data.drop_gold
	npc.drop_exp = data.drop_exp
	npc.drop_items = data.drop_items

	npc.chase_start_pos = vec2(-1000)
	npc.chase_tick = 0

	local character_tick = npc.tick
	npc.tick = function()
		character_tick()

		if npc.state == "attack_target" and distance_3(npc.pos, npc.chase_start_pos) > 18 then
			npc.change_state("move_to", npc.chase_start_pos)
		end

		if npc.state == "idle" then
			local char = npc.find_closest_obj(npc.group == 1 and TAG_CHARACTER_G2 or TAG_CHARACTER_G1, 5)
			if char then
				npc.change_state("attack_target", char)
				npc.chase_start_pos = npc.pos
			elseif math.random() < 0.002 then
				local p = vec3(npc.pos)
				p.x = p.x - 3 + math.random() * 6
				p.z = p.z - 3 + math.random() * 6
				npc.change_state("move_to", p)
			end
		end
	end
	
	local character_die = npc.die
	npc.die = function()
		if npc.last_receive_damage_src then
			if npc.last_receive_damage_src.on_reward then
				npc.last_receive_damage_src.on_reward(npc.drop_gold, npc.drop_exp)
				for i=1, #npc.drop_items, 1 do
					local item = npc.drop_items[i]
					if math.random() < item.prob then
						add_item_obj(npc.pos.to_flat() + circle_rand(1.0), item.id, math.random(item.min_num, item.max_num))
					end
				end
			end
		end

		character_die()
	end

	return npc
end
