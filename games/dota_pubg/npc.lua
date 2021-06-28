function make_npc(character)
	character.chase_start_pos = vec2(-1000)
	character.chase_tick = 0

	character.on_event = function()
		if character.chase_tick > 0 then
			character.chase_tick = character.chase_tick - 1
		end
		
		if character.chase_tick == 0 then
			local char = character.find_closest_enemy(5)
			if char then
				character.change_state("attack_target", char)
				character.chase_start_pos = character.pos
				character.chase_tick = 600
			end
		end
		if character.state == "attack_target" and distance_3(character.pos, character.chase_start_pos) > 20 then
			local p = character.chase_start_pos
			character.change_state("move_to", vec2(p.x, p.z))
			character.chase_tick = 600
		end
		if character.state == "idle" then
			if math.random() < 0.002 then
				local p = character.pos
				p.x = p.x - 3 + math.random() * 6
				p.z = p.z - 3 + math.random() * 6
				character.change_state("move_to", vec2(p.x, p.z))
			end
		end
	end
end
