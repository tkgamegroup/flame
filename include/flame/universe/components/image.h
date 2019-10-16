#pragma once

#include <flame/universe/component.h>

namespace flame
{
	struct cElement;

	struct cImage : Component
	{
		cElement* element;

		uint id;
		Vec2f uv0;
		Vec2f uv1;
		Vec4c color;

		cImage() :
			Component("Image")
		{
		}

		FLAME_UNIVERSE_EXPORTS virtual void on_enter_hierarchy(Component* c) override;
		FLAME_UNIVERSE_EXPORTS virtual void update() override;

		FLAME_UNIVERSE_EXPORTS static cImage* create();
	};
}
