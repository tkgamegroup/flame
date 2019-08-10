#pragma once

#include <flame/universe/component.h>

namespace flame
{
	struct Window;

	struct cEvent;

	struct cUI : Component
	{
		cEvent* hovering;
		cEvent* focusing;

		FLAME_UNIVERSE_EXPORTS cUI(Entity* e);
		FLAME_UNIVERSE_EXPORTS virtual ~cUI() override;

		FLAME_UNIVERSE_EXPORTS virtual void update() override;

		FLAME_UNIVERSE_EXPORTS static cUI* create(Entity* e, Window* window = nullptr);
	};
}
