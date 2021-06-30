obj_root = scene.find_child("obj_root")
obj_root_n = obj_root.find_component("cNode")

alt_pressing = false

scene_receiver = scene.find_component("cReceiver")
local e_key_alt = find_enum("KeyboardKey")["Alt"]
scene_receiver.add_key_down_listener(function(key)
	if key == e_key_alt then
		alt_pressing = true
	end
end)
scene_receiver.add_key_up_listener(function(key)
	if key == e_key_alt then
		alt_pressing = false
	end
end)

PLAYER_STAT = {
	HP_MAX = 200,
	HP_RECOVER = 1,
	ATTACK_DAMAGE = 15
}

STAT = {
	HP_MAX = 100,
	HP_RECOVER = 1,
	ATTACK_DAMAGE = 10
}

local e = create_entity("player")
e.set_name("player")
e.find_component("cNode").set_pos(vec3(200, 65, 200))
main_player = make_character(e, 1, PLAYER_STAT)
make_player(main_player)
main_player.awake()
obj_root.add_child(e)

local e_grasses = {}
local e = create_entity("D:\\assets\\vegetation\\grass1.prefab")
table.insert(e_grasses, e)

local e_trees = {}
local e = create_entity("D:\\assets\\vegetation\\tree1.prefab")
table.insert(e_trees, e)

local e_terrain = scene.find_child("terrain")
local terrain = e_terrain.find_component("cTerrain")
local vegetation_root = e_terrain.find_child("vegetation")

terrain_scatter(terrain, vegetation_root, vec4(190, 190, 20, 20), 0.2, e_grasses, 0.03, 0.8)
terrain_scatter(terrain, vegetation_root, vec4(190, 190, 20, 20), 2.5, e_trees, 0.1, 0.8)
--[[
local e_plants = {}
table.insert(e_plants, create_entity("D:\\assets\\vegetation\\plant1.prefab"))

scatter(vec4(0.0, 0.0, 400.0, 400.0), 0.2, e_grasses, 0.05, 2.5)
scatter(vec4(0.0, 0.0, 400.0, 400.0), 0.5, e_plants, 0.0025, 1.0)
]]

local character_panel = scene.find_child("character_panel")
local hp_text = character_panel.find_child("hp_text").find_component("cText")
obj_root.add_event(function()
	hp_text.set_text("HP: "..main_player.HP.."/"..main_player.HP_MAX.."  +"..main_player.HP_RECOVER)
end, 0.0)

--[[
for i=1, 300, 1 do
	local e = create_entity("human")
	e.set_name("enemy_"..tostring(math.floor(math.random() * 10000)))
	e.find_component("cNode").set_pos(vec3(math.random() * 400, 200, math.random() * 400))
	--e.find_component("cNode").set_pos(vec3(190 + math.random() * 20, 200, 190 + math.random() * 20))
    make_npc(make_character(e, 2, STAT))
	obj_root.add_child(e)
end
]]

obj_root.add_event(function()
	for n, char in pairs(characters[2]) do
		if obj_root_n.is_any_within_circle(char.pos.to_flat(), 50, 1) then
			char.awake()
		else
			char.sleep()
		end
	end
end, 1.0)
