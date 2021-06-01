local terrain = entity
local node = terrain.find_component("cNode")
local pos = node.get_pos();
local scale = node.get_scale()

local c_terrain = terrain.find_component("cTerrain")
local blocks = c_terrain.get_blocks()
local height_texture = c_terrain.get_height_texture()

local range = vec4(200.0, 200.0, 20.0, 20.0)
local density = 0.4
  
local e_vegetation_root = create_entity("prefabs/node")
local node = e_vegetation_root.find_component("cNode")
node.set_pos(pos + vec3(scale.x * 0.5, 0.0, scale.z * 0.5))
node.set_octree_length(math.max(math.max(scale.x, scale.z), scale.y))
node.set_is_octree(true)
terrain.get_parent().add_child(e_vegetation_root, terrain.get_index() + 1)

local cx = math.floor(range.z / density)
local cy = math.floor(range.w / density)
local num = cx * cy
local ptr_uvs = malloc_vec2(num)
local ptr_samples = malloc_vec4(num)

local i = 0 
for x = 0, cx - 1, 1 do
	for y = 0, cy - 1, 1 do
        local uv = vec2((range.x + x * density) / scale.x, (range.y + y * density) / scale.z)
        set_vec2(ptr_uvs, i, uv)
        i = i + 1
	end
end

height_texture.get_samples(num, ptr_uvs, ptr_samples)

local off = vec3(scale.x, 0.0, scale.z) * 0.5;
local i = 0 
for x = 0, cx - 1, 1 do
	for y = 0, cy - 1, 1 do
        local sample = get_vec4(ptr_samples, i)
        local e = create_entity("D:\\assets\\grass_new\\grass.prefab")
        local node = e.find_component("cNode")
        node.set_pos(vec3(range.x + (x + math.random() - 0.5) * density, sample.x * scale.y, range.y + (y + math.random() - 0.5) * density) - off)
        node.set_euler(vec3(math.random() * 360, 0, 0))
        node.set_scale(vec3(0.8 + math.random() * 0.4))
        e_vegetation_root.add_child(e)
        i = i + 1
	end
end

flame_free(ptr_uvs)
flame_free(ptr_samples)
