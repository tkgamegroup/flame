local FIXED_DIST = 17

camera = {
	node = entity.find_component("cNode"),
	camera = entity.find_component("cCamera"),
	length = FIXED_DIST,
	yaw = 0,
	pitch = -60
}

camera.node.set_euler(vec3(camera.yaw, camera.pitch, 0))

entity.add_event(function()
	if not main_player.dead then
		camera.node.set_pos(main_player.pos + camera.node.get_local_dir(2) * camera.length)
	end
end, 0)
