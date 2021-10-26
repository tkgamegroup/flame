DEBUG = false

obj_root = scene.find_child("obj_root")
obj_root_n = obj_root.find_component("cNode")

hovering_entity = { p=nil }
hovering_entity_lis = 0
hovering_obj = nil
hovering_pos = vec3(0)
select_mode = false
select_mode_filters = -1
select_mode_callback = nil

local ui_mouse_icon = scene.find_child("mouse_icon")
ui_mouse_icon.element = ui_mouse_icon.find_component("cElement")
ui_mouse_icon.image = ui_mouse_icon.find_component("cImage")

function enter_select_mode(filters, callback)
	select_mode_filters = filters
	select_mode_callback = callback
	select_mode = true
end

function exit_select_mode(tag, target)
	if select_mode then
		select_mode = false
		if select_mode_callback then
			select_mode_callback(tag, target)
		end
	end
end

scene_receiver = scene.find_component("cReceiver")

local e_keyboardkey = find_enum("KeyboardKey")
local e_key_ctrl = e_keyboardkey["Ctrl"]
local e_key_alt = e_keyboardkey["Alt"]
local e_key_a = e_keyboardkey["A"]
local e_key_s = e_keyboardkey["S"]
local e_key_g = e_keyboardkey["G"]
local e_key_q = e_keyboardkey["Q"]
local e_key_w = e_keyboardkey["W"]
local e_key_e = e_keyboardkey["E"]
local e_key_r = e_keyboardkey["R"]
local e_key_d = e_keyboardkey["D"]
local e_key_f = e_keyboardkey["F"]
local e_key_0 = e_keyboardkey["0"]
local e_key_1 = e_keyboardkey["1"]
local e_key_2 = e_keyboardkey["2"]
local e_key_3 = e_keyboardkey["3"]
local e_key_4 = e_keyboardkey["4"]
local e_key_5 = e_keyboardkey["5"]
local e_key_6 = e_keyboardkey["6"]
local e_key_7 = e_keyboardkey["7"]
local e_key_8 = e_keyboardkey["8"]
local e_key_9 = e_keyboardkey["9"]
local e_key_left = e_keyboardkey["Left"]
local e_key_up = e_keyboardkey["Up"]
local e_key_right = e_keyboardkey["Right"]
local e_key_down = e_keyboardkey["Down"]

scene_receiver.add_key_down_listener(function(key)
	if key == e_key_a then
		enter_select_mode(TAG_TERRAIN + TAG_CHARACTER_G2, function(tag, target)
			if tag then
				if tag == TAG_TERRAIN then
					main_player.change_state("attack_on_pos", target)
					auto_attack = true
				else
					main_player.change_state("attack_target", target)
					auto_attack = true
				end
			end
		end)
	elseif key == e_key_q then
		skill_click(1)
	elseif key == e_key_w then
		skill_click(2)
	elseif key == e_key_e then
		skill_click(3)
	elseif key == e_key_r then
		skill_click(4)
	elseif key == e_key_d then
		skill_click(5)
	elseif key == e_key_f then
		skill_click(6)
	end
end)

scene_receiver.add_mouse_left_down_listener(function()
	if select_mode then 
		if s_dispatcher.get_hovering().p ~= scene_receiver.p then return end
		
		if not hovering_obj then return end

		if hovering_obj.tag == TAG_TERRAIN and (select_mode_filters & TAG_TERRAIN) ~= 0 then 
			exit_select_mode(TAG_TERRAIN, hovering_pos)
		elseif hovering_obj.tag == TAG_CHARACTER_G1 and (select_mode_filters & TAG_CHARACTER_G1) ~= 0 then
			exit_select_mode(TAG_CHARACTER_G1, hovering_obj)
		elseif hovering_obj.tag == TAG_CHARACTER_G2 and (select_mode_filters & TAG_CHARACTER_G2) ~= 0 then
			exit_select_mode(TAG_CHARACTER_G2, hovering_obj)
		elseif hovering_obj.tag == TAG_CHARACTER_G3 and (select_mode_filters & TAG_CHARACTER_G3) ~= 0 then
			exit_select_mode(TAG_CHARACTER_G3, hovering_obj)
		end
	end
end)

local auto_attack = false
scene_receiver.add_mouse_right_down_listener(function()
	if select_mode then 
		exit_select_mode(nil, nil) 
		return
	end

	if s_dispatcher.get_hovering().p ~= scene_receiver.p then return end

	if not hovering_obj or hovering_obj.tag == TAG_TERRAIN then
		main_player.change_state("move_to", hovering_pos)
		auto_attack = false
	else
		local tag = hovering_obj.tag
		if tag == TAG_CHARACTER_G1 then

		elseif tag == TAG_CHARACTER_G2 then
			main_player.change_state("attack_target", hovering_obj)
			auto_attack = true
		elseif tag == TAG_CHARACTER_G3 then
			main_player.change_state("interact", hovering_obj)
		elseif tag == TAG_ITEM_OBJ then
			main_player.change_state("interact", hovering_obj)
			auto_attack = false
		end
	end
end)

