particles = {}

function make_particle(entity, ttl)
	local particle = {
		name = entity.get_name(),

		entity = entity,
		node = entity.find_component("cNode"),

		ttl = ttl
	}

	particle.die = function()
		particles[particle.name] = nil

		particle.entity.get_parent().remove_child(particle.entity)
	end

	particle.tick = function()
		if particle.ttl > 0 then
			particle.ttl = particle.ttl - 1
		else
			particle.die()
		end
	end
	
	particles[particle.name] = particle

	return particle
end
