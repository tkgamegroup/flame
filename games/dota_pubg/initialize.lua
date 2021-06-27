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

for i=1, 10, 1 do
	local e = create_entity("human")
	e.set_name("enemy_"..tostring(math.random() * 10000))
	e.find_component("cNode").set_pos(vec3(190 + math.random() * 20, 80, 190 + math.random() * 20))
    make_enemy(make_character(e))
	entity.add_child(e)
end