ui_floating_tips = {}
local e_floating_tip = create_entity("prefabs/ui/floating_tip")
function new_floating_tip(pos, sp, str)
	local item = {}
	item.e = e_floating_tip.copy()
	local text = item.e.find_component("cText")
	text.set_text(str)
	__ui_scene.add_child(item.e)
	item.sp = sp
	item.element = item.e.find_component("cElement")
	item.element.set_pos(pos)
	item.tick = 15
	table.insert(ui_floating_tips, item)
end

local e_shading_flags = find_enum("ShadingFlags")
local e_shading_material = e_shading_flags["Material"]
local e_shading_outline = e_shading_flags["Outline"]

local character_panel = scene.find_child("character_panel")
local hp_bar = character_panel.find_child("hp_bar").find_component("cElement")
local hp_text = character_panel.find_child("hp_text").find_component("cText")
local mp_bar = character_panel.find_child("mp_bar").find_component("cElement")
local mp_text = character_panel.find_child("mp_text").find_component("cText")
local exp_bar = character_panel.find_child("exp_bar").find_component("cElement")
local exp_text = character_panel.find_child("exp_text").find_component("cText")
local gold_text = character_panel.find_child("gold_text").find_component("cText")

local ui_tip = nil

obj_root.add_event(function()
	frame = flame_get_frame()

	-- process characters
	for _, g in pairs({ TAG_CHARACTER_G1, TAG_CHARACTER_G2, TAG_CHARACTER_G3 }) do
		for _, chr in pairs(characters[g]) do
			chr.tick()
		end
	end

	-- process particles 
	for _, pt in pairs(particles) do
		pt.tick()
	end

	-- process projectiles
	for _, pt in pairs(projectiles) do
		pt.tick()
	end

	local mpos = s_dispatcher.get_mouse_pos()
	ui_mouse_icon.element.set_pos(mpos)
	
	local state = main_player.state

	if state == "idle" and auto_attack then
		local target = main_player.find_closest_obj(TAG_CHARACTER_G2, 5)
		if target and not target.dead then
			main_player.change_state("attack_target", target)
		end
	end

	local has_tip = false
	function new_tip(p)
		has_tip = true
		if not ui_tip then
			ui_tip = create_entity("prefabs/ui/tip")
			ui_tip.data = p
			ui_tip.element = ui_tip.find_component("cElement")
			ui_tip.txt_attr = ui_tip.find_child("txt_attr").find_component("cText")
			ui_tip.e_desc = ui_tip.find_child("txt_desc")
			ui_tip.txt_desc = ui_tip.e_desc.find_component("cText")
			__ui_pop.add_child(ui_tip)
			return true
		end
		if ui_tip.data ~= p then
			ui_tip.data = p
			return true
		end
		return false
	end

	function set_item_tip(ui_tip, item_type, in_shop)
		local str = item_type.display_name
		if item_type.type == "Equipment" then
			str = str.."\n"..EQUIPMENT_SLOT_NAMES[item_type.slot]
			for k, v in pairs(item_type.attributes) do
				k = string.gsub(k, "_", " ")
				if type(v) == "string" then
					str = str.."\n"..string.format("%s: %s", k, v)
				else
					if v.a then
						str = str.."\n"..string.format("+%d %s", v.a, k)
					end
					if v.p then
						str = str.."\n"..string.format("+%d%% %s", v.p, k)
					end
				end
			end
		end
		if npc_dialog and npc_dialog.shop and item_type.price ~= 0 then
			str = str.."\n"
			local v = item_type.price
			if not in_shop then
				str = str.."Sell Price: "
				v = math.floor(v / 2) 
			end
			str = str.."\xef\x94\x9e "..tostring(v)
		end
		ui_tip.txt_attr.set_text(str)
	end

	function set_skill_tip(ui_tip, skill_type)
		local str = skill_type.display_name
		if skill_type.type == "Active" then
			str = str.."\n".."Active"
			if skill_type.target_type == "Enemy" then
				str = str.."\n".."Target: Enemy"
				str = str.."\n"..string.format("Distance: %d", skill_type.distance)
			end
			str = str.."\n"..string.format("Mana: %d", skill_type.cost_mana / 10)
			str = str.."\n"..string.format("Cooldown: %ds", skill_type.cool_down / 60)
		else
			str = str.."\n".."Passive"
		end
		ui_tip.txt_attr.set_text(str)
		ui_tip.e_desc.set_visible(true)
		ui_tip.txt_desc.set_text(skill_type.description)
	end

	local hovering_r = s_dispatcher.get_hovering()
	if hovering_r.p == scene_receiver.p then
		local o = camera.node.get_global_pos()
		local d = normalize_3(camera.camera.screen_to_world(mpos) - o)
		local arr = flame_malloc(8)
		hovering_pos = s_physics.raycast(o, d, arr)
		local p = flame_get_p(arr, 0)
		flame_free(arr)

		local _hovering_entity = { p=nil }
		if p then
			_hovering_entity = make_entity(p)
			local tag = _hovering_entity.get_tag()
			if tag == -2147483648 and not DEBUG then 
				_hovering_entity = { p=nil } 
			end
			if select_mode then
				if (tag & select_mode_filters) == 0 then
					local arr = flame_malloc(8)
					if obj_root_n.get_within_circle(hovering_pos.to_flat(), 5, arr, 1, select_mode_filters) > 0 then
						local p = flame_get_p(arr, 0)
						flame_free(arr)
						if p then
							_hovering_entity = make_entity(p)
						end
					end
				end
			end
		end

		function change_outline(e, f)
			local mesh = e.find_component("cMesh")
			if mesh.p then
				mesh.set_shading_flags(f)
			end
			for i=0, e.get_children_count() - 1, 1 do
				local mesh = e.get_child(i).find_component("cMesh")
				if mesh.p then
					mesh.set_shading_flags(f)
				end
			end
		end

		if _hovering_entity.p ~= hovering_entity.p then
			if hovering_entity.p then
				hovering_entity.remove_message_listener(hovering_entity_lis)
				change_outline(hovering_entity, e_shading_material)
			end
			if _hovering_entity.p then
				change_outline(_hovering_entity, e_shading_material + e_shading_outline)
				local hash_destroyed = flame_hash("destroyed")
				hovering_entity_lis = _hovering_entity.add_message_listener(function(m)
					if m == hash_destroyed then
						hovering_entity.remove_message_listener(hovering_entity_lis)
						hovering_entity = { p=nil }
					end
				end)
			end

			hovering_entity = _hovering_entity

			if hovering_entity.p then
				local name = hovering_entity.get_name()
				local tag = hovering_entity.get_tag()
				if tag == TAG_TERRAIN then
					hovering_obj = { tag=TAG_TERRAIN }
				elseif tag == TAG_CHARACTER_G1 then
					hovering_obj = characters[TAG_CHARACTER_G1][name]
				elseif tag == TAG_CHARACTER_G2 then
					hovering_obj = characters[TAG_CHARACTER_G2][name]
				elseif tag == TAG_CHARACTER_G3 then
					hovering_obj = characters[TAG_CHARACTER_G3][name]
				elseif tag == TAG_ITEM_OBJ then
					hovering_obj = item_objs[name]
				end
			else
				hovering_obj = nil
			end
		end

		if hovering_obj then
			if hovering_obj.tag == TAG_ITEM_OBJ then
				if new_tip(hovering_entity.p) then
					ui_tip.element.set_pos(mpos + vec2(10, -20))
					ui_tip.txt_attr.set_text(ITEM_LIST[hovering_obj.id].display_name)
				end
			end
		end

	elseif hovering_r.p then
		local str = hovering_r.get_tooltip()
		if str ~= "" then
			local pos = hovering_r.entity.find_component("cElement").get_point(0) + vec2(0, -100)
			local sp = {}
			for w in str:gmatch("[^%s]+") do
				table.insert(sp, w)
			end
			if sp[1] == "item" then
				if new_tip(hovering_r.p) then
					ui_tip.element.set_pos(pos)
					set_item_tip(ui_tip, ITEM_LIST[sp[2]])
				end
			elseif sp[1] == "shop_item" then
				if new_tip(hovering_r.p) then
					ui_tip.element.set_pos(pos)
					set_item_tip(ui_tip, ITEM_LIST[sp[2]], true)
				end
			elseif sp[1] == "skill" then
				if new_tip(hovering_r.p) then
					ui_tip.element.set_pos(pos)
					set_skill_tip(ui_tip, SKILL_LIST[sp[2]])
				end
			end
		end
	end

	if select_mode then
		ui_mouse_icon.image.set_tile("select")
		ui_mouse_icon.element.set_pivot(vec2(0.5))
	else
		local tag = hovering_obj and hovering_obj.tag or 0
		if tag == TAG_CHARACTER_G2 then
			ui_mouse_icon.image.set_tile("attack")
			ui_mouse_icon.element.set_pivot(vec2(0.5))
		elseif tag == TAG_CHARACTER_G3 then
			ui_mouse_icon.image.set_tile("talk")
			ui_mouse_icon.element.set_pivot(vec2(0.5))
		elseif tag == TAG_ITEM_OBJ then
			ui_mouse_icon.image.set_tile("pick_up")
			ui_mouse_icon.element.set_pivot(vec2(0.5))
		else
			ui_mouse_icon.image.set_tile("mouse")
			ui_mouse_icon.element.set_pivot(vec2(0.0))
		end
	end

	if not has_tip then
		if ui_tip then
			__ui_pop.remove_child(ui_tip)
			ui_tip = nil
		end
	end

	local i = 1
	while i <= #ui_floating_tips do
		local item = ui_floating_tips[i]
		item.element.add_pos(item.sp)
		item.tick = item.tick - 1
		if item.tick <= 0 then
			__ui_scene.remove_child(item.e)
			table.remove(ui_floating_tips, i)
		else
			i = i + 1
		end
	end

	hp_bar.set_scalex(main_player.HP / main_player.HP_MAX.t)
	hp_text.set_text(string.format("%d/%d +%.1f", math.floor(main_player.HP / 10.0), math.floor(main_player.HP_MAX.t / 10.0), main_player.HP_REC.t / 10.0))
	mp_bar.set_scalex(main_player.MP / main_player.MP_MAX.t)
	mp_text.set_text(string.format("%d/%d +%.1f", math.floor(main_player.MP / 10.0), math.floor(main_player.MP_MAX.t / 10.0), main_player.MP_REC.t / 10.0))
	exp_bar.set_scalex(main_player.EXP / main_player.EXP_NEXT)
	exp_text.set_text("LV "..main_player.LV..":  "..main_player.EXP.."/"..main_player.EXP_NEXT)
	gold_text.set_text("Gold "..main_player.GOLD)

	for i=1, SKILL_SLOTS_COUNT, 1 do
		local slot = main_player.skills[i]
		if slot then
			local ui = ui_skill_slots[i]
			if slot.cd > 0 then
				ui.cooldown.set_visible(true)
				ui.cooldown.text.set_text(string.format("%.1f", slot.cd / 60.0))
			else
				ui.cooldown.set_visible(false)
			end
		end
	end
end, 0.0)

