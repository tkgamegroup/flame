#pragma once

#include <flame/universe/component.h>

namespace flame
{
	struct cElement;
	struct cEventReceiver;

	struct cCheckbox : Component
	{
		cElement* element;
		cEventReceiver* event_receiver;

		bool checked;

		FLAME_UNIVERSE_EXPORTS cCheckbox(Entity* e);
		FLAME_UNIVERSE_EXPORTS virtual ~cCheckbox() override;

		FLAME_UNIVERSE_EXPORTS virtual void update() override;

		FLAME_UNIVERSE_EXPORTS static cCheckbox* create(Entity* e);
	};
}
