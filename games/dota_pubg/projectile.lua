projectiles = {}

function make_projectile(entity, target, pos, speed, callback)
	local projectile = {
		name = entity.get_name(),

		entity = entity,
		node = entity.find_component("cNode"),
		pos = pos,

		target = target,
		speed = speed,

		callback = callback
	}

	projectile.node.set_pos(pos)

	projectile.die = function()
		if projectile.callback then
			projectile.callback()
		end

		projectiles[projectile.name] = nil

		projectile.entity.get_parent().remove_child(projectile.entity)
	end

	projectiles[projectile.name] = projectile

	return projectile
end
