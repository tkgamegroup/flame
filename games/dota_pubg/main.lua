obj_root = scene.find_child("obj_root")
obj_root_n = obj_root.find_component("cNode")

projectile_root = scene.find_child("projectile_root")

TAG_TERRAIN = 1
TAG_CHARACTER_G1 = 2
TAG_CHARACTER_G2 = 3
TAG_ITEM_OBJ = 4

ctrl_pressing = false
alt_pressing = false
hovering_entity = { p=nil }
hovering_entity_lis = 0
hovering_obj = nil
hovering_pos = vec3(0)
select_mode = false
select_mode_callback = nil

local ui_mouse_icon = scene.find_child("mouse_icon")
ui_mouse_icon.element = ui_mouse_icon.find_component("cElement")
ui_mouse_icon.image = ui_mouse_icon.find_component("cImage")

function enter_select_mode(callback)
	select_mode_callback = callback
	if not select_mode then
		ui_mouse_icon.image.set_tile("select")
		ui_mouse_icon.element.set_pivotx(0.5)
		ui_mouse_icon.element.set_pivoty(0.5)
		select_mode = true
	end
end

function exit_select_mode(target)
	if select_mode then
		ui_mouse_icon.image.set_tile("mouse")
		ui_mouse_icon.element.set_pivotx(0.0)
		ui_mouse_icon.element.set_pivoty(0.0)
		select_mode = false
		if select_mode_callback and target then
			select_mode_callback(target)
		end
	end
end

scene_receiver = scene.find_component("cReceiver")

local e_keyboardkey = find_enum("KeyboardKey")
local e_key_ctrl = e_keyboardkey["Ctrl"]
local e_key_alt = e_keyboardkey["Alt"]
local e_key_q = e_keyboardkey["Q"]

scene_receiver.add_key_down_listener(function(key)
	if key == e_key_ctrl then
		ctrl_pressing = true
	elseif key == e_key_alt then
		alt_pressing = true
	elseif key == e_key_q then
		skill_click(1)
	end
end)

scene_receiver.add_key_up_listener(function(key)
	if key == e_key_ctrl then
		ctrl_pressing = false
	elseif key == e_key_alt then
		alt_pressing = false
	end
end)

scene_receiver.add_mouse_left_down_listener(function()
	if select_mode then 
		if s_dispatcher.get_hovering().p ~= scene_receiver.p then return end
		
		if not hovering_obj then return end
		if hovering_obj.name == "terrain" then return end
		if hovering_obj.tag == TAG_CHARACTER_G2 then
			exit_select_mode(hovering_obj)
		end
	end
end)

scene_receiver.add_mouse_right_down_listener(function()
	if select_mode then 
		exit_select_mode(nil) 
		return
	end

	if s_dispatcher.get_hovering().p ~= scene_receiver.p then return end

	if not hovering_obj then return end
	if hovering_obj.name == "terrain" then
		if alt_pressing then
			main_player.change_state("attack_on_pos", hovering_pos)
		elseif ctrl_pressing then
			main_player.change_state("pick_up_on_pos", hovering_pos)
		else
			main_player.change_state("move_to", hovering_pos)
		end
	else
		local tag = hovering_obj.tag
		if tag == TAG_CHARACTER_G1 then

		elseif tag == TAG_CHARACTER_G2 then
			main_player.change_state("attack_target", hovering_obj)
		elseif tag == TAG_ITEM_OBJ then
			main_player.change_state("pick_up", hovering_obj)
		end
	end
end)

local ui_action_tip1 = scene.find_child("action_tip1")
ui_action_tip1.element = ui_action_tip1.find_component("cElement")
ui_action_tip1.image = ui_action_tip1.find_component("cImage")
local ui_action_tip2 = scene.find_child("action_tip2")
ui_action_tip2.element = ui_action_tip2.find_component("cElement")
ui_action_tip2.image = ui_action_tip2.find_component("cImage")

local ui_tip = nil

ui_floating_tips = {}
local e_floating_tip = create_entity("floating_tip")
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
local exp_text = character_panel.find_child("exp_text").find_component("cText")

local target = { pos = vec3(220, 60, 220) }

