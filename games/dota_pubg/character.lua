characters = {}
characters[1] = {}
characters[2] = {}

function make_character(entity, group, stats)
	local character = {
		name = entity.get_name(),
		tag = group == 1 and TAG_CHARACTER_G1 or TAG_CHARACTER_G2,
		dead = false,
		group = group,
		sleeping = true,

		entity = entity,
		node = entity.find_component("cNode"),
		animation = entity.find_component("cAnimation"),
		controller = entity.find_component("cController"),
		pos = vec3(0),
		yaw = 0,
		dir = vec3(0, 0, 1),

		radius = 0.28,
		speed = 0.06,
		attack_interval = 100,

		state = "null",
		target_pos = vec3(0),
		stuck_tick = 0,
		target = nil,
		attack_tick = 0,
		attacking = false,
		last_hit_character = nil,

		on_tick = nil,
		on_die = nil,
		on_change_extra_state = nil,
		on_process_extra_state = nil,
		on_reward = nil,

		HP_MAX =		stats and stats.HP_MAX or 0,
		HP =			stats and stats.HP_MAX or 0,
		MP_MAX =		stats and stats.MP_MAX or 0,
		MP =			stats and stats.MP_MAX or 0,
		HP_RECOVER =	stats and stats.HP_RECOVER or 0,
		MP_RECOVER =	stats and stats.MP_RECOVER or 0,
		ATK_DMG =		stats and stats.ATK_DMG or 0,
		ATK_TYPE =		stats and stats.ATK_TYPE or 0,
		recover_tick = 0
	}
	
	entity.set_tag(character.tag)
	entity.set_visible(false)
	character.pos = character.node.get_global_pos()

	character.ui = create_entity("character_hud")
	character.ui.set_visible(false)
	character.ui.element = character.ui.find_component("cElement")
	character.ui.hp_bar = character.ui.find_child("hp_bar").find_component("cElement")
	__ui_scene.add_child(character.ui)

	character.die = function()
		if character.on_die then
			character.on_die()
		end

		characters[character.group][character.name] = nil

		character.entity.get_parent().remove_child(character.entity)
		character.ui.get_parent().remove_child(character.ui)
		character.dead = true
	end

	character.sleep = function()
		if not character.sleeping then
			character.sleeping = true
			character.entity.set_visible(false)
		end
	end

	character.awake = function()
		if character.sleeping then
			character.sleeping = false
			character.entity.set_visible(true)
		end
	end

	character.update_dir = function()
		character.node.set_euler(vec3(character.yaw, 0, 0))
		character.dir = character.node.get_local_dir(2)
	end

	character.get_enemy_group = function()
		if character.group == 1 then return 2 end
		if character.group == 2 then return 1 end
		return nil
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
		elseif character.on_change_extra_state then
			character.on_change_extra_state(s, t)
		end
		character.state = s
	end

	character.animation.add_callback(function(frame)
		if character.attacking then
			if frame == 12 then
				if character.target and not character.target.dead then
					local l, d = length_and_dir_2(character.target.pos.to_flat() - character.pos.to_flat())
					if l <= character.radius + character.target.radius + 3 then
						character.target.receive_damage(character, math.floor(0.5 + character.ATK_DMG * (math.random() * 0.2 + 0.9)))
					end
				end
			end
			if frame == -1 then
				character.attacking = false
			end
		end
	end, character.animation)

	character.receive_damage = function(src, value)
		character.last_hit_character = src

		local c = camera.camera
		local p1 = c.world_to_screen(character.pos + vec3(0, 1.8, 0))
		local p2 = c.world_to_screen(src.pos + vec3(0, 1.8, 0))
		local l, d = length_and_dir_2(p1 - p2)
		if d then
			l = d * 2
		else
			l = vec2(0, -2)
		end
		new_floating_item(p1, l, tostring(math.floor(value / 10.0)))

		if character.HP > value then
			character.HP = character.HP - value
		else
			character.die()
		end
	end

	character.receive_heal = function(src, value)
		character.HP = character.HP + value
		if character.HP > character.HP_MAX then
			character.HP = character.HP_MAX
		end
	end

	character.find_closest_obj = function(tag, r)
		local arr = flame_malloc(8)
		local n = obj_root_n.get_within_circle(character.pos.to_flat(), 5, arr, 1, tag)
		local p = flame_get(arr, 0, e_type_pointer, e_else_type, 1, 1)
		flame_free(arr)

		if n == 0 or not p then return nil end

		local name = make_entity(p).get_name()
		if tag == TAG_CHARACTER_G1 then return characters[1][name] end
		if tag == TAG_CHARACTER_G2 then return characters[2][name] end
		if tag == TAG_ITEM_OBJ then return item_objs[name] end
	end

	character.move_to_pos = function(tp, r)
		local v = tp - character.pos.to_flat()
		local l, d = length_and_dir_2(v)
		if d then -- aim
			character.yaw = math.atan(d.x, d.y) / 3.14 * 180
			character.update_dir()
		end

		if l <= character.speed + r then
			return false
		end

		character.animation.play(1)
		character.animation.set_loop(true)
		character.controller.move(vec3(d.x * character.speed, 0, d.y * character.speed))
		return true
	end

	character.attack_target = function()
		local l, d = length_and_dir_2(character.target.pos.to_flat() - character.pos.to_flat())
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
	end

	character.tick = function()
		local pos = character.node.get_global_pos()
		
		character.ui.set_visible(false)
		local ui_pos = camera.camera.world_to_screen(vec3(pos.x, pos.y + 1.8, pos.z))
		if ui_pos.x > -100 then
			character.ui.set_visible(true)
			character.ui.element.set_pos(ui_pos + vec2(-30, -20))
			character.ui.hp_bar.set_scalex(character.HP / character.HP_MAX)
		end

		if character.attack_tick > 0 then
			character.attack_tick = character.attack_tick - 1
		end

		if character.recover_tick > 0 then
			character.recover_tick = character.recover_tick - 1
		else
			character.receive_heal(character, character.HP_RECOVER)
			character.recover_tick = 60
		end

		if character.state == "move_to" then
			if not character.move_to_pos(character.target_pos.to_flat(), 0) then
				character.change_state("idle")
			end

			if distance_3(pos, character.pos) < 0.001 then
				character.stuck_tick = character.stuck_tick + 1
				if character.stuck_tick >= 5 then
					character.change_state("idle")
				end
			end
		elseif character.state == "attack_target" then
			if character.target and not character.target.dead then
				character.attack_target()
			else
				character.target = nil
				character.change_state("idle")
			end
		elseif character.on_process_extra_state then
			character.on_process_extra_state()
		end

		character.pos = pos

		if character.on_tick then
			character.on_tick()
		end
	end

	character.change_state("idle")

	characters[group][character.name] = character

	return character

end
