local element = entity.find_component("cElement")
local drawer = find_udt("ElementScriptDrawer").static_functions.create()
drawer.set_callback(get_callback_slot(function(l, r)
	
end))
element.add_drawer(drawer)
