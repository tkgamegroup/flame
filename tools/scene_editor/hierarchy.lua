hierarchy = entity

local tree = hierarchy.find_driver("dTree")
local hash_selected = flame_hash("selected")
hierarchy.add_driver_data_listener(function(h)
    if h == hash_selected then
        local s = tree.get_selected()
        if s.p then
            local o = { p=s.get_userdata() }
            make_obj(o, "flame::Entity")
            select(o, true)
        else
            select({ p=nil }, true)
        end
    end
end, tree)
              
function update_hierachy()
    if hierarchy.get_children_count() > 0 then
        hierarchy.remove_child(hierarchy.get_child(0))
    end
    if prefab.p then
        function create_nodes(src, dst)
            local count = src.get_children_count()
            if count > 0 then
                local e = create_entity("prefabs/tree_node")
                e.find_driver("dTreeNode").set_title("["..src.get_name().."]")
                e.set_userdata(src)
                src.set_userdata(e)
                dst.add_child(e)
                for i=0, count-1, 1 do
                    local c = src.get_child(i)
                    create_nodes(c, e)
                end
            else
                local e = create_entity("prefabs/tree_leaf")
                e.find_driver("dTreeLeaf").set_title("["..src.get_name().."]")
                e.set_userdata(src)
                src.set_userdata(e)
                dst.add_child(e)
            end
        end
        create_nodes(prefab, hierarchy)
    end
end