function skill_click(idx)
	local slot = main_player.skills[idx]
	if slot then
		local skill_type = SKILL_LIST[slot.id]
		if skill_type.type == "Active" and skill_type.cost_mana <= main_player.MP and slot.cd == 0 then
			if skill_type.target_type ~= "NULL" then
				local filters = -1
				if skill_type.target_type == "Enemy" then filters = TAG_CHARACTER_G2
				end
				local element = ui_skill_slots[idx].element
				element.set_border(2)
				element.set_border_color(vec4(255, 255, 0, 255))
				enter_select_mode(filters, function(tag, target)
					if tag then
						main_player.change_state("cast_to_target", target, { idx=idx, dist=skill_type.distance })
					end
					local element = ui_skill_slots[idx].element
					element.set_border(1)
					element.set_border_color(vec4(255))
				end)
			else
				main_player.change_state("use_skill", nil, { idx=idx })
			end
		end
	end
end

ui_skill_slots = {}
for i=1, SKILL_SLOTS_COUNT, 1 do
	local ui = scene.find_child("skill_slot"..i)
	ui_skill_slots[i] = ui
	local icon = ui.find_child("icon")
	ui.element = icon.find_component("cElement")
	ui.receiver = icon.find_component("cReceiver")
	ui.image = icon.find_component("cImage")
	ui.cooldown = ui.find_child("cooldown")
	ui.cooldown.text = ui.cooldown.find_component("cText")

	ui.receiver.add_mouse_left_down_listener(function()
		skill_click(i)
	end)
