camera = {
	node = entity.find_component("cNode"),
	camera = entity.find_component("cCamera"),
	length = 17,
	yaw = 0,
	pitch = -60
}

camera.update_pos = function()
	if main_player.character then
		camera.node.set_pos(main_player.character.pos + camera.node.get_local_dir(2) * camera.length)
	end
end

camera.node.set_euler(vec3(camera.yaw, camera.pitch, 0))

entity.add_event(function()
	camera.update_pos()
end, 0)
