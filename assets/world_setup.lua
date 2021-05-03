s_physics = world.find_system("sPhysics")
s_dispatcher = world.find_system("sDispatcher")
s_renderer = world.find_system("sRenderer")

root = world.get_root();

function create_entity(src)
	local e = find_udt("Entity").static_functions.create()
	local ok = e.load(src)
	return e, ok
end
