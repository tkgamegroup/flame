function terrain_spawn(extent, height_tex, e_dst, poses, prefab, y_off)
    local num = #poses
    
    local uvs = flame_malloc(num * 8)
    local heights = flame_malloc(num * 16)

    local one_over_ext = 1.0 / extent.x
    for i=0, num-1, 1 do
        set_vec2(uvs, i, poses[i + 1] * one_over_ext)
    end
    height_tex.arbitrarily_sample(num, uvs, heights, 0, 0)
    
    for i=0, num-1, 1 do
        local e = prefab.copy()
        local node = e.find_component("cNode")
        node.set_pos(vec3(
            poses[i + 1].x, 
            get_vec4(heights, i).x * extent.y + y_off, 
            poses[i + 1].y
        ))
        e_dst.add_child(e)
    end 
    
    flame_free(uvs)
    flame_free(heights)
end

function terrain_scatter(extent, height_tex, normal_tex, e_dst, range, density, prefabs, probability, scale)
    local cx = math.floor(range.z / density) + 1
    local cy = math.floor(range.w / density) + 1

    local num = cx * cy
    local heights = flame_malloc(num * 16)
    local normals = flame_malloc(num * 16)

    local off_step = vec4(
        range.x / extent.x, range.y / extent.x, 
        density / extent.x, density / extent.x)
    height_tex.grid_sample(off_step, vec2(cx, cy), heights, 0, 0)
    normal_tex.grid_sample(off_step, vec2(cx, cy), normals, 0, 0)

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
