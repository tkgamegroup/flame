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
		Side popup_side;
		bool move_to_open;
		bool layer_penetrable;

		bool opened;

		cMenuButton() :
			Component("cMenuButton")
		{
		}

		FLAME_UNIVERSE_EXPORTS bool can_open(KeyState action, MouseKey key);

		FLAME_UNIVERSE_EXPORTS void open();
		FLAME_UNIVERSE_EXPORTS void close();
		
		FLAME_UNIVERSE_EXPORTS static cMenuButton* create();
	};

	struct cMenu : Component
	{
		cMenuButton* button;

		cMenu() :
			Component("cMenu")
		{
		}

		FLAME_UNIVERSE_EXPORTS static cMenu* create();
	};

	FLAME_UNIVERSE_EXPORTS void close_menu(Entity* menu);
	FLAME_UNIVERSE_EXPORTS void popup_menu(Entity* menu, Entity* root, const Vec2f& pos);
}
