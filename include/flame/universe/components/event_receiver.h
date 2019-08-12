#pragma once

#include <flame/universe/component.h>

namespace flame
{
	struct cElement;

	struct cEventReceiver : Component
	{
		cElement* element;

		bool want_key;

		bool hovering;
		bool dragging;
		bool focusing;

		FLAME_UNIVERSE_EXPORTS cEventReceiver(Entity* e);
		FLAME_UNIVERSE_EXPORTS virtual ~cEventReceiver() override;

		FLAME_UNIVERSE_EXPORTS virtual void update() override;

		FLAME_UNIVERSE_EXPORTS void on_key(KeyState action, int value);
		FLAME_UNIVERSE_EXPORTS void on_mouse(KeyState action, MouseKey key, const Vec2f& value);
		FLAME_UNIVERSE_EXPORTS void on_drop(cEventReceiver* src);

		FLAME_UNIVERSE_EXPORTS static cEventReceiver* create(Entity* e);
	};
}
