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
		bool stretch;
		Vec4f border;

		FLAME_UNIVERSE_EXPORTS cImage(Entity* e);
		FLAME_UNIVERSE_EXPORTS virtual ~cImage() override;

		FLAME_UNIVERSE_EXPORTS virtual void update() override;

		FLAME_UNIVERSE_EXPORTS static cImage* create(Entity* e);
	};
}
