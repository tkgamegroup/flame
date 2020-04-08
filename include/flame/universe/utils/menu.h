#pragma once

#include <flame/universe/components/menu.h>
#include <flame/universe/utils/event.h>

namespace flame
{
	namespace utils
	{
		inline bool is_menu_can_open(cMenu* menu, KeyStateFlags action, MouseKey key)
		{
			if (menu->mode == cMenu::ModeContext)
			{
				if (!is_focusing_and_not_normal(menu->event_receiver) && is_mouse_down(action, key, true) && key == Mouse_Right)
					return true;
			}
			else
			{
				if (is_mouse_down(action, key, true) && key == Mouse_Left)
					return true;
				else if (is_mouse_move(action, key))
					return menu->root->last_child(FLAME_CHASH("layer_menu"));
			}
			return false;
		}
	}
}
