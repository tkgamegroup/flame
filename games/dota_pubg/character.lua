local character = {
	entity = entity,
	node = entity.find_component("cNode"),
	animation = entity.find_component("cAnimation"),
	controller = entity.find_component("cController"),
	yaw = 0,
	dir = vec3(0, 0, 1),
	radius = 0.28,
	speed = 0.057,
	state = "idle",
	target_pos = vec2(0),
	target = nil
}

character.update_dir = function()
	character.node.set_euler(vec3(character.yaw, 0, 0))
	character.dir = character.node.get_local_dir(2)
end

character.animation.add_callback(function(frame)
	if frame == -1 then
		if character.state == "attacking" then
			character.state = "attack_target"
		end
	end
end, character.animation)

character.animation.play(0)
character.animation.set_loop(true)

character.entity.add_event(function()
	if character.state == "moving" then
		local p = character.node.get_global_pos()
		local v = character.target_pos - vec2(p.x, p.z)
		local l, d = length_and_dir_2(v)
		if d then -- aim
			character.yaw = math.atan(d.x, d.y) / 3.14 * 180
			character.update_dir()
		end
		if l <= character.speed then
			character.state = "idle"
			character.animation.play(0)
			character.animation.set_loop(true)
		else
			character.animation.play(1)
			character.animation.set_loop(true)
			character.controller.move(vec3(d.x * character.speed, 0, d.y * character.speed))
		end
	elseif character.state == "attack_target" then
		if not character.target then
			character.state = "idle"
			character.animation.play(0)
			character.animation.set_loop(true)
		end
		local p = character.node.get_global_pos()
		local tp = character.target.node.get_global_pos()
		local v = vec2(tp.x, tp.z) - vec2(p.x, p.z)
		local l, d = length_and_dir_2(v)
		if d then -- aim
			character.yaw = math.atan(d.x, d.y) / 3.14 * 180
			character.update_dir()
		end
		if l <= character.radius + character.target.radius + 1 then
			character.animation.play(2)
			character.animation.set_loop(false)
			character.state = "attacking"
		else
			character.animation.play(1)
			character.animation.set_loop(true)
			character.controller.move(vec3(d.x * character.speed, 0, d.y * character.speed))
		end
	end
end, 0)

__character = character
