local player = {
	character = __character
}

local scene_receiver = scene.find_component("cReceiver")

scene_receiver.add_mouse_right_down_listener(function(mpos)
	local o = camera.node.get_global_pos()
	local d = normalize_3(camera.camera.screen_to_world(mpos) - o)
	local p = s_physics.raycast(o, d)
	player.character.state = "moving"
	player.character.target_pos = vec2(p.x, p.z)
end)

__player = player
