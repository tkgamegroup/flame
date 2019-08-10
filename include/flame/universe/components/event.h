#pragma once

#include <flame/universe/component.h>

namespace flame
{
	struct cEvent : Component // requires: Element
	{
		bool blackhole;
		bool want_key;

		bool hovering;
		bool dragging;
		bool focusing;

		FLAME_UNIVERSE_EXPORTS cEvent(Entity* e);
		FLAME_UNIVERSE_EXPORTS virtual ~cEvent() override;

		FLAME_UNIVERSE_EXPORTS virtual void update() override;

		FLAME_UNIVERSE_EXPORTS bool contains(const Vec2f& pos) const;

		FLAME_UNIVERSE_EXPORTS void on_key(KeyState action, int value);
		FLAME_UNIVERSE_EXPORTS void on_mouse(KeyState action, MouseKey key, const Vec2f& value);
		FLAME_UNIVERSE_EXPORTS void on_drop(cEvent* src);

		FLAME_UNIVERSE_EXPORTS static cEvent* create(Entity* e);
	};
}
