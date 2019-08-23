#pragma once

#include <flame/universe/component.h>

namespace flame
{
	struct cEventReceiver;

	struct cSubMenuButton : Component // popup a menu when clik
	{
		cElement* element;
		cEventReceiver* event_receiver;

		Entity* menu;

		bool opened;

		cSubMenuButton() :
			Component("SubMenuButton")
		{
		}

		FLAME_UNIVERSE_EXPORTS virtual ~cSubMenuButton() override;

		FLAME_UNIVERSE_EXPORTS virtual void on_add_to_parent() override;

		FLAME_UNIVERSE_EXPORTS virtual void update() override;

		FLAME_UNIVERSE_EXPORTS void open();
		FLAME_UNIVERSE_EXPORTS void close();

		FLAME_UNIVERSE_EXPORTS static cSubMenuButton* create();
	};

	FLAME_UNIVERSE_EXPORTS void popup_menu(Entity* menu, Entity* root, const Vec2f& pos);
}
