local terrain = entity
local node = terrain.find_component("cNode")
local pos = node.get_pos();
local scale = node.get_scale()

local c_terrain = terrain.find_component("cTerrain")
local blocks = c_terrain.get_blocks()
local height_texture = c_terrain.get_height_texture()

local density = 1.5

local lod = 5
              
local e_vegetation_root = create_entity("prefabs/node")
local node = e_vegetation_root.find_component("cNode")
node.set_pos(pos + vec3(scale.x * 0.5, 0.0, scale.z * 0.5))
node.set_octree_length(math.max(math.max(scale.x, scale.z), scale.y))
node.set_is_octree(true)
terrain.get_parent().add_child(e_vegetation_root, terrain.get_index() + 1)

local blocks = {}
for i = 1, 9, 1 do
    blocks[i] = {
        x = -1,
        y = -1,
        e = nil
    }
end

terrain.add_event(function()
    local camera_pos = camera.node.get_global_pos()
    for i = 1, 9, 1 do
        local x = math.floor(camera_pos.x / lod)
        local y = math.floor(camera_pos.z / lod)
        if i == 1 then
            x = x - 1
            y = y - 1
        elseif i == 2 then
            y = y - 1
        elseif i == 3 then
            x = x + 1
            y = y - 1
        elseif i == 4 then
            x = x - 1
        elseif i == 6 then
            x = x + 1
        elseif i == 7 then
            x = x - 1
            y = y + 1
        elseif i == 8 then
            y = y + 1
        elseif i == 9 then
            x = x + 1
            y = y + 1
        end

        local block = blocks[i]
        if block.x ~= x or block.y ~= y then
            block.x = x
            block.y = y
            if block.e then
                block.e.get_parent().remove_child(block.e)
            end

            local len = math.floor(lod / density)
            local num = len * len
            local ptr_uvs = malloc_vec2(num)
            local ptr_samples = malloc_vec4(num)

            x = x * 5
            y = y * 5
            local i = 0
            for xx = 0, len - 1, 1 do
	            for yy = 0, len - 1, 1 do
                    local uv = vec2((x + xx * density) / scale.x, (y + yy * density) / scale.z)
                    set_vec2(ptr_uvs, i, uv)
                    i = i + 1
	            end
            end

            height_texture.get_samples(num, ptr_uvs, ptr_samples)

            block.e = create_entity("prefabs/node")

            local off = vec2(scale.x, scale.z) * 0.5;
            local i = 0
            for xx = 0, len - 1, 1 do
	            for yy = 0, len - 1, 1 do
                    local sample = get_vec4(ptr_samples, i)
                    local e = create_entity("D:\\assets\\grass\\02_d.prefab")
                    local node = e.find_component("cNode")
                    node.set_pos(vec3(x + xx * density - off.x, sample.x * scale.y, y + yy * density - off.y))
                    node.set_euler(vec3(math.random() * 360, 0, 0))
                    node.set_scale(vec3(0.8 + math.random() * 0.4))
                    block.e.add_child(e)
                    i = i + 1
	            end
            end

            e_vegetation_root.add_child(block.e)

            flame_free(ptr_uvs)
            flame_free(ptr_samples)
        end
    end
end, 1.0)
