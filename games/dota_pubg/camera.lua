camera = {
	node = entity.find_component("cNode"),
	length = 30,
	yaw = 0,
	pitch = -60
}

camera.node.set_euler(vec3(camera.yaw, camera.pitch, 0))
camera.node.set_pos(camera.node.get_local_dir(2) * camera.length)
