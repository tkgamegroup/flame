for i=1, 10, 1 do
	local e = create_entity("human")
	e.set_name("enemy_"..tostring(math.random() * 10000))
	e.find_component("cNode").set_pos(vec3(190 + math.random() * 20, 80, 190 + math.random() * 20))
    make_enemy(make_character(e))
	entity.add_child(e)
end
