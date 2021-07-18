local config = load_ini("config.ini")
local locations = config["location"]
last_open = locations["last_open"]
last_save = locations["last_save"]
      
function on_menu(ev)
    entity.find_component("cReceiver").add_mouse_left_down_listener(ev)
end

function on_single_check_menu(f)
    local dmi = entity.find_driver("dMenuItem")
    entity.find_component("cReceiver").add_mouse_left_down_listener(function()
        dmi.set_single_checked()
        f(checked)
    end)
end

function on_multi_check_menu(f)
    local dmi = entity.find_driver("dMenuItem")
    entity.find_component("cReceiver").add_mouse_left_down_listener(function()
        local checked = dmi.get_checked()
        checked = not checked
        dmi.set_checked(checked)
        f(checked)
    end)
end