end

function update_ui_skill_slots()
	for i=1, SKILL_SLOTS_COUNT, 1 do
		local slot = main_player.skills[i]
		if slot then
			ui_skill_slots[i].image.set_tile(SKILL_LIST[slot.id].name)
			ui_skill_slots[i].receiver.set_tooltip("skill "..slot.id)
		else
			ui_skill_slots[i].image.set_tile("")
			ui_skill_slots[i].receiver.set_tooltip("")
		end
	end
end

ui_equipment_slots = {}
for i=1, EQUIPMENT_SLOTS_COUNT, 1 do
	local ui = scene.find_child("equipment_slot"..i)
	ui_equipment_slots[i] = ui
	local icon = ui.find_child("icon")
	ui.element = icon.find_component("cElement")
	ui.receiver = icon.find_component("cReceiver")
	ui.image = icon.find_component("cImage")

	ui.receiver.add_mouse_right_down_listener(function()
		main_player.take_off_equipment(i)
	end)
end

function update_ui_equipment_slots()
	for i=1, EQUIPMENT_SLOTS_COUNT, 1 do
		local id = main_player.equipments[i]
		if id then
			ui_equipment_slots[i].image.set_tile(ITEM_LIST[id].name)
			ui_equipment_slots[i].receiver.set_tooltip("item "..id)
		else
			ui_equipment_slots[i].image.set_tile("")
			ui_equipment_slots[i].receiver.set_tooltip("")
		end
	end
