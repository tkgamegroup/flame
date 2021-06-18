camera = {
	node = entity.find_component("cNode"),
	length = 30,
	yaw = 0,
	pitch = -60
}

camera.update_pos = function()
	camera.node.set_pos(character.node.get_global_pos() + camera.node.get_local_dir(2) * camera.length)
end

camera.node.set_euler(vec3(camera.yaw, camera.pitch, 0))

entity.add_event(function()
	camera.update_pos()
end, 0)
