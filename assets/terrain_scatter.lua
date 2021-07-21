function terrain_spawn(extent, height_tex, e_dst, pos, prefab, y_off)
    local e = prefab.copy()
    local node = e.find_component("cNode")
    node.set_pos(vec3(pos.x, 
        height_tex.linear_sample(vec2(pos.x / extent.x, pos.y / extent.x), 0, 0).x * extent.y + y_off, 
    pos.y))
    e_dst.add_child(e)
end

function terrain_scatter(extent, height_tex, normal_tex, e_dst, range, density, prefabs, probability, height_constraint, normal_constraint, scale)
    local cx = math.floor(range.z / density) + 1
    local cy = math.floor(range.w / density) + 1

    local n_prefabs = #prefabs
    for y = 0, cy - 1, 1 do
	    for x = 0, cx - 1, 1 do
            local uv = vec2((range.x + x * density) / extent.x, (range.y + y * density) / extent.x)
            local normal = vec3(normal_tex.linear_sample(uv, 0, 0)) * 2 - vec3(1)
            if math.random() < probability and dot_3(normal, vec3(0, 1, 0)) > normal_constraint then
                local height = height_tex.linear_sample(uv, 0, 0).x * extent.y
                if height > height_constraint then
                    local e = prefabs[math.floor(math.random() * n_prefabs) + 1].copy()
                    local node = e.find_component("cNode")
                    node.set_pos(vec3(
                        range.x + (x + math.random() - 0.5) * density, 
                        height, 
                        range.y + (y + math.random() - 0.5) * density
                    ))
                    node.set_euler(vec3(math.random() * 360, 0, 0))
                    node.set_scale(vec3((0.8 + math.random() * 0.4) * scale))
                    e_dst.add_child(e)
                end
            end
	    end
    end
end
