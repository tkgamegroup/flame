projectiles = {}

function make_projectile(entity, target, speed)
	local projectile = {
		name = entity.get_name(),

		entity = entity,
		node = entity.find_component("cNode"),
		pos = vec3(0),

		target = target,
		speed = speed
	}

	projectile.die = function()
		projectiles[projectile.name] = nil

		projectile.entity.get_parent().remove_child(projectile.entity)
	end

	projectiles[projectile.name] = projectile

	return projectile
end
