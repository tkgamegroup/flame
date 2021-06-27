octnode = entity.find_component("cNode")

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
e.find_component("cNode").set_pos(vec3(200, 80, 200))
main_player = make_character(e, 1, PLAYER_STAT)
make_player(main_player)
entity.add_child(e)

for i=1, 10, 1 do
	local e = create_entity("human")
	e.set_name("enemy_"..tostring(math.random() * 10000))
	e.find_component("cNode").set_pos(vec3(190 + math.random() * 20, 80, 190 + math.random() * 20))
	--e.find_component("cNode").set_pos(vec3(math.random() * 400, 200, math.random() * 400))
    make_npc(make_character(e, 2, STAT))
	entity.add_child(e)
end
