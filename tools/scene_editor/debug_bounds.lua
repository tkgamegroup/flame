local element = entity.find_component("cElement")
local drawer = find_udt("ElementScriptDrawer").static_functions.create()
drawer.set_callback(get_callback_slot(function(l, r)
	local draw_bounds = nil
	draw_bounds = function(e)
		local node = e.find_component("cNode")
		if node.p then
			local m = node.get_bounds()
			local bounds = AABB(vec3(m.a1, m.a2, m.a3), vec3(m.b1, m.b2, m.b3))
			if bounds.a.x ~= bounds.b.x or bounds.a.y ~= bounds.b.y or bounds.a.z ~= bounds.b.z then
				local col = flame_malloc(4)
				flame_set_c4(col, 0, vec4(255, 100, 100, 255))
				local points = flame_malloc(8 * 12)
				bounds.get_points(points)
				local lines = flame_malloc(12 * 32)
				hexahedron_draw_lines(lines, points, col)
				r.draw_lines(12, lines)
				flame_free(lines)
				flame_free(points)
				flame_free(col)
			end
		end
		
		local n = e.get_children_count()
		for i=0, n-1, 1 do
			draw_bounds(e.get_child(i))
		end
	end

	if prefab.p then
		draw_bounds(prefab)
	end
end))
element.add_drawer(drawer)
