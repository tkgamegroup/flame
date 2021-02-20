world = root.get_world()
s_physics = world.find_system("sPhysics")
s_dispatcher = world.find_system("sDispatcher")
s_renderer = world.find_system("sRenderer")
canvas = s_renderer.get_canvas()

function create_entity(src)
	local e = find_udt("Entity").static_functions.create()
	e.load(src)
	return e
end
