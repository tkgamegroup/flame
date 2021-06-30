function terrain_scatter(terrain, e_dst, range, density, prefabs, probability, scale)
    local cx = math.floor(range.z / density) + 1
    local cy = math.floor(range.w / density) + 1
    local extent = terrain.get_extent()

    local num = cx * cy
    local heights = flame_malloc(num * 16)
    local normals = flame_malloc(num * 16)

    local off_step = vec4(
        range.x / extent.x, range.y / extent.x, 
        density / extent.x, density / extent.x)
    terrain.get_height_texture().get_samples(off_step, vec2(cx, cy), heights, 0, 0)
    terrain.get_normal_texture().get_samples(off_step, vec2(cx, cy), normals, 0, 0)

    local n_prefabs = #prefabs
    local i = 0 
    for y = 0, cy - 1, 1 do
	    for x = 0, cx - 1, 1 do
            local normal4 = get_vec4(normals, i)
            local normal = vec3(normal4.x, normal4.y, normal4.z) * 2 - vec3(1)
            if math.random() < probability and dot_3(normal, vec3(0, 1, 0)) > 0.9 then
                local e = prefabs[math.floor(math.random() * n_prefabs) + 1].copy()
                local node = e.find_component("cNode")
                node.set_pos(vec3(
                    range.x + (x + math.random() - 0.5) * density, 
                    get_vec4(heights, i).x * extent.y, 
                    range.y + (y + math.random() - 0.5) * density
                ))
                node.set_euler(vec3(math.random() * 360, 0, 0))
                node.set_scale(vec3((0.8 + math.random() * 0.4) * scale))
                e_dst.add_child(e)
            end
            i = i + 1
	    end
    end

    flame_free(heights)
    flame_free(normals)
end
