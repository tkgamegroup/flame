#pragma once

#include <flame/universe/component.h>

namespace flame
{
	struct Window;

	struct cEventReceiver;

	struct cEventDispatcher : Component
	{
		cEventReceiver* hovering;
		cEventReceiver* focusing;

		FLAME_UNIVERSE_EXPORTS cEventDispatcher(Entity* e);
		FLAME_UNIVERSE_EXPORTS virtual ~cEventDispatcher() override;

		FLAME_UNIVERSE_EXPORTS virtual void update() override;

		FLAME_UNIVERSE_EXPORTS static cEventDispatcher* create(Entity* e, Window* window = nullptr);
	};
}
