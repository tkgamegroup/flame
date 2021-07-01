NPC_STATS = {
	{
		HP_MAX = 1000,
		MP_MAX = 500,
		HP_RECOVER = 5,
		MP_RECOVER = 5,
		ATK_DMG = 100,
		ATK_TYPE = "PHY"
	}
}

function make_npc(e, ID)
	local stats = NPC_STATS[ID]
	local npc = make_character(e, 2, stats.HP_MAX, stats.MP_MAX, stats.HP_RECOVER, stats.MP_RECOVER, stats.ATK_DMG, stats.ATK_TYPE)

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

	return npc
end
