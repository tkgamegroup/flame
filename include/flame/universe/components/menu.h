#pragma once

#include <flame/universe/component.h>

namespace flame
{
	struct cEventReceiver;

	struct cMenu : Component
	{
		cElement* element;
		cEventReceiver* event_receiver;

		Entity* root;
		Entity* items;

		bool opened;

		cMenu() :
			Component("Menu")
		{
		}

		FLAME_UNIVERSE_EXPORTS virtual ~cMenu() override;

		FLAME_UNIVERSE_EXPORTS virtual void on_add_to_parent() override;

		FLAME_UNIVERSE_EXPORTS virtual void update() override;

		FLAME_UNIVERSE_EXPORTS void open(const Vec2f& pos);
		FLAME_UNIVERSE_EXPORTS void close();

		FLAME_UNIVERSE_EXPORTS static cMenu* create();
	};
}
