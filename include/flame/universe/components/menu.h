#pragma once

#include <flame/universe/component.h>

namespace flame
{
	struct cEventReceiver;

	struct cMenuButton : Component // popup a menu when clik
	{
		cElement* element;
		cEventReceiver* event_receiver;

		Entity* root;
		Entity* menu;
		bool move_to_open;
		Side popup_side;
		bool topmost_penetrable;

		bool opened;

		cMenuButton() :
			Component("MenuButton")
		{
		}

		FLAME_UNIVERSE_EXPORTS virtual ~cMenuButton() override;

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

		FLAME_UNIVERSE_EXPORTS virtual ~cMenu() override;

		FLAME_UNIVERSE_EXPORTS virtual void update() override;

		FLAME_UNIVERSE_EXPORTS static cMenu* create();
	};

	FLAME_UNIVERSE_EXPORTS void close_menu(Entity* menu);
	FLAME_UNIVERSE_EXPORTS void popup_menu(Entity* menu, Entity* root, const Vec2f& pos);
}