obj_root.add_event(function()
	-- process characters
	for g=1, 2, 1 do
		for _, char in pairs(characters[g]) do
			if not char.sleeping then
				char.tick()
			end
		end
	end

	-- process item objs
	for _, item in pairs(item_objs) do
		item.pos = item.node.get_global_pos()
	end

	-- process projectiles
	for _, prjtl in pairs(projectiles) do
		prjtl.pos = prjtl.node.get_global_pos()

		local tpos = prjtl.target.pos + vec3(0, prjtl.target.height * 0.8, 0)
		local l, d = length_and_dir_3(tpos - prjtl.pos)
		if d then
			prjtl.node.look_at(tpos)
		end

		if l <= prjtl.speed then
			prjtl.die()
		else
			prjtl.node.add_pos(d * prjtl.speed)
		end
	end

	local mpos = s_dispatcher.get_mouse_pos()
	ui_mouse_icon.element.set_pos(mpos)

	local state = main_player.state
	if state == "move_to" or state == "attack_on_pos" or state == "pick_up_on_pos" then
		ui_action_tip1.set_visible(true)
		ui_action_tip1.element.set_pos(camera.camera.world_to_screen(main_player.target_pos))
		ui_action_tip1.image.set_tile("move")
	else
		ui_action_tip1.set_visible(false)
	end
	if state == "attack_target" or state == "attack_on_pos" then
		if main_player.target then
			ui_action_tip2.set_visible(true)
			ui_action_tip2.element.set_pos(camera.camera.world_to_screen(main_player.target.pos + vec3(0, 1.8, 0)) + vec2(0, 10))
			ui_action_tip2.image.set_tile("attack")
		else
			ui_action_tip2.set_visible(false)
		end
	elseif state == "pick_up" or state == "pick_up_on_pos" then
		if main_player.target then
			ui_action_tip2.set_visible(true)
			ui_action_tip2.element.set_pos(camera.camera.world_to_screen(main_player.target.pos) + vec2(0, 10))
			ui_action_tip2.image.set_tile("pick_up")
		else
			ui_action_tip2.set_visible(false)
		end
	else
		ui_action_tip2.set_visible(false)
	end

	local has_tip = false
	function new_tip(p)
		has_tip = true
		if not ui_tip then
			ui_tip = create_entity("tip")
			ui_tip.data = p
			ui_tip.element = ui_tip.find_component("cElement")
			ui_tip.text = ui_tip.find_component("cText")
			__ui.add_child(ui_tip)
			return true
		end
		if ui_tip.data ~= p then
			ui_tip.data = p
			return true
		end
		return false
	end

	function item_tip(item_type)
		local str = item_type.display_name
		if item_type.type == "EQUIPMENT" then
			str = str.."\n"..EQUIPMENT_SLOT_NAMES[item_type.data.slot]
			if item_type.data.slot == EQUIPMENT_SLOT_MAIN_HAND then
				str = str.."\n"..string.format("%s DMG %d", item_type.data.ATK_TYPE, item_type.data.ATK)
			end
		end
		return str
	end

	local hovering_r = s_dispatcher.get_hovering().p
	if hovering_r == scene_receiver.p then
		local o = camera.node.get_global_pos()
		local d = normalize_3(camera.camera.screen_to_world(mpos) - o)
		local pe = flame_malloc(8)
		hovering_pos = s_physics.raycast(o, d, pe)
		local p = flame_get(pe, 0, e_type_pointer, e_else_type, 1, 1)
		flame_free(pe)

		local _hovering_entity = nil
		if p then
			_hovering_entity = make_entity(p)
		else
			_hovering_entity = { p=nil }
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
				if name == "terrain" then
					hovering_obj = { name="terrain", tag=0 }
				else
					local tag = hovering_entity.get_tag()
					if tag == TAG_CHARACTER_G1 and name ~= "main_player" then
						hovering_obj = characters[1][name]
					elseif tag == TAG_CHARACTER_G2 then
						hovering_obj = characters[2][name]
					elseif tag == TAG_ITEM_OBJ then
						hovering_obj = item_objs[name]
					end
				end
			else
				hovering_obj = nil
			end
		end

		if not has_tip and hovering_obj then
			if hovering_obj.tag == TAG_ITEM_OBJ then
				if new_tip(hovering_entity.p) then
					ui_tip.element.set_pos(mpos + vec2(10, -20))
					ui_tip.text.set_text(ITEM_LIST[hovering_obj.id].display_name)
				end
			end
		end
	else
		if not has_tip then
			for i=1, ITEM_SLOTS_COUNT, 1 do
				local ui_slot = ui_item_slots[i]
				local slot = main_player.items[i]
				if slot and ui_slot.receiver.p == hovering_r then
					if new_tip(ui_slot.receiver.p) then
						ui_tip.element.set_pos(ui_slot.element.get_point(0) + vec2(0, -100))
						ui_tip.text.set_text(item_tip(ITEM_LIST[slot.id]))
					end
				end
			end
		end
		if not has_tip then
			for i=1, EQUIPMENT_SLOTS_COUNT, 1 do
				local ui_slot = ui_equipment_slots[i]
				local equipment = main_player.equipments[i]
				if equipment ~= 0 and ui_slot.receiver.p == hovering_r then
					if new_tip(ui_slot.receiver.p) then
						ui_tip.element.set_pos(ui_slot.element.get_point(0) + vec2(0, -100))
						ui_tip.text.set_text(item_tip(ITEM_LIST[equipment]))
					end
				end
			end
		end
	end

	if not has_tip then
		if ui_tip then
			__ui.remove_child(ui_tip)
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

	hp_bar.set_scalex(main_player.HP / main_player.HP_MAX)
	hp_text.set_text(string.format("%d/%d +%.1f", math.floor(main_player.HP / 10.0), math.floor(main_player.HP_MAX / 10.0), main_player.HP_RECOVER / 10.0))
	mp_bar.set_scalex(main_player.MP / main_player.MP_MAX)
	mp_text.set_text(string.format("%d/%d +%.1f", math.floor(main_player.MP / 10.0), math.floor(main_player.MP_MAX / 10.0), main_player.MP_RECOVER / 10.0))
	exp_bar.set_scalex(main_player.EXP / main_player.EXP_NEXT)
	exp_text.set_text("LV "..main_player.LV..":  "..main_player.EXP.."/"..main_player.EXP_NEXT)
end, 0.0)

