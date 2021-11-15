s_physics = world.find_system("sPhysics")
s_dispatcher = world.find_system("sDispatcher")
s_renderer = world.find_system("sRenderer")

root = world.get_root()
scene = root

local udt_entity = find_udt("Entity")

function make_entity(p)
	local e = {p = p}
	make_obj(e, udt_entity)
	return e
end

function create_entity(src)
	local e = udt_entity.static_functions.create()
	local ok = true
	if src ~= "" then
		ok = e.load(src)
	end
	return e, ok
end
