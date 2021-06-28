characters = {}
characters[1] = {}
characters[2] = {}

function make_character(entity, group, stats)
	local character = {
		name = entity.get_name(),
		group = group,
		sleeping = true,
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
		on_event = nil,

		recover_tick = 0
	}

	entity.set_tag(group)
	entity.set_visible(false)
	character.pos = character.node.get_global_pos()
	
	for key, val in pairs(stats) do
		character[key] = val
	end
	character.HP = character.HP_MAX

	character.ui = create_entity("character_hud")
	character.ui.set_visible(false)
	character.ui.element = character.ui.find_component("cElement")
	character.ui.floating_tips = character.ui.find_child("floating_tips")
	character.ui.floating_tips.items = {}
	character.ui.hp_text = character.ui.find_child("hp_text").find_component("cText")
	character.ui.hp_bar = character.ui.find_child("hp_bar").find_component("cElement")
	__ui.add_child(character.ui)

	character.die = function()
		if character.on_die then
			character.on_die()
		end

		characters[character.group][character.name] = nil

		character.entity.remove_event(character.event)
		character.entity.get_parent().remove_child(character.entity)
		character.ui.get_parent().remove_child(character.ui)
		character.dead = true
	end

	character.sleep = function()
		if not character.sleeping then
			character.sleeping = true
			character.entity.set_visible(false)
			character.entity.remove_event(character.event)
		end
	end

	character.awake = function()
		if character.sleeping then
			character.sleeping = false
			character.entity.set_visible(true)
			character.event = character.entity.add_event(function()
				character.do_logic()
			end, 0)
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
		elseif s == "attack_on_pos" then
			character.target_pos = t
			character.attacking = false
		end
		character.state = s
	end

	character.animation.add_callback(function(frame)
		if character.attacking then
			if frame == 12 then
				if character.target and not character.target.dead then
					local l, d = length_and_dir_2(character.target.pos.to_flat() - character.pos.to_flat())
					if l <= character.radius + character.target.radius + 3 then
						character.target.on_receive_damage(character, math.floor(0.5 + character.ATTACK_DAMAGE * (math.random() * 0.2 + 0.9)))
					end
				end
			end
			if frame == -1 then
				character.attacking = false
			end
		end
	end, character.animation)

	character.on_receive_damage = function(src, value)
		local tip_item = {}
		tip_item.e = create_entity("floating_tip")
		local text = tip_item.e.find_component("cText")
		text.set_text(tostring(value))
		if character == main_player.character then
			text.set_font_color(vec4(255, 0, 0, 255))
		end
		character.ui.floating_tips.add_child(tip_item.e)
		tip_item.element = tip_item.e.find_component("cElement")
		tip_item.tick = 35
		table.insert(character.ui.floating_tips.items, tip_item)

		if character.HP > value then
			character.HP = character.HP - value
		else
			character.die()
		end
	end

	character.on_receive_heal = function(src, value)
		character.HP = character.HP + value
		if character.HP > character.HP_MAX then
			character.HP = character.HP_MAX
		end
	end

	character.find_closest_enemy = function(r)
		local g = character.get_enemy_group()
		if not g then return nil end
		
		local parr = flame_malloc(8)
		local n = obj_root.get_within_circle(character.pos.to_flat(), 5, parr, 1, g)
		local p = flame_get(parr, 0, e_type_pointer, e_else_type, 1, 1)
		flame_free(parr)

		if n == 0 or not p then return nil end

		local e = make_entity(p)
		local name = e.get_name()
		return characters[g][name]
	end

	character.do_logic = function()
		local pos = character.node.get_global_pos()
		local dis_to_mp = distance_3(pos, main_player.pos)
		
		character.ui.set_visible(false)
		if dis_to_mp < 50 then
			local ui_pos = camera.camera.world_to_screen(vec3(pos.x, pos.y + 1.8, pos.z))
			if ui_pos.x > -100 then
				character.ui.set_visible(true)
				character.ui.element.set_pos(ui_pos + vec2(-30, -20))
				character.ui.hp_text.set_text(character.HP.."/"..character.HP_MAX)
				character.ui.hp_bar.set_scalex(character.HP / character.HP_MAX)

				local i = 1
				while i <= #character.ui.floating_tips.items do
					local item = character.ui.floating_tips.items[i]
					item.element.add_pos(vec2(0, -2))
					item.tick = item.tick - 1
					if item.tick <= 0 then
						character.ui.floating_tips.remove_child(item.e)
						table.remove(character.ui.floating_tips.items, i)
					else
						i = i + 1
					end
				end
			end
		end

		if character.attack_tick > 0 then
			character.attack_tick = character.attack_tick - 1
		end

		if character.recover_tick > 0 then
			character.recover_tick = character.recover_tick - 1
		else
			character.on_receive_heal(character, character.HP_RECOVER)
			character.recover_tick = 60
		end

		function move_to_pos()
			local v = character.target_pos - vec2(pos.x, pos.z)
			local l, d = length_and_dir_2(v)
			if d then -- aim
				character.yaw = math.atan(d.x, d.y) / 3.14 * 180
				character.update_dir()
			end

			if l <= character.speed then
				return false
			end

			character.animation.play(1)
			character.animation.set_loop(true)
			character.controller.move(vec3(d.x * character.speed, 0, d.y * character.speed))
			return true
		end

		function attack_target()
			local l, d = length_and_dir_2(character.target.pos.to_flat() - pos.to_flat())
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

		if character.state == "move_to" then
			if not move_to_pos() then
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
				attack_target()
			else
				character.target = nil
				character.change_state("idle")
			end
		elseif character.state == "attack_on_pos" then
			if not character.attacking then
				character.target = nil

				local char = character.find_closest_enemy(5)
				if char then 
					character.target = char
				end
			end
			
			if character.target and not character.target.dead then
				attack_target()
			else
				character.target = nil
				move_to_pos()
			end
		end

		character.pos = pos

		if character.on_event then
			character.on_event()
		end
	end

	character.change_state("idle")

	characters[group][character.name] = character

	return character

end