local e_grasses = {}
local e = create_entity("D:\\assets\\vegetation\\grass1.prefab")
table.insert(e_grasses, e)

local e_trees = {}
local e = create_entity("D:\\assets\\vegetation\\tree1.prefab")
table.insert(e_trees, e)

local e_terrain = scene.find_child("terrain")
local terrain = e_terrain.find_component("cTerrain")
local terrain_ext = terrain.get_extent()
local terrain_height_tex = terrain.get_height_texture()
local terrain_normal_tex = terrain.get_normal_texture()
local vegetation_root = e_terrain.find_child("vegetation")
terrain_scatter(terrain_ext, terrain_height_tex, terrain_normal_tex, vegetation_root, vec4(190, 190, 20, 20), 0.2, e_grasses, 0.03, 0.8)
terrain_scatter(terrain_ext, terrain_height_tex, terrain_normal_tex, vegetation_root, vec4(190, 190, 20, 20), 2.5, e_trees, 0.1, 0.8)
--[[
local e_plants = {}
table.insert(e_plants, create_entity("D:\\assets\\vegetation\\plant1.prefab"))

scatter(vec4(0.0, 0.0, 400.0, 400.0), 0.2, e_grasses, 0.05, 2.5)
scatter(vec4(0.0, 0.0, 400.0, 400.0), 0.5, e_plants, 0.0025, 1.0)
]]

function skill_click(idx)
	local slot = main_player.skills[idx]
	if slot then
		local skill_type = SKILL_LIST[slot.id]
		if skill_type.type == "ACTIVE" and skill_type.data.cast_mana <= main_player.MP then
			if skill_type.data.target_type ~= "NULL" then
				enter_select_mode(function(target)
					main_player.use_skill(idx, target)
				end)
			else
				main_player.use_skill(idx)
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

	ui.receiver.add_mouse_left_down_listener(function()
		skill_click(i)
	end)
end

function update_ui_skill_slots()
	for i=1, SKILL_SLOTS_COUNT, 1 do
		local slot = main_player.skills[i]
		if not slot then
			ui_skill_slots[i].image.set_tile("")
		else
			ui_skill_slots[i].image.set_tile(SKILL_LIST[slot.id].name)
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
		main_player.use_equipment(i)
	end)
end