end

ui_item_slots = {}
for i=1, ITEM_SLOTS_COUNT, 1 do
	local ui = scene.find_child("item_slot"..i)
	ui_item_slots[i] = ui
	local icon = ui.find_child("icon")
	ui.element = icon.find_component("cElement")
	ui.receiver = icon.find_component("cReceiver")
	ui.image = icon.find_component("cImage")

	ui.receiver.add_mouse_right_down_listener(function()
		if npc_dialog and npc_dialog.shop then
			local slot = main_player.items[i]
			if slot then
				local price = ITEM_LIST[slot.id].price
				if price > 0 then
					price = math.floor(price / 2)
					price = price * slot.num
					main_player.GOLD = main_player.GOLD + price
					main_player.items[i] = nil
					update_ui_item_slots()
				end
			end
		else
			main_player.use_item(i)
		end
	end)
end

function update_ui_item_slots()
	for i=1, ITEM_SLOTS_COUNT, 1 do
		local slot = main_player.items[i]
		if slot then
			ui_item_slots[i].image.set_tile(ITEM_LIST[slot.id].name)
			ui_item_slots[i].receiver.set_tooltip("item "..slot.id)
		else
			ui_item_slots[i].image.set_tile("")
			ui_item_slots[i].receiver.set_tooltip("")
		end
	end
end

local attributes_btn = scene.find_child("attributes_btn")
attributes_btn.wnd = nil
attributes_btn.find_component("cReceiver").add_mouse_click_listener(function()
	if not attributes_btn.wnd then
		attributes_btn.wnd = create_entity("prefabs/ui/attributes")
		attributes_btn.wnd.find_component("cWindow").add_close_listener(function()
			attributes_btn.wnd = nil
		end)

		local hp_max_text = attributes_btn.wnd.find_child("hp_max_text").find_component("cText")
		local mp_max_text = attributes_btn.wnd.find_child("mp_max_text").find_component("cText")
		local lv_text = attributes_btn.wnd.find_child("lv_text").find_component("cText")
		local exp_text = attributes_btn.wnd.find_child("exp_text").find_component("cText")
		local atk_dmg_text = attributes_btn.wnd.find_child("atk_dmg_text").find_component("cText")
		local arrmor_text = attributes_btn.wnd.find_child("arrmor_text").find_component("cText")
		local mov_sp_text = attributes_btn.wnd.find_child("mov_sp_text").find_component("cText")
		local sta_text = attributes_btn.wnd.find_child("sta_text").find_component("cText")
		local spi_text = attributes_btn.wnd.find_child("spi_text").find_component("cText")
		local luk_text = attributes_btn.wnd.find_child("luk_text").find_component("cText")
		local str_text = attributes_btn.wnd.find_child("str_text").find_component("cText")
		local agi_text = attributes_btn.wnd.find_child("agi_text").find_component("cText")
		local int_text = attributes_btn.wnd.find_child("int_text").find_component("cText")
		local points_text = attributes_btn.wnd.find_child("points_text").find_component("cText")

		local add_sta_btn = attributes_btn.wnd.find_child("add_sta_btn")
		local add_spi_btn = attributes_btn.wnd.find_child("add_spi_btn")
		local add_luk_btn = attributes_btn.wnd.find_child("add_luk_btn")
		local add_str_btn = attributes_btn.wnd.find_child("add_str_btn")
		local add_agi_btn = attributes_btn.wnd.find_child("add_agi_btn")
		local add_int_btn = attributes_btn.wnd.find_child("add_int_btn")
		if main_player.attribute_points > 0 then
			add_sta_btn.set_visible(true)
			add_spi_btn.set_visible(true)
			add_luk_btn.set_visible(true)
			add_str_btn.set_visible(true)
			add_agi_btn.set_visible(true)
			add_int_btn.set_visible(true)
		end

		function update()
			hp_max_text.set_text(string.format("HP MAX: %d", math.floor(main_player.HP_MAX.t / 10.0)))
			mp_max_text.set_text(string.format("MP MAX: %d", math.floor(main_player.MP_MAX.t / 10.0)))
			lv_text.set_text(string.format("LV: %d", main_player.LV))
			exp_text.set_text(string.format("EXP: %d/%d", main_player.EXP, main_player.EXP_NEXT))
			atk_dmg_text.set_text(string.format("ATK: %d (%s)", math.floor(main_player.ATK_DMG.t / 10.0), main_player.ATK_TYPE))
			arrmor_text.set_text(string.format("ARRMMOR: %d", main_player.ARMOR.t))
			mov_sp_text.set_text(string.format("MOV SP: %d", main_player.MOV_SP.a))
		
			sta_text.set_text(string.format("STA: %d", main_player.STA.t))
			spi_text.set_text(string.format("SPI: %d", main_player.SPI.t))
			luk_text.set_text(string.format("LUK: %d", main_player.LUK.t))
			str_text.set_text(string.format("STR: %d", main_player.STR.t))
			agi_text.set_text(string.format("AGI: %d", main_player.AGI.t))
			int_text.set_text(string.format("INT: %d", main_player.INT.t))
			points_text.set_text(string.format("Points: %d", main_player.attribute_points))
		end

		function add_attribute(attr)
			if main_player.attribute_points > 0 then
				main_player[attr] = main_player[attr] + 1
				main_player.calc_stats()
				main_player.attribute_points = main_player.attribute_points - 1
				update()
				if main_player.attribute_points == 0 then
					add_sta_btn.set_visible(false)
					add_spi_btn.set_visible(false)
					add_luk_btn.set_visible(false)
					add_str_btn.set_visible(false)
					add_agi_btn.set_visible(false)
					add_int_btn.set_visible(false)
				end
			end
		end
		add_sta_btn.find_component("cReceiver").add_mouse_click_listener(function()
			add_attribute("STA")
		end)
		add_spi_btn.find_component("cReceiver").add_mouse_click_listener(function()
			add_attribute("SPI")
		end)
		add_luk_btn.find_component("cReceiver").add_mouse_click_listener(function()
			add_attribute("LUK")
		end)
		add_str_btn.find_component("cReceiver").add_mouse_click_listener(function()
			add_attribute("STR")
		end)
		add_agi_btn.find_component("cReceiver").add_mouse_click_listener(function()
			add_attribute("AGI")
		end)
		add_int_btn.find_component("cReceiver").add_mouse_click_listener(function()
			add_attribute("INT")
		end)

		update()

		__ui_pop.add_child(attributes_btn.wnd)
	end
end)

