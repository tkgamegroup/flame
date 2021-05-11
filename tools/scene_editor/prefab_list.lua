prefab_list = entity
              
local prefabs = flame_get_directory_files("prefabs")
for i=1, #prefabs, 1 do
    local e = create_entity("prefabs/list_item")
    e.find_component("cText").set_text(prefabs[i]:gsub("%.prefab", ""))
    prefab_list.add_child(e)
end
