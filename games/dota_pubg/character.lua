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
		armature = entity.find_component("cArmature"),
		controller = entity.find_component("cCharacterController"),
		pos = vec3(0),
		prev_pos = vec3(0),

		curr_anim = -1,
		curr_frame = -1,

		radius = 0.28,
		height = 1.8,
		speed = 0.06,
		attack_interval = 100,

		state = "null",
		state_date = nil,
		target_pos = vec3(0),
		target = nil,
		stuck_tick = 0,
		attack_semp = true,
		attack_tick = 0,
		last_receive_damage_src = nil,

		on_reward = nil,

		HP_MAX =		stats and stats.HP_MAX or 0,
		HP =			stats and stats.HP_MAX or 0,
		MP_MAX =		stats and stats.MP_MAX or 0,
		MP =			stats and stats.MP_MAX or 0,
		HP_RECOVER =	stats and stats.HP_RECOVER or 0,
		MP_RECOVER =	stats and stats.MP_RECOVER or 0,
		MOV_SP =		stats and stats.MOV_SP or 0,
		ATK =			stats and stats.ATK or 0,

		recover_tick = 0
	}
	
	entity.set_tag(character.tag)
	character.pos = character.node.get_global_pos()

	character.skills = {}
	for i=1, SKILL_SLOTS_COUNT, 1 do
		character.skills[i] = nil
	end

	character.equipments = {}
	for i=1, EQUIPMENT_SLOTS_COUNT, 1 do
		character.equipments[i] = 0
	end

	character.items = {}
	for i=1, ITEM_SLOTS_COUNT, 1 do
		character.items[i] = nil
	end

	character.ui = create_entity("character_hud")
	character.ui.set_visible(false)
	character.ui.element = character.ui.find_component("cElement")
	character.ui.hp_bar = character.ui.find_child("hp_bar").find_component("cElement")
	__ui_scene.add_child(character.ui)

	character.die = function()
		characters[character.group][character.name] = nil

		character.entity.get_parent().remove_child(character.entity)
		character.ui.get_parent().remove_child(character.ui)
		character.dead = true
	end

	character.sleep = function()
		character.sleeping = true
	end

	character.awake = function()
		character.sleeping = false
	end

	character.get_enemy_group = function()
		if character.group == 1 then return 2 end
		if character.group == 2 then return 1 end
		return nil
	end

	character.change_state = function(s, t, d)
		if s == "idle" then
			character.armature.play(0, 1.0, true)
			character.state = s
		elseif s == "move_to" then
			character.target_pos = t
			character.stuck_tick = 0
			character.state = s
		elseif s == "attack_target" then
			character.target = t
			character.state = s
		elseif s == "use_skill_to_target" then
			character.target = t
			character.state_date = d
			character.state = s
		elseif s == "pick_up" then
			character.target = t
			character.state = s
		end
	end

	character.receive_damage = function(src, value)
		character.last_receive_damage_src = src

		local c = camera.camera
		local p1 = c.world_to_screen(character.pos + vec3(0, 1.8, 0))
		local p2 = c.world_to_screen(src.pos + vec3(0, 1.8, 0))
		local l, d = length_and_dir_2(p1 - p2)
		if d then
			l = d * 2
		else
			l = vec2(0, -2)
		end
		new_floating_tip(p1, l, tostring(math.floor(value / 10.0)))

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

	character.receive_mana = function(src, value)
		character.MP = character.MP + value
		if character.MP > character.MP_MAX then
			character.MP = character.MP_MAX
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
			character.node.set_euler(vec3(math.atan(d.x, d.y) / 3.14 * 180, 0, 0))
		end

		if l <= character.speed + r then
			return true
		end

		local ratio = character.MOV_SP / 100.0
		character.armature.play(1, ratio, true)
		character.controller.move(vec3(d.x, 0, d.y) * character.speed * ratio)
		return false
	end

	character.attack_target = function()
		local l, d = length_and_dir_2(character.target.pos.to_flat() - character.pos.to_flat())
		if d then -- aim
			character.node.set_euler(vec3(math.atan(d.x, d.y) / 3.14 * 180, 0, 0))
		end
		if character.curr_anim ~= 2 then
			if l <= character.radius + character.target.radius + 1 then
				if character.attack_tick == 0 then
					character.attack_tick = character.attack_interval
					character.armature.play(2, 1.0, false)
					character.attack_semp = true
				else
					character.armature.stop_at(2, -1)
				end
			else
				character.armature.play(1, 1.0, true)
				character.controller.move(vec3(d.x * character.speed, 0, d.y * character.speed))
			end
		elseif character.attack_semp and (character.curr_frame == -1 or character.curr_frame >= 12) then
			if l <= character.radius + character.target.radius + 3 then
				character.target.receive_damage(character, math.floor(0.5 + character.ATK * (math.random() * 0.2 + 0.9)))
			end
			character.attack_semp = false
		end
	end

	character.process_state = function()
		if character.state == "move_to" then
			if character.move_to_pos(character.target_pos.to_flat(), 0) then
				character.change_state("idle")
			end

			if distance_3(character.prev_pos, character.pos) < 0.001 then
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
		elseif character.state == "pick_up" then
			if not character.target or character.target.dead then
				character.change_state("idle")
			else
				if character.pick_up_target() then
					character.change_state("idle")
				end
			end
		elseif character.state == "use_skill_to_target" then
			if character.target and not character.target.dead then
				local l, d = length_and_dir_2(character.target.pos.to_flat() - character.pos.to_flat())
				if d then -- aim
					character.node.set_euler(vec3(math.atan(d.x, d.y) / 3.14 * 180, 0, 0))
				end
				if l <= character.state_date.dist then
					character.use_skill(character.state_date.idx, character.target)
					character.change_state("idle")
				else
					character.armature.play(1, 1.0, true)
					character.controller.move(vec3(d.x * character.speed, 0, d.y * character.speed))
				end
			else
				character.target = nil
				character.change_state("idle")
			end
		end
	end

	character.tick = function()
		character.prev_pos = character.pos
		character.pos = character.node.get_global_pos()

		character.curr_anim = character.armature.get_curr_anim()
		character.curr_frame = character.armature.get_curr_frame()
		
		local show_ui = false
		local ui_pos = camera.camera.world_to_screen(character.pos + vec3(0, 1.8, 0))
		if ui_pos.x > -100 then
			show_ui = true
			character.ui.element.set_pos(ui_pos + vec2(-30, -20))
			character.ui.hp_bar.set_scalex(character.HP / character.HP_MAX)
		end
		character.ui.set_visible(show_ui)

		if character.attack_tick > 0 then
			character.attack_tick = character.attack_tick - 1
		end

		if character.recover_tick > 0 then
			character.recover_tick = character.recover_tick - 1
		else
			character.receive_heal(character, character.HP_RECOVER)
			character.receive_mana(character, character.MP_RECOVER)
			character.recover_tick = 60
		end

		for i=1, SKILL_SLOTS_COUNT, 1 do
			local slot = character.skills[i]
			if slot and slot.cd > 0 then
				slot.cd = slot.cd - 1
			end
		end

		character.process_state()
	end

	character.calc_stats = function()
	end

	character.learn_skill = function(id)
		for i=1, SKILL_SLOTS_COUNT, 1 do
			if not character.skills[i] then
				character.skills[i] = { id=id, cd=0 }
				if character == main_player then
					update_ui_skill_slots()
				end
				return true
			end
		end
		return false
	end

	character.use_skill = function(idx, target)
		local slot = character.skills[idx]
		if slot then
			local skill_type = SKILL_LIST[slot.id]
			if skill_type.type == "ACTIVE" then
				if slot.cd == 0 and skill_type.cost_mana <= character.MP and 
				distance_2(target.pos.to_flat(), character.pos.to_flat()) <= skill_type.distance + 1 then
					slot.cd = skill_type.cool_down
					character.MP = character.MP - skill_type.cost_mana
					skill_type.logic(character, target)
				end
			end
		end
	end

	character.receive_item = function(id, num)
		local item_type = ITEM_LIST[id]
		local max_num = item_type.stack_num
		local ori_num = num
		while num > 0 do
			for i=1, ITEM_SLOTS_COUNT, 1 do
				local slot = character.items[i]
				if slot and slot.id == id then
					local n = max_num - slot.num
					if n >= num then
						slot.num = slot.num + num
						num = 0
						break
					else
						num = num - n
						slot.num = slot.num + n
					end
				end
			end
			if num == 0 then break end
			for i=1, ITEM_SLOTS_COUNT, 1 do
				local slot = character.items[i]
				if not slot then
					slot = { id=id, num=0 }
					character.items[i] = slot
					if max_num >= num then
						slot.num = max_num
						num = 0
						break
					else
						num = num - max_num
						slot.num = max_num
					end
					break
				end
			end
		end
		
		if character == main_player and num ~= ori_num then
			update_ui_item_slots()
		end
		return num
	end

	character.use_item = function(idx, target)
		local slot = character.items[idx]
		if slot then
			local item_type = ITEM_LIST[slot.id]
			if item_type.type == "EQUIPMENT" then
				local euip_slot = item_type.slot

				local ori_id = character.equipments[euip_slot]
				character.equipments[euip_slot] = slot.id

				character.items[idx] = nil

				if ori_id ~= 0 then
					character.receive_item(ori_id, 1)
				end
				character.calc_stats()

				if character == main_player then
					update_ui_item_slots()
					update_ui_equipment_slots()
				end
			end
		end
	end

	character.take_off_equipment = function(idx)
		local id = character.equipments[idx]
		if id ~= 0 then
			if character.receive_item(id, 1) == 0 then
				character.equipments[idx] = 0
				character.calc_stats()

				if character == main_player then
					update_ui_equipment_slots()
				end
			end
		end
	end

	character.change_state("idle")

	characters[group][character.name] = character

	return character

end
