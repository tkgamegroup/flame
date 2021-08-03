NPC_LIST = {}

local n = {
	name = "zombie",
	display_name = "Zombie",
	tag = TAG_CHARACTER_G2,
	stats = {
		HP_MAX = 500,
		MP_MAX = 500,
		HP_REC = 5,
		MP_REC = 5,
		MOV_SP = 0.06,
		ATK_DMG = 100,
		ATK_SP = 100,
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
NPC_LIST[n.name] = n

local n = {
	name = "crazy_zombie",
	display_name = "Crazy Zombie",
	tag = TAG_CHARACTER_G2,
	stats = {
		HP_MAX = 5000,
		MP_MAX = 5000,
		HP_REC = 20,
		MP_REC = 20,
		MOV_SP = 0.09,
		ATK_DMG = 200,
		ATK_SP = 100,
		ARMOR = 5
	},
	drop_gold = 50,
	drop_exp = 400,
	drop_items = {
		{ prob=0.1, id="wooden_stick", min_num=1, max_num=1 },
		{ prob=0.1, id="wooden_shield", min_num=1, max_num=1 },
		{ prob=0.1, id="leather_hat", min_num=1, max_num=1 },
		{ prob=0.1, id="leather_clothes", min_num=1, max_num=1 },
		{ prob=0.1, id="leather_pants", min_num=1, max_num=1 },
		{ prob=0.1, id="leather_shoes", min_num=1, max_num=1 },
	}
}
NPC_LIST[n.name] = n

function make_npc(e, ID)
	local data = NPC_LIST[ID]
	local npc = make_character(e, data.tag, data.stats)
	npc.drop_gold = data.drop_gold
	npc.drop_exp = data.drop_exp
	npc.drop_items = data.drop_items

	npc.chase_start_pos = vec3(-1000)
	npc.target_tick = 0

	local character_tick = npc.tick
	npc.tick = function()
		character_tick()

		if npc.state == "idle" then
			npc.target_tick = 0
		elseif npc.target_tick > 0 then
			npc.target_tick = npc.target_tick - 1
		end

		if npc.state == "idle" or npc.state == "move_to" then
			if npc.target_tick == 0 then
				local tar = npc.last_damage_src
				if tar and (npc.last_damage_src_frame + 90 < frame or distance_3(npc.pos, tar.pos) > 20) then tar = nil end
				if not tar then
					tar = npc.find_closest_obj(npc.enemy_tag, 5)
				end
				if tar then
					npc.change_state("attack_target", tar)
					npc.chase_start_pos = npc.pos
				end
			elseif math.random() < 0.002 then
				local p = vec3(npc.pos)
				p.x = p.x - 3 + math.random() * 6
				p.z = p.z - 3 + math.random() * 6
				npc.change_state("move_to", p)
			end
		elseif npc.state == "attack_target" then
			if distance_3(npc.pos, npc.chase_start_pos) > 18 or distance_3(npc.pos, npc.target.pos) > 21 then
				npc.change_state("move_to", npc.chase_start_pos)
				npc.target_tick = 600
			elseif npc.curr_anim ~= 2 then
				if npc.target_tick == 0 then
					local tar = npc.find_closest_obj(npc.enemy_tag, 5)
					if tar then
						npc.target = tar
					end
					npc.target_tick = 60
				end
				
				for i=1, SKILL_SLOTS_COUNT, 1 do
					npc.use_skill(i)
				end
			end
		end
	end
	
	local character_die = npc.die
	npc.die = function()
		if npc.last_damage_src and npc.last_damage_src.on_reward then
			npc.last_damage_src.on_reward(npc.drop_gold, npc.drop_exp)
			for i=1, #npc.drop_items, 1 do
				local item = npc.drop_items[i]
				if math.random() < item.prob then
					add_item_obj(npc.pos.to_flat() + circle_rand(1.0), item.id, math.random(item.min_num, item.max_num))
				end
			end
		end

		character_die()
	end

	return npc
end
