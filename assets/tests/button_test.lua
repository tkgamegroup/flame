local receiver = entity:find_component("cReceiver")
receiver:add_mouse_click_listener(function()
	print("Hello World")
end)
