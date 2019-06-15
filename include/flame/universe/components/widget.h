#pragma once

#include <flame/universe/component.h>

namespace flame
{
	struct cWidget$ : Component // requires: Element
	{
		bool blackhole;
		bool want_key;

		ATTRIBUTE_BOOL hovering;
		ATTRIBUTE_BOOL dragging;
		ATTRIBUTE_BOOL focusing;

		FLAME_UNIVERSE_EXPORTS virtual ~cWidget$() override;

		FLAME_UNIVERSE_EXPORTS virtual const char* type_name() const override;
		FLAME_UNIVERSE_EXPORTS virtual uint type_hash() const override;

		FLAME_UNIVERSE_EXPORTS virtual void on_attach() override;

		FLAME_UNIVERSE_EXPORTS virtual void update(float delta_time) override;

		FLAME_UNIVERSE_EXPORTS bool contains(const Vec2f& pos) const;

		FLAME_UNIVERSE_EXPORTS void on_key(KeyState action, int value);
		FLAME_UNIVERSE_EXPORTS void on_mouse(KeyState action, MouseKey key, const Vec2f& value);
		FLAME_UNIVERSE_EXPORTS void on_drop(cWidget$* src);

		FLAME_UNIVERSE_EXPORTS static cWidget$* create$(void* data);
	};
}
