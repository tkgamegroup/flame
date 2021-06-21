enemies = {}

function make_enemy(character)
	local enemy = {
		character = character
	}

	table.insert(enemies, enemy)

	return enemy
end