npc_dialog = nil
function open_npc_dialog(npc)
	if npc_dialog and npc_dialog.npc == npc then return end

	npc_dialog = create_entity("prefabs/ui/npc_dialog")
	npc_dialog.content = npc_dialog.find_child("content")
	local c_window = npc_dialog.find_component("cWindow")
	c_window.set_title(npc.name)
	c_window.add_close_listener(function()
		npc_dialog = nil
	end)
	npc_dialog.npc = npc

	local ui_text = create_entity("prefabs/text")
	ui_text.find_component("cText").set_text(npc.interact.text)
	npc_dialog.add_child(ui_text)

	local options = npc.interact.options
	for i=1, #options, 1 do
		local ui_option = create_entity("prefabs/ui/npc_dialog_option")
		local option = options[i]
		ui_option.find_component("cText").set_text(option.title)
		ui_option.find_component("cReceiver").add_mouse_click_listener(function()
			npc_dialog.content.remove_all_children()

			if option.type == "shop" then
				npc_dialog.shop = create_entity("prefabs/ui/npc_dialog_shop")
				npc_dialog.shop.list = npc_dialog.shop.find_child("list")
				local items = option.items
				for i=1, #items, 1 do
					local item = items[i]
					local item_type = ITEM_LIST[item.id]
					local ui_item = create_entity("prefabs/ui/npc_dialog_shop_item")
					local e_icon = ui_item.find_child("icon")
					e_icon.find_component("cImage").set_tile(item_type.name)
					e_icon.find_component("cReceiver").set_tooltip("shop_item "..item.id)
					ui_item.find_child("name").find_component("cText").set_text(item_type.display_name)
					ui_item.find_child("buy").find_component("cReceiver").add_mouse_click_listener(function()
						if main_player.GOLD >= item_type.price then
							if main_player.receive_item(item.id, 1) == 0 then
								main_player.GOLD = main_player.GOLD - item_type.price
							end
						end
					end)
					npc_dialog.shop.list.add_child(ui_item)
				end
				npc_dialog.content.add_child(npc_dialog.shop)
			elseif option.type == "trainer" then
				npc_dialog.trainer = create_entity("prefabs/ui/npc_dialog_trainer")
				npc_dialog.content.add_child(npc_dialog.trainer)
			end
		end)
		npc_dialog.add_child(ui_option)
	end

	__ui_pop.add_child(npc_dialog)
