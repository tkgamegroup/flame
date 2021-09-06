characters = {}
characters[TAG_CHARACTER_G1] = {}
characters[TAG_CHARACTER_G2] = {}
characters[TAG_CHARACTER_G3] = {}

function make_character(entity, tag, stats)
	local character = {
		name = entity.get_name(),
		tag = tag,
		dead = false,
		sleeping = true,

		entity = entity,
		node = entity.find_component("cNode"),
		armature = entity.find_component("cArmature"),
		controller = entity.find_component("cCharacterController"),

		curr_anim = -1,
		curr_frame = -1,
		anim_swap_tick = 0,

		state = "null",
		state_date = nil,
		target_pos = vec3(0),
		target = nil,
		stuck_tick = 0,
		attack_semp = true,
		attack_tick = 0,
		last_damage_src = nil,
		last_damage_src_frame = 0,

		on_tick = nil,
		on_reward = nil,

		recover_tick = 0
	}
	
	entity.set_tag(character.tag)
	if tag == TAG_CHARACTER_G1 then
		character.enemy_tag = TAG_CHARACTER_G2
	elseif tag == TAG_CHARACTER_G2 then
		character.enemy_tag = TAG_CHARACTER_G1
	else
		character.enemy_tag = 0
	end

	character.pos = character.node.get_global_pos()
	character.prev_pos = character.pos
	character.scale = character.node.get_global_scale().x
	character.radius = character.controller.get_radius() * character.scale
	character.height = character.controller.get_height() * character.scale
	
	character.HP_MAX = new_attribute(1000) -- health point max
	character.MP_MAX = new_attribute(1000) -- magic point max
	character.HP_REC = new_attribute() -- health point recover
	character.MP_REC = new_attribute() -- magic point recover
	character.MOV_SP = new_attribute() -- movement speed
	
	character.ATK_TYPE = "Physical"
	character.ATK_DMG = new_attribute(100) -- attack damage
	character.ATK_SP = new_attribute() -- attack speed, in frames
	character.PHY_INC = new_attribute() -- inflicted physical damage increase, in percentage
	character.PHY_INC.p = nil
	character.MAG_INC = new_attribute() -- inflicted magical damage increase, in percentage
	character.MAG_INC.p = nil
	character.DMG_INC =	new_attribute() -- inflicted damage increase, in percentage
	character.DMG_INC.p = nil

	character.ARMOR   = new_attribute() -- physical resistance
	character.MAG_RES =	new_attribute() -- magical resistance
	character.PHY_RED = new_attribute() -- received physical damage reduction, in percentage
	character.PHY_RED.p = nil
	character.MAG_RED = new_attribute() -- received magical damage reduction, in percentage
	character.MAG_RED.p = nil
	character.DMG_RED =	new_attribute() -- received damage reduction, in percentage
	character.DMG_RED.p = nil
	
	if stats then
		if stats.HP_MAX then character.HP_MAX.b = stats.HP_MAX end
		if stats.MP_MAX then character.MP_MAX.b = stats.MP_MAX end
		if stats.HP_REC then character.HP_REC.b = stats.HP_REC end
		if stats.MP_REC then character.MP_REC.b = stats.MP_REC end
		if stats.MOV_SP then character.MOV_SP.d = stats.MOV_SP end

		if stats.ATK_TYPE then character.ATK_TYPE = stats.ATK_TYPE end
		if stats.ATK_DMG then character.ATK_DMG.b = stats.ATK_DMG end
		if stats.ATK_SP then character.ATK_SP.d = stats.ATK_SP end
		if stats.PHY_INC then character.PHY_INC.b = stats.PHY_INC end
		if stats.MAG_INC then character.MAG_INC.b = stats.MAG_INC end
		if stats.DMG_INC then character.DMG_INC.b = stats.DMG_INC end

		if stats.ARMOR then character.ARMOR.b = stats.ARMOR end
		if stats.MAG_RES then character.MAG_RES.b = stats.MAG_RES end
		if stats.PHY_RED then character.PHY_RED.b = stats.PHY_RED end
		if stats.MAG_RED then character.MAG_RED.b = stats.MAG_RED end
		if stats.DMG_RED then character.DMG_RED.b = stats.DMG_RED end
	end

	character.HP = character.HP_MAX.t
	character.MP = character.MP_MAX.t

	character.skills = {}
	character.equipments = {}
	character.items = {}
	character.perks = {}
	character.buffs = {}

	character.ui = create_entity("prefabs/ui/character_hud")
	character.ui.set_visible(false)
	character.ui.element = character.ui.find_component("cElement")
	character.ui.hp_bar = character.ui.find_child("hp_bar").find_component("cElement")
	__ui_scene.add_child(character.ui)

	character.die = function()
		characters[character.tag][character.name] = nil

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

	character.change_state = function(s, t, d)
		if s == "idle" then
			character.armature.play(0, 1.0, true)
		elseif s == "move_to" then
			character.target_pos = t
			character.stuck_tick = 0
		elseif s == "attack_target" then
			character.target = t
		elseif s == "attack_on_pos" then
			character.target_pos = t
		elseif s == "cast_to_target" then
			character.target = t
			character.state_date = d
		elseif s == "interact" then
			character.target = t
		end
		character.state = s
	end

	character.inflict_damage = function(tar, type, value)
		if type == "Physical" then
			value = value * (1.0 + character.PHY_INC.t / 100.0)
		else
			value = value * (1.0 + character.MAG_INC.t / 100.0)
		end
		value = value * (1.0 + character.DMG_INC.t / 100.0)
		tar.receive_damage(character, type, value)
	end

	character.receive_damage = function(src, type, value)
		character.last_damage_src = src
		character.last_damage_src_frame = frame

		local cam = camera.camera
		local p1 = cam.world_to_screen(character.pos + vec3(0, 1.8, 0), vec4(-50, -16, -50, -16))
		if p1.x < 10000 then
			local p2 = cam.world_to_screen(src.pos + vec3(0, 1.8, 0), vec4(0))
			local l, d = length_and_dir_2(p1 - p2)
			if d then
				d = d * 2
			else
				d = vec2(0, -2)
			end
			new_floating_tip(p1, d, tostring(math.floor(value / 10.0)))
		end

		if character.HP > value then
			character.HP = character.HP - value
		else
			character.die()
		end
	end

	character.receive_heal = function(src, value)
		character.HP = math.min(character.HP + value, character.HP_MAX.t)
	end

	character.receive_mana = function(src, value)
		character.MP = math.min(character.MP + value, character.MP_MAX.t)
	end

	character.find_closest_obj = function(tag, r)
		local arr = flame_malloc(8)
		local n = obj_root_n.get_within_circle(character.pos.to_flat(), 5, arr, 1, tag)
		local p = flame_get_p(arr, 0)
		flame_free(arr)

		if n == 0 then return nil end

		local name = make_entity(p).get_name()
		if tag == TAG_CHARACTER_G1 then return characters[TAG_CHARACTER_G1][name] end
		if tag == TAG_CHARACTER_G2 then return characters[TAG_CHARACTER_G2][name] end
		if tag == TAG_ITEM_OBJ then return item_objs[name] end
		return nil
	end

	character.aim = function(p)
		local l, d = length_and_dir_2(p - character.pos.to_flat())
		if d then
			character.node.set_euler(vec3(math.atan(d.x, d.y) / 3.14 * 180, 0, 0))
		end
		return l, d
	end

	character.move = function(d)
		character.armature.play(1, character.MOV_SP.p, true)
		character.controller.move(vec3(d.x, 0, d.y) * character.MOV_SP.t)
	end

	character.move_to_pos = function(tp, r)
		local l, d = character.aim(tp)

		if l <= character.MOV_SP.t + r then
			return true
		end

		character.move(d)
		return false
	end

	character.attack_target = function()
		local l, d = character.aim(character.target.pos.to_flat())

		if character.curr_anim ~= 2 then
			if l <= character.radius + character.target.radius + 1 then
				if character.attack_tick == 0 then
					character.attack_tick = character.ATK_SP.t
					character.armature.play(2, character.ATK_SP.p, false)
					character.attack_semp = true
				else
					if character.anim_swap_tick == 0 then
						character.armature.stop_at(2, -1)
					end
				end
			else
				character.move(d)
				character.anim_swap_tick = 5
			end
		elseif character.attack_semp and (character.curr_frame == -1 or character.curr_frame >= 12) then
			if l <= character.radius + character.target.radius + 3 then
				character.inflict_damage(character.target, character.ATK_TYPE, math.floor(0.5 + character.ATK_DMG.t * (math.random() * 0.2 + 0.9)))
			end
			character.attack_semp = false
		end
	end

	character.interact = function()
		local target = character.target
		local tag = target.tag
		if character.move_to_pos(target.pos.to_flat(), 1.0) then
			if tag == TAG_ITEM_OBJ then
				local n = character.receive_item(target.id, target.num)
				if n == 0 then
					target.die()
				else
					target.num = target.num - n
				end
			elseif tag == TAG_CHARACTER_G3 then
				target.on_interact()
			end
			return true
		end
		return false
	end

	character.tick = function()
		character.prev_pos = character.pos
		character.pos = character.node.get_global_pos()
		
		if character.pos.y < -100 then
			character.die()
			return
		end

		if character.sleeping then return end

		character.curr_anim = character.armature.get_curr_anim()
		character.curr_frame = character.armature.get_curr_frame()
		if character.anim_swap_tick > 0 then
			character.anim_swap_tick = character.anim_swap_tick - 1
		end
		
		local show_ui = false
		local ui_pos = camera.camera.world_to_screen(character.pos + vec3(0, 1.8, 0), vec4(-60, -6, -60, -6))
		if ui_pos.x < 10000 then
			show_ui = true
			character.ui.element.set_pos(ui_pos + vec2(-30, -20))
			character.ui.hp_bar.set_scalex(character.HP / character.HP_MAX.t)
		end
		character.ui.set_visible(show_ui)

		if character.attack_tick > 0 then
			character.attack_tick = character.attack_tick - 1
		end

		if character.recover_tick > 0 then
			character.recover_tick = character.recover_tick - 1
		else
			character.receive_heal(character, character.HP_REC.t)
			character.receive_mana(character, character.MP_REC.t)
			character.recover_tick = 60
		end

		local i=1
		local b = character.buffs[1]
		local buff_changed = false
		while b do
			if b.t > 0 then
				b.t = b.t - 1
				i = i + 1
				b = character.buffs[i]
			else
				table.remove(character.buffs, i)
				b = character.buffs[i]
				buff_changed = true
			end
		end
		if buff_changed then
			character.calc_stats()
		end

		for i=1, SKILL_SLOTS_COUNT, 1 do
			local slot = character.skills[i]
			if slot and slot.cd > 0 then
				slot.cd = slot.cd - 1
			end
		end
		
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
		elseif character.state == "attack_on_pos" then
			if character.curr_anim ~= 2 then
				character.target = character.find_closest_obj(character.enemy_tag, 5)
			end
			
			if character.target and not character.target.dead then
				character.attack_target()
			else
				character.target = nil
				if character.move_to_pos(character.target_pos.to_flat(), 0) then 
					character.change_state("idle")
				end
			end
		elseif character.state == "interact" then
			if character.target and not character.target.dead then
				if character.interact() then
					character.change_state("idle")
				end
			else
				character.target = nil
				character.change_state("idle")
			end
		elseif character.state == "cast_to_target" then
			if character.target and not character.target.dead then
				local l, d = character.aim(character.target.pos.to_flat())
				if l <= character.state_date.dist then
					character.use_skill(character.state_date.idx, character.target)
					character.change_state("idle")
				else
					character.move(d)
				end
			else
				character.target = nil
				character.change_state("idle")
			end
		end

		if character.on_tick then
			character.on_tick()
		end
	end

	character.collect_attribute = function(n)
		local a = character[n]
		if a.a then a.a = 0 end
		if a.p then a.p = 0 end
		for i=1, EQUIPMENT_SLOTS_COUNT, 1 do
			local id = character.equipments[i]
			if id then
				local _a = ITEM_LIST[id].attributes[n]
				if _a then
					if _a.a then
						a.a = a.a + _a.a
					end
					if _a.p then
						a.p = a.p + _a.p
					end
				end
			end
		end
		for i=1, #character.buffs, 1 do
			local _a = BUFF_LIST[character.buffs[i].id].attributes[n]
			if _a then
				if _a.a then
					a.a = a.a + _a.a
				end
				if _a.p then
					a.p = a.p + _a.p
				end
			end
		end
	end

	character.calc_stats = function()
		local hp_ratio = character.HP_MAX.t > 0 and character.HP / character.HP_MAX.t or 1.0
		local mp_ratio = character.MP_MAX.t > 0 and character.MP / character.MP_MAX.t or 1.0

		character.collect_attribute("HP_MAX")
		character.HP_MAX.calc()
		character.collect_attribute("MP_MAX")
		character.MP_MAX.calc()
		character.collect_attribute("HP_REC")
		character.HP_REC.calc()
		character.collect_attribute("MP_REC")
		character.MP_REC.calc()
		character.collect_attribute("MOV_SP")
		character.MOV_SP.p = 1.0 + (character.MOV_SP.b + character.MOV_SP.a) / 100.0
		character.MOV_SP.t = character.MOV_SP.d * character.MOV_SP.p
		
		character.collect_attribute("ATK_DMG")
		character.ATK_DMG.calc()
		character.collect_attribute("ATK_SP")
		character.ATK_SP.p = 1.0 + (character.ATK_SP.b + character.ATK_SP.a) / 100.0
		character.ATK_SP.t = math.floor(character.ATK_SP.d / character.ATK_SP.p)
		character.collect_attribute("PHY_INC")
		character.PHY_INC.calc()
		character.collect_attribute("MAG_INC")
		character.MAG_INC.calc()
		character.collect_attribute("DMG_INC")
		character.DMG_INC.calc()
		
		character.collect_attribute("ARMOR")
		character.ARMOR.calc()
		character.collect_attribute("MAG_RES")
		character.MAG_RES.calc()
		local v = 0.06 * character.ARMOR.t
		character.PHY_RED.b = math.floor(v / (1.0 + v) * 100.0)
		character.collect_attribute("PHY_RED")
		character.PHY_RED.calc()
		local v = 0.06 * character.MAG_RES.t
		character.MAG_RED.b = math.floor(v / (1.0 + v) * 100.0)
		character.collect_attribute("MAG_RED")
		character.MAG_RED.calc()
		character.collect_attribute("DMG_RED")
		character.DMG_RED.calc()

		character.HP = hp_ratio * character.HP_MAX.t
		character.MP = mp_ratio * character.MP_MAX.t
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
			if skill_type.type == "Active" then
				if slot.cd == 0 and skill_type.cost_mana <= character.MP then
					if skill_type.target_type ~= "None" and 
						distance_2(target.pos.to_flat(), character.pos.to_flat()) > skill_type.distance + 1 then
							return
					end
					slot.cd = skill_type.cool_down
					character.MP = character.MP - skill_type.cost_mana
					skill_type.logic(character, target)
				end
			end
		end
	end

	character.receive_buff = function(src, id)
		local b = { id=id, t=BUFF_LIST[id].duration }
		table.insert(character.buffs, b)
		character.calc_stats()
	end

	character.receive_item = function(id, num)
		local item_type = ITEM_LIST[id]
		local max_num = item_type.stack_num
		local ori_num = num

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
		if num > 0 then
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
			if item_type.type == "Equipment" then
				local euip_slot = item_type.slot

				local ori_id = character.equipments[euip_slot]
				character.equipments[euip_slot] = slot.id

				character.items[idx] = nil

				if ori_id then
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
		if id then
			if character.receive_item(id, 1) == 0 then
				character.equipments[idx] = nil
				character.calc_stats()

				if character == main_player then
					update_ui_equipment_slots()
				end
			end
		end
	end

	if stats then
		character.calc_stats()
	end

	character.change_state("idle")

	characters[tag][character.name] = character

	return character
end
