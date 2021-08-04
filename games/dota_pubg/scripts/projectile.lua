projectiles = {}

function make_projectile(entity, target, speed, callback)
	local projectile = {
		name = entity.get_name(),

		entity = entity,
		node = entity.find_component("cNode"),

		target = target,
		speed = speed,

		callback = callback
	}

	projectile.pos = projectile.node.get_global_pos()

	projectile.die = function(lost_target)
		if not lost_target and projectile.callback then
			projectile.callback()
		end

		projectiles[projectile.name] = nil

		projectile.entity.get_parent().remove_child(projectile.entity)
	end

	projectile.tick = function()
		if not projectile.target or projectile.target.dead then
			projectile.die(true)
			return
		end
		
		projectile.pos = projectile.node.get_global_pos()

		local tpos = projectile.target.pos + vec3(0, projectile.target.height * 0.8, 0)
		local l, d = length_and_dir_3(tpos - projectile.pos)
		if d then
			projectile.node.look_at(tpos)
		end

		if l <= projectile.speed then
			projectile.die(false)
		else
			projectile.node.add_pos(d * projectile.speed)
		end
	end

	projectiles[projectile.name] = projectile

	return projectile
end
