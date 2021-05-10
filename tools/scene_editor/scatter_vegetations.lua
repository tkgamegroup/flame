local menu_item = entity.find_driver("dMenuItem")
entity.find_component("cReceiver").add_mouse_left_down_listener(function()
    local e_node_root = scene.find_child("node_root")
    if not e_node_root.p then return end
    local e_terrain = e_node_root.find_child("terrain")
    if not e_terrain.p then return end
    local terrain = e_terrain.find_component("cTerrain")
              
    local blocks = terrain.get_blocks()
    local scale = terrain.get_scale()
    local num = 100
    local ptr_uvs = flame_malloc(num * 16) -- 16 is the size of vec2
    local ptr_samples = flame_malloc(num * 32) -- 32 is the size of vec4
              
    local tag = find_enum("TypeTag")["Data"]
    local basic = find_enum("BasicType")["FloatingType"]

    for i=0,num-1,1 do
        local uv = vec2(math.random(), math.random())
        flame_set(ptr_uvs, i * 16, tag, basic, 2, 1, uv)
    end
              
    local height_texture = terrain.get_height_texture()
    height_texture.get_ptr_samples(num, ptr_uvs, ptr_samples)
              
    local e_grass_root = create_entity("prefabs/node")
    e_grass_root.set_name("grass_root")
    for i=0,num-1,1 do
        local uv = flame_get(ptr_uvs, i * 16, tag, basic, 2, 1)
        local sample = flame_get(ptr_samples, i * 32, tag, basic, 4, 1)
        local e = create_entity("D:\\grass\\01_d.prefab")
        e.find_component("cNode").set_pos(vec3(uv.x * blocks.x * scale.x, sample.x * scale.y, uv.y * blocks.y * scale.z))
        e_grass_root.add_child(e)
    end
    e_node_root.add_child(e_grass_root)
              
    flame_free(ptr_uvs)
    flame_free(ptr_samples)
end)
