#pragma once

#include <flame/universe/component.h>

namespace flame
{
	struct cEventReceiver;

	struct cSubMenu : Component
	{
		cElement* element;
		cEventReceiver* event_receiver;

		Entity* items;

		bool opened;

		cSubMenu() :
			Component("SubMenu")
		{
		}

		FLAME_UNIVERSE_EXPORTS virtual ~cSubMenu() override;

		FLAME_UNIVERSE_EXPORTS virtual void on_add_to_parent() override;

		FLAME_UNIVERSE_EXPORTS virtual void update() override;

		FLAME_UNIVERSE_EXPORTS void open();
		FLAME_UNIVERSE_EXPORTS void close();

		FLAME_UNIVERSE_EXPORTS static cSubMenu* create();
	};

	struct cPopupMenu : Component
	{
		cElement* element;

		cPopupMenu() :
			Component("PopupMenu")
		{
		}

		FLAME_UNIVERSE_EXPORTS virtual ~cPopupMenu() override;

		FLAME_UNIVERSE_EXPORTS virtual void on_add_to_parent() override;

		FLAME_UNIVERSE_EXPORTS virtual void update() override;

		FLAME_UNIVERSE_EXPORTS void open(Entity* root, const Vec2f& pos);

		FLAME_UNIVERSE_EXPORTS static cPopupMenu* create();
	};
}
