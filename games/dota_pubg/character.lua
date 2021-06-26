function make_character(entity)
	local character = {
		dead = false,

		entity = entity,
		node = entity.find_component("cNode"),
		animation = entity.find_component("cAnimation"),
		controller = entity.find_component("cController"),
		pos = vec3(0),
		yaw = 0,
		dir = vec3(0, 0, 1),

		radius = 0.28,
		speed = 0.057,
		attack_interval = 100,

		state = "null",
		target_pos = vec2(0),
		stuck_tick = 0,
		target = nil,
		attack_tick = 0,
		attacking = false,

		on_die = nil,
	
		HP = 100,
		HP_MAX = 100,
		ATTACK_DAMAGE = 10
	}

	character.ui_bar = create_entity("bar")
	character.ui_bar.element = character.ui_bar.find_component("cElement")
	character.ui_bar.text = character.ui_bar.get_child(0).find_component("cText")
	character.ui_bar.bar = character.ui_bar.get_child(1).find_component("cElement")
	__ui.add_child(character.ui_bar)

	character.die = function()
		if character.on_die then
			character.on_die()
		end

		character.entity.remove_event(character.event)
		character.entity.get_parent().remove_child(character.entity)
		character.ui_bar.get_parent().remove_child(character.ui_bar)
		character.dead = true
	end

	character.update_dir = function()
		character.node.set_euler(vec3(character.yaw, 0, 0))
		character.dir = character.node.get_local_dir(2)
	end

	character.change_state = function(s, t)
		if s == "idle" then
			character.animation.play(0)
			character.animation.set_loop(true)
			character.attacking = false
		elseif s == "move_to" then
			character.target_pos = t
			character.stuck_tick = 0
			character.attacking = false
		elseif s == "attack_target" then
			if character.attacking and character.target ~= t then
				character.attacking = false
			end
			character.target = t
		end
		character.state = s
	end

	character.animation.add_callback(function(frame)
		if character.attacking then
			if frame == 12 then
				if character.target then
					local p = character.pos
					local tp = character.target.pos
					local l, d = length_and_dir_2(vec2(tp.x, tp.z) - vec2(p.x, p.z))
					if l <= character.radius + character.target.radius + 3 then
						character.target.on_receive_damage(character, character.ATTACK_DAMAGE)
					end
				end
			end
			if frame == -1 then
				character.attacking = false
			end
		end
	end, character.animation)

	character.on_receive_damage = function(src, value)
		if character.HP > value then
			character.HP = character.HP - value
		else
			character.die()
		end
	end

	character.change_state("idle")

	character.event = character.entity.add_event(function()
		local pos = character.node.get_global_pos()

		character.ui_bar.element.set_pos(camera.camera.world_to_screen(vec3(pos.x, pos.y + 1.8, pos.z)) + vec2(-30, -20))
		character.ui_bar.text.set_text(character.HP.."/"..character.HP_MAX)
		character.ui_bar.bar.set_scalex(character.HP / character.HP_MAX)

		if character.attack_tick > 0 then
			character.attack_tick = character.attack_tick - 1
		end

		if character.state == "move_to" then
			local v = character.target_pos - vec2(pos.x, pos.z)
			local l, d = length_and_dir_2(v)
			if d then -- aim
				character.yaw = math.atan(d.x, d.y) / 3.14 * 180
				character.update_dir()
			end
			if l <= character.speed then
				character.change_state("idle")
			else
				character.animation.play(1)
				character.animation.set_loop(true)
				character.controller.move(vec3(d.x * character.speed, 0, d.y * character.speed))
			end

			if distance_3(pos, character.pos) < 0.001 then
				character.stuck_tick = character.stuck_tick + 1
				if character.stuck_tick >= 5 then
					character.change_state("idle")
				end
			end
		elseif character.state == "attack_target" then
			if character.target and character.target.dead ~= true then
				local tp = character.target.node.get_global_pos()
				local v = vec2(tp.x, tp.z) - vec2(pos.x, pos.z)
				local l, d = length_and_dir_2(v)
				if d then -- aim
					character.yaw = math.atan(d.x, d.y) / 3.14 * 180
					character.update_dir()
				end
				if not character.attacking then
					if l <= character.radius + character.target.radius + 1 then
						if character.attack_tick == 0 then
							character.attack_tick = character.attack_interval
							character.attacking = true
							character.animation.play(2)
							character.animation.set_loop(false)
						else
							character.animation.stop_at(2, -1)
						end
					else
						character.animation.play(1)
						character.animation.set_loop(true)
						character.controller.move(vec3(d.x * character.speed, 0, d.y * character.speed))
					end
				end
			else
				character.target = nil
				character.change_state("idle")
			end
		end

		character.pos = pos
	end, 0)

	return character

end
