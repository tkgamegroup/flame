s_physics = world.find_system("sPhysics")
s_dispatcher = world.find_system("sDispatcher")
s_renderer = world.find_system("sRenderer")
canvas = s_renderer.get_canvas()

element_root = world.get_element_root();
node_root = world.get_node_root();

function create_entity(src)
	local e = find_udt("Entity").static_functions.create()
	local ok = e.load(src)
	return e, ok
end
