enemies = {}

function make_enemy(character)
	local enemy = {
		character = character,
		chase_start_pos,
		chase_tick  = 0
	}

	local name = enemy.character.entity.get_name()
	name = string.sub(name, 7)
	enemy.character.on_die = function()
		enemies[name] = nil
	end

	enemy.event = enemy.character.entity.add_event(function()
		if enemy.chase_tick > 0 then
			enemy.chase_tick = enemy.chase_tick - 1
		end

		if enemy.chase_tick == 0 and distance_3(enemy.character.pos, main_player.character.pos) < 5 then
			enemy.character.change_state("attack_target", main_player.character)
			enemy.chase_start_pos = enemy.character.pos
			enemy.chase_tick = 600
		end
		if enemy.character.state == "attack_target" and distance_3(enemy.character.pos, enemy.chase_start_pos) > 20 then
			local p = enemy.chase_start_pos
			enemy.character.change_state("move_to", vec2(p.x, p.z))
			enemy.chase_tick = 600
		end
		if enemy.character.state == "idle" then
			if math.random() < 0.002 then
				local p = enemy.character.pos
				p.x = p.x - 3 + math.random() * 6
				p.z = p.z - 3 + math.random() * 6
				enemy.character.change_state("move_to", vec2(p.x, p.z))
			end
		end
	end, 0)

	enemies[name] = enemy

	return enemy
end
