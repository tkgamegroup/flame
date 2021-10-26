local config = load_ini("config.ini")
local locations = config["location"]
last_open = locations["last_open"]
last_save = locations["last_save"]
      
function on_menu(ev)
    entity.find_component("cReceiver").add_mouse_left_down_listener(ev)
end

function on_radio_menu(f)
    local dmi = entity.find_component("cMenuItem")
    entity.find_component("cReceiver").add_mouse_left_down_listener(function()
        dmi.set_radio_checked()
        f(true)
    end)
end

function on_check_menu(o)
    local dmi = entity.find_component("cMenuItem")
    if type(o) == "table" then
        o.dmi = dmi
    end
    entity.find_component("cReceiver").add_mouse_left_down_listener(function()
        local checked = dmi.get_checked()
        checked = not checked
        dmi.set_checked(checked)
        if type(o) == "table" then
            o.f(checked)
        else
            o(checked)
        end
    end)
end
