#pragma once

#include <flame/universe/component.h>

namespace flame
{
	struct cEventReceiver;

	struct cWindow : Component
	{
		cEventReceiver* event_receiver;

		cWindow() :
			Component("Window")
		{
		}

		FLAME_UNIVERSE_EXPORTS virtual ~cWindow() override;

		FLAME_UNIVERSE_EXPORTS virtual void start() override;

		FLAME_UNIVERSE_EXPORTS virtual void update() override;

		FLAME_UNIVERSE_EXPORTS void* add_pos_listener(void (*listener)(void* c), const Mail<>& capture);

		FLAME_UNIVERSE_EXPORTS void remove_pos_listener(void* ret_by_add);

		FLAME_UNIVERSE_EXPORTS static cWindow* create();
	};
}
