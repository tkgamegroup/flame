SceneEditorWindow::SceneEditorWindow() :
	GraphicsWindow(&app, true, true, "Scene Editor", uvec2(300, 200), WindowFrame | WindowResizable, nullptr, true)
{
	c_receiver->key_listeners.add([](Capture& c, KeyStateFlags action, int value) {
		if (is_key_down(action))
		{
			auto ed = c.current<cReceiver>()->dispatcher;
			switch (value)
			{
			case Keyboard_S:
				if (ed->key_states[Keyboard_Ctrl] & KeyStateDown)
					scene_editor.save();
				break;
			}
		}
	}, Capture());
}
