character = {
	speed = 0.1,
	node = entity.find_component("cNode"),
	animation = entity.find_component("cAnimation"),
	controller = entity.find_component("cController"),
	yaw = 0,
	dir = vec3(0, 0, 1),
	state = "moving",
	move_pos = vec2(0)
}

character.update_dir = function()
	character.node.set_euler(vec3(character.yaw, 0, 0))
	character.dir = character.node.get_local_dir(2)
end

local h_src = flame_hash("src")
character.animation.entity.add_component_data_listener(function(h)
	if h == h_src then
		if character.animation.get_playing() == -1 then
			character.animation.play(0)
			character.animation.set_loop(true)
		end
	end
end, character.animation)

character.animation.play(0)
character.animation.set_loop(true)

entity.add_event(function()
	if character.state == "moving" then
		local p = character.node.get_global_pos()
		local v = character.move_pos - vec2(p.x, p.z)
		local l = length_2(v)
		local d = vec2(0)
		if l > 0 then
			d = vec2(v.x / l, v.y / l)
			character.yaw = math.atan(d.y, d.x) / 3.14 * 180
			character.update_dir()
		end
		if l <= character.speed then
			character.state = "idle"
			character.animation.play(0)
			character.animation.set_loop(true)
			return
		end
		character.animation.play(1)
		character.animation.set_loop(true)
		character.controller.move(vec3(d.x * character.speed, 0, d.y * character.speed))
	end
end, 0)