end

local e_terrain = scene.find_child("terrain")
local terrain = e_terrain.find_component("cTerrain")
local terrain_ext = terrain.get_extent()
local terrain_height_tex = terrain.get_height_texture()
local terrain_normal_tex = terrain.get_normal_texture()
local terrain_obj_root = e_terrain.find_child("obj_root")
e_terrain.set_tag(TAG_TERRAIN)

local e_chest = create_entity("prefabs/chest")
function add_item_obj(pos, item_id, item_num)
	local e = e_chest.copy()
	e.set_name("item_obj_"..tostring(math.random(1, 10000)))
    e.find_component("cNode").set_pos(s_physics.raycast(vec3(pos.x, 1000, pos.y), vec3(0, -1, 0)))
	make_item_obj(e, item_id, item_num)
	obj_root.add_child(e)
end

local e_particles = {}
function add_particle(name, pos, ttl)
	local e = e_particles[name]
	if not e then
		e = create_entity("prefabs/particles/"..name)
		e_particles[name] = e
	end
	e = e.copy()
	e.set_name("particles"..tostring(math.random(1, 10000)))
	e.find_component("cNode").set_pos(pos)
	make_particle(e, ttl)
	obj_root.add_child(e)
end

local e_projectiles = {}
function add_projectile(name, target, pos, sp, cb)
	local e = e_projectiles[name]
	if not e then
		e = create_entity("prefabs/projectiles/"..name)
		e_projectiles[name] = e
	end
	e = e.copy()
	e.set_name("projectile_"..tostring(math.random(1, 10000)))
	e.find_component("cNode").set_pos(pos)
	make_projectile(e, target, sp, cb)
	obj_root.add_child(e)
end

local e_player = create_entity("prefabs/player")

local e = e_player.copy()
e.set_name("main_player")
e.find_component("cNode").set_pos(vec3(215, 65, 215))
main_player = make_player(e)
main_player.learn_skill("fire_ball")
main_player.learn_skill("ice_bolt")
main_player.GOLD = 500
main_player.awake()
obj_root.add_child(e)

local e_npcs = {}
function add_npc(pos, ID, n)
	local e = e_npcs[ID]
	if e == nil then
		e = create_entity("prefabs/"..ID)
		e_npcs[ID] = e
	end
	e = e.copy()
	if not n then
		n = "npc_"..tostring(math.floor(math.random() * 10000))
	end
	e.set_name(n)
	e.find_component("cNode").set_pos(pos)
	make_npc(e, ID)
	obj_root.add_child(e)
end