function update_ui_equipment_slots()
	for i=1, EQUIPMENT_SLOTS_COUNT, 1 do
		local id = main_player.equipments[i]
		if id == 0 then
			ui_equipment_slots[i].image.set_tile("")
		else
			ui_equipment_slots[i].image.set_tile(ITEM_LIST[id].name)
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
		main_player.use_item(i)
	end)
end

function update_ui_item_slots()
	for i=1, ITEM_SLOTS_COUNT, 1 do
		local slot = main_player.items[i]
		if not slot then
			ui_item_slots[i].image.set_tile("")
		else
			ui_item_slots[i].image.set_tile(ITEM_LIST[slot.id].name)
		end
	end
end

local attributes_btn = scene.find_child("attributes_btn")
attributes_btn.wnd = nil
attributes_btn.find_component("cReceiver").add_mouse_click_listener(function()
	if not attributes_btn.wnd_openning then
		attributes_btn.wnd = create_entity("attributes")
		attributes_btn.wnd.find_driver("dWindow").add_close_listener(function()
			__ui.remove_child(attributes_btn.wnd)
			attributes_btn.wnd = nil
		end)

		local hp_max_text = attributes_btn.wnd.find_child("hp_max_text").find_component("cText")
		local mp_max_text = attributes_btn.wnd.find_child("mp_max_text").find_component("cText")
		local lv_text = attributes_btn.wnd.find_child("lv_text").find_component("cText")
		local exp_text = attributes_btn.wnd.find_child("exp_text").find_component("cText")
		local phy_dmg_text = attributes_btn.wnd.find_child("phy_dmg_text").find_component("cText")
		local mag_dmg_text = attributes_btn.wnd.find_child("mag_dmg_text").find_component("cText")
		local atk_dmg_text = attributes_btn.wnd.find_child("atk_dmg_text").find_component("cText")
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
			hp_max_text.set_text(string.format("HP MAX: %d", math.floor(main_player.HP_MAX / 10.0)))
			mp_max_text.set_text(string.format("MP MAX: %d", math.floor(main_player.MP_MAX / 10.0)))
			lv_text.set_text(string.format("LV: %d", main_player.LV))
			exp_text.set_text(string.format("EXP: %d/%d", main_player.EXP, main_player.EXP_NEXT))
			phy_dmg_text.set_text(string.format("PHY DMG: %d", main_player.PHY_DMG))
			mag_dmg_text.set_text(string.format("MAG DMG: %d", main_player.MAG_DMG))
			atk_dmg_text.set_text(string.format("ATK DMG: %d (%s)", math.floor(main_player.ATK_DMG / 10.0), main_player.ATK_TYPE))
		
			sta_text.set_text(string.format("STA: %d", main_player.STA))
			spi_text.set_text(string.format("SPI: %d", main_player.SPI))
			luk_text.set_text(string.format("LUK: %d", main_player.LUK))
			str_text.set_text(string.format("STR: %d", main_player.STR))
			agi_text.set_text(string.format("AGI: %d", main_player.AGI))
			int_text.set_text(string.format("INT: %d", main_player.INT))
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

		__ui.add_child(attributes_btn.wnd)
	end
end)

for i=1, 10, 1 do
	local e = create_entity("remore")
	e.set_name("enemy_"..tostring(math.floor(math.random() * 10000)))
	--e.find_component("cNode").set_pos(vec3(math.random() * 400, 200, math.random() * 400))
	e.find_component("cNode").set_pos(vec3(190 + math.random() * 20, 100, 190 + math.random() * 20))
	make_npc(e, 1)
	obj_root.add_child(e)
end

local e_chest = create_entity("chest")
function add_chest(pos, item_id, item_num)
	local e = e_chest.copy()
	e.set_name("item_obj_"..tostring(math.random(1, 10000)))
	e.find_component("cNode").set_pos(pos)
	obj_root.add_child(e)
	make_item_obj(e, item_id, item_num)
end

local e = create_entity("player")
e.set_name("main_player")
e.find_component("cNode").set_pos(vec3(200, 65, 200))
main_player = make_player(e)
main_player.learn_skill(1)
main_player.awake()
obj_root.add_child(e)

obj_root.add_event(function()
	for _, char in pairs(characters[2]) do
		if obj_root_n.is_any_within_circle(char.pos.to_flat(), 50, TAG_CHARACTER_G2) then
			char.awake()
		else
			char.sleep()
		end
	end
end, 1.0)
