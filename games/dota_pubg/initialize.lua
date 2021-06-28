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

local e = create_entity("human")
e.set_name("player")
e.find_component("cNode").set_pos(vec3(200, 65, 200))
main_player = make_character(e, 1, PLAYER_STAT)
make_player(main_player)
main_player.awake()
obj_root.add_child(e)

local character_panel = scene.find_child("character_panel")
local hp_text = character_panel.find_child("hp_text").find_component("cText")
obj_root.add_event(function()
	hp_text.set_text("HP: "..main_player.HP.."/"..main_player.HP_MAX)
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
