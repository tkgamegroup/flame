enemies = {}

function make_enemy(character)
	local enemy = {
		character = character
	}

	enemy.event = enemy.character.entity.add_event(function()
		if enemy.character.state == "idle" then
			if math.random() < 0.002 then
				local pos = enemy.character.node.get_global_pos()
				pos.x = pos.x - 3 + math.random() * 6
				pos.z = pos.z - 3 + math.random() * 6
				enemy.character.change_state("move_to", vec2(pos.x, pos.z))
			end
		end
	end, 0)

	local name = enemy.character.entity.get_name()
	name = string.sub(name, 7)
	enemy.character.on_die = function()
		enemies[name] = nil
	end

	enemies[name] = enemy

	return enemy
end
