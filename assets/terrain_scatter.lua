function terrain_spawn(extent, height_tex, e_dst, pos, prefab, y_off)
    local e = prefab.copy()
    local node = e.find_component("cNode")
    node.set_pos(vec3(pos.x, 
        height_tex.linear_sample(vec2(pos.x / extent.x, pos.y / extent.x), 0, 0).x * extent.y + y_off, 
    pos.y))
    e_dst.add_child(e)
end

function terrain_scatter(extent, height_tex, normal_tex, e_dst, range, density, prefabs, probability, height_constraint, normal_constraint, rotation_range, scale_range)
    local cx = math.floor(range.z / density) + 1
    local cy = math.floor(range.w / density) + 1

    local n_prefabs = #prefabs
    for y = 0, cy - 1, 1 do
	    for x = 0, cx - 1, 1 do
            local uv = vec2((range.x + x * density) / extent.x, (range.y + y * density) / extent.x)
            if math.random() < probability then
                local height = height_tex.linear_sample(uv, 0, 0).x * extent.y
                if height > height_constraint then
                    local normal = vec3(normal_tex.linear_sample(uv, 0, 0)) * 2 - vec3(1)
                    local ndotup = dot_3(normal, vec3(0, 1, 0))
                    if ndotup > normal_constraint then
                        local p = math.random()
                        for i=1, n_prefabs, 1 do
                            if p < prefabs[i].p then
                                local e = prefabs[i].e.copy()
                                local node = e.find_component("cNode")
                                node.set_pos(vec3(
                                    range.x + (x + math.random() - 0.5) * density, 
                                    height, 
                                    range.y + (y + math.random() - 0.5) * density
                                ))
                                node.set_euler(vec3(rand2(rotation_range.x, rotation_range.y), 0, 0))
                                node.set_scale(vec3(rand2(scale_range.x, scale_range.y)))
                                e_dst.add_child(e)
                                break
                            else
                                p = p - prefabs[i].p
                            end
                        end
                    end
                end
            end
	    end
    end
end
