#pragma once

#include <flame/universe/component.h>

namespace flame
{
	struct cEventReceiver;

	struct cWindow : Component
	{
		cElement* element;
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

	struct cSizeDragger : Component 
	{
		cEventReceiver* event_receiver;

		cSizeDragger() :
			Component("SizeDragger")
		{
		}

		FLAME_UNIVERSE_EXPORTS virtual ~cSizeDragger() override;

		FLAME_UNIVERSE_EXPORTS virtual void start() override;

		FLAME_UNIVERSE_EXPORTS virtual void update() override;

		FLAME_UNIVERSE_EXPORTS static cSizeDragger* create();
	};

	struct cDockableTitle : Component
	{
		cElement* element;
		cEventReceiver* event_receiver;

		Entity* root;

		bool flying;

		cDockableTitle() :
			Component("DockableTitle")
		{
		}

		FLAME_UNIVERSE_EXPORTS virtual ~cDockableTitle() override;

		FLAME_UNIVERSE_EXPORTS virtual void start() override;

		FLAME_UNIVERSE_EXPORTS virtual void update() override;

		FLAME_UNIVERSE_EXPORTS static cDockableTitle* create();
	};
}
