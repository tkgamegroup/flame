local terrain = entity
local node = terrain.find_component("cNode")
local pos = node.get_pos();
local scale = node.get_scale()

local c_terrain = terrain.find_component("cTerrain")
local blocks = c_terrain.get_blocks()
local height_texture = c_terrain.get_height_texture()
  
local e_vegetation_root = create_entity("prefabs/node")
local node = e_vegetation_root.find_component("cNode")
node.set_pos(pos + vec3(scale.x * 0.5, 0.0, scale.z * 0.5))
node.set_octree_length(math.max(math.max(scale.x, scale.z), scale.y))
node.set_is_octree(true)
terrain.get_parent().add_child(e_vegetation_root, terrain.get_index() + 1)

function scatter(range, density, prefabs, probability, model_scale)
    local cx = math.floor(range.z / density) + 1
    local cy = math.floor(range.w / density) + 1
    local num = cx * cy
    local ptr_samples = malloc_vec4(num)

    height_texture.get_samples(vec4(range.x / scale.x, range.y / scale.z, 
        density / scale.x, density / scale.z), vec2(cx, cy), ptr_samples, 0, 0)

    local n_prefabs = #prefabs
    local off = vec3(scale.x, 0.0, scale.z) * 0.5;
    local i = 0 
    for y = 0, cy - 1, 1 do
	    for x = 0, cx - 1, 1 do
            if math.random() < probability then
                local sample = get_vec4(ptr_samples, i)
                local e = prefabs[math.floor(math.random() * n_prefabs) + 1].copy()
                local node = e.find_component("cNode")
                node.set_pos(vec3(range.x + (x + math.random() - 0.5) * density, sample.x * scale.y, range.y + (y + math.random() - 0.5) * density) - off)
                node.set_euler(vec3(math.random() * 360, 0, 0))
                node.set_scale(vec3((0.8 + math.random() * 0.4) * model_scale))
                e_vegetation_root.add_child(e)
            end
            i = i + 1
	    end
    end

    flame_free(ptr_samples)
end

local e_grasses = {}
e_grasses[1] = create_entity("D:\\assets\\vegetation\\grass1.prefab")
e_grasses[2] = create_entity("D:\\assets\\vegetation\\grass2.prefab")
e_grasses[3] = create_entity("D:\\assets\\vegetation\\grass3.prefab")
e_grasses[4] = create_entity("D:\\assets\\vegetation\\grass4.prefab")
e_grasses[5] = create_entity("D:\\assets\\vegetation\\grass5.prefab")
e_grasses[6] = create_entity("D:\\assets\\vegetation\\grass6.prefab")

local e_plants = {}
e_plants[1] = create_entity("D:\\assets\\vegetation\\plant1.prefab")
e_plants[2] = create_entity("D:\\assets\\vegetation\\plant2.prefab")

scatter(vec4(0.0, 0.0, 400.0, 400.0), 0.2, e_grasses, 0.05, 2.5)
scatter(vec4(0.0, 0.0, 400.0, 400.0), 0.5, e_plants, 0.0025, 1.0)