local basic_items = {
	"wooden_stick",
	"wooden_shield",
	"leather_hat",
	"leather_cloth",
	"leather_pants",
	"leather_shoes"
}
add_item_obj(vec2(215, 215) + circle_rand(1.0), basic_items[math.random(1, #basic_items)], 1)

add_npc(vec3(217, 65, 213), "archmage", "Archmage")

local e_grasses = {}
table.insert(e_grasses, { e=create_entity("prefabs/grass1"), p=0.35 })
table.insert(e_grasses, { e=create_entity("prefabs/grass2"), p=0.35 })
table.insert(e_grasses, { e=create_entity("prefabs/grass3"), p=0.1 })
table.insert(e_grasses, { e=create_entity("prefabs/grass4"), p=0.1 })
table.insert(e_grasses, { e=create_entity("prefabs/grass5"), p=0.05 })
table.insert(e_grasses, { e=create_entity("prefabs/grass6"), p=0.05 })

local e_trees = {}
table.insert(e_trees, { e=create_entity("prefabs/tree1"), p=0.5 })
table.insert(e_trees, { e=create_entity("prefabs/tree2"), p=0.5 })

local e_rocks = {}
table.insert(e_rocks, { e=create_entity("prefabs/rock1"), p=0.2 })
table.insert(e_rocks, { e=create_entity("prefabs/rock2"), p=0.2 })
table.insert(e_rocks, { e=create_entity("prefabs/rock3"), p=0.2 })
table.insert(e_rocks, { e=create_entity("prefabs/rock4"), p=0.2 })
table.insert(e_rocks, { e=create_entity("prefabs/rock5"), p=0.2 })

local grid_size = 10
local grid_num = terrain_ext.x / grid_size
local grids = {}
for i=1, grid_num * grid_num, 1 do
	grids[i] = false
end

function build_grid(x, z)
	if x < 0 then x = 0 end
	if z < 0 then z = 0 end
	if x == 21 and z == 21 then return end

	if x >= grid_num then x = grid_num - 1 end
	if z >= grid_num then z = grid_num - 1 end

	local idx = z * grid_num + x
	if not grids[idx] then
		grids[idx] = true
		
		local range = vec4(x * grid_size, z * grid_size, grid_size, grid_size)
		terrain_scatter(terrain_ext, terrain_height_tex, terrain_normal_tex, terrain_obj_root, range, 
			0.15, e_grasses, 0.05, vec2(40.0, 200), vec2(0.7, 1.0), vec2(0, 360), vec2(3.0, 4.0))
			
		terrain_scatter(terrain_ext, terrain_height_tex, terrain_normal_tex, terrain_obj_root, range, 
			4.2, e_trees, 0.5, vec2(35.0, 200), vec2(0.9, 1.0), vec2(0, 360), vec2(0.8, 1.2))
			
		terrain_scatter(terrain_ext, terrain_height_tex, terrain_normal_tex, terrain_obj_root, range, 
			4.7, e_rocks, 0.5, vec2(0.0, 200), vec2(0.7, 1.0), vec2(0, 360), vec2(0.3, 0.4))
	end
end

local ns = {}
for str in string.gmatch(flame_load_file("blocks.txt"), "([^%s]+)") do
	table.insert(ns, tonumber(str))
end

local blocks = {}
local n_blocks = ns[1]
for i=1, n_blocks, 1 do
	blocks[i] = {}
end
local i=2
for x=1, n_blocks, 1 do
	for y=1, n_blocks, 1 do
		blocks[x][y] = ns[i]
		i = i + 1
	end
end

local block_ext = terrain_ext.x / n_blocks

obj_root.add_event(function()
	if npc_dialog and distance_3(main_player.pos, npc_dialog.npc.pos) > 1.5 then
		__ui_pop.remove_child(npc_dialog)
		npc_dialog = nil
	end

	local px = math.floor(main_player.pos.x / grid_size)
	local pz = math.floor(main_player.pos.z / grid_size)

	build_grid(px, pz)
	build_grid(px - 1, pz)
	build_grid(px + 1, pz)
	build_grid(px, pz - 1)
	build_grid(px, pz + 1)
	build_grid(px - 1, pz - 1)
	build_grid(px + 1, pz - 1)
	build_grid(px - 1, pz + 1)
	build_grid(px + 1, pz + 1)

	for n=1, 10, 1 do
		local pos = main_player.pos.to_flat() + circle_rand(30.0)
		if pos.x > 10.0 and pos.x < 390.0 and pos.y > 10.0 and pos.y < 390.0 then
			local bx = math.floor(pos.x / block_ext) + 1
			local by = math.floor(pos.y / block_ext) + 1
			if blocks[bx][by] == 0 and not obj_root_n.is_any_within_circle(pos, 10, TAG_CHARACTER_G2) then
				local pos = s_physics.raycast(vec3(pos.x, 1000, pos.y), vec3(0, -1, 0))
				if math.random() < 0.1 then
					add_npc(pos, "crazy_zombie")
				else
					add_npc(pos, "zombie")
				end
			end
		end
	end

	for _, chr in pairs(characters[TAG_CHARACTER_G2]) do
		if obj_root_n.is_any_within_circle(chr.pos.to_flat(), 50, TAG_CHARACTER_G1) then
			chr.awake()
		else
			chr.sleep()
		end
	end
end, 1.0)

local e_debugger = scene.find_child("debugger")
if e_debugger.p and e_debugger.get_visible() then
	DEBUG = true

	local hovering_pos_text = e_debugger.find_child("hovering_pos_text").find_component("cText")
	local hovering_entity_pos_text = e_debugger.find_child("hovering_entity_pos_text").find_component("cText")
	obj_root.add_event(function()
		hovering_pos_text.set_text(string.format("%.2f %.2f %.2f", hovering_pos.x, hovering_pos.y, hovering_pos.z))
		if hovering_entity.p and hovering_entity.get_tag() ~= TAG_TERRAIN then
			local node = hovering_entity.find_component("cNode")
			local pos = node.get_global_pos()
			hovering_entity_pos_text.set_text(string.format("%.2f %.2f %.2f", pos.x, pos.y, pos.z))
			
			if s_dispatcher.get_keyboard_state(e_key_left) then
				node.add_euler(vec3(-0.5, 0, 0))
			end
			if s_dispatcher.get_keyboard_state(e_key_right) then
				node.add_euler(vec3(0.5, 0, 0))
			end
			if s_dispatcher.get_keyboard_state(e_key_up) then
				node.add_pos(vec3(0, 0.01, 0))
			end
			if s_dispatcher.get_keyboard_state(e_key_down) then
				node.add_pos(vec3(0, -0.01, 0))
			end
		else
			hovering_entity_pos_text.set_text("")
		end
	end, 0.0)
end
