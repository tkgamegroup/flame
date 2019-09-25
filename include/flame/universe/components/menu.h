#pragma once

#include <flame/universe/component.h>

namespace flame
{
	namespace graphics
	{
		struct FontAtlas;
	}

	struct cElement;
	struct cEventReceiver;

	struct cMenuButton : Component // popup a menu when clik
	{
		cElement* element;
		cEventReceiver* event_receiver;

		Entity* root;
		Entity* menu;
		bool move_to_open;
		Side$ popup_side;
		bool topmost_penetrable;

		bool opened;

		cMenuButton() :
			Component("MenuButton")
		{
		}

		bool can_open(KeyState action, MouseKey key)
		{
			if ((is_mouse_down(action, key, true) && key == Mouse_Left))
				return true;
			else if (move_to_open && is_mouse_move(action, key))
			{
				auto t = get_topmost(root);
				if (t && t->name_hash() == cH("topmost"))
					return true;
			}
			return false;
		}

		FLAME_UNIVERSE_EXPORTS virtual void start() override;
		FLAME_UNIVERSE_EXPORTS virtual void update() override;

		FLAME_UNIVERSE_EXPORTS void open();
		FLAME_UNIVERSE_EXPORTS void close();

		FLAME_UNIVERSE_EXPORTS static cMenuButton* create();
	};

	struct cMenu : Component
	{
		cMenuButton* popuped_by;

		cMenu() :
			Component("Menu")
		{
		}

		FLAME_UNIVERSE_EXPORTS virtual void update() override;

		FLAME_UNIVERSE_EXPORTS static cMenu* create();
	};

	FLAME_UNIVERSE_EXPORTS void close_menu(Entity* menu);
	FLAME_UNIVERSE_EXPORTS void popup_menu(Entity* menu, Entity* root, const Vec2f& pos);

	FLAME_UNIVERSE_EXPORTS Entity* create_standard_menu();
	FLAME_UNIVERSE_EXPORTS Entity* create_standard_menu_item(graphics::FontAtlas* font_atlas, float sdf_scale, const std::wstring& text);
	FLAME_UNIVERSE_EXPORTS Entity* create_standard_menu_button(graphics::FontAtlas* font_atlas, float sdf_scale, const std::wstring& text, Entity* root, Entity* menu, bool move_to_open, Side$ popup_side, bool topmost_penetrable, bool width_greedy, bool background_transparent, const wchar_t* arrow_text);
	FLAME_UNIVERSE_EXPORTS Entity* create_standard_menubar();
}
