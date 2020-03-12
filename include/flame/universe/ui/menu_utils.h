#pragma once

namespace flame
{
	namespace ui
	{
		inline bool is_menu_can_open(cMenu* menu, KeyStateFlags action, MouseKey key)
		{
			if (menu->mode == cMenu::ModeContext)
			{
				if ((is_mouse_down(action, key, true) && key == Mouse_Right))
					return true;
			}
			else
			{
				if ((is_mouse_down(action, key, true) && key == Mouse_Left))
					return true;
				else if (is_mouse_move(action, key))
					return get_top_layer(menu->root, true, "menu");
			}
			return false;
		}
	}
}
