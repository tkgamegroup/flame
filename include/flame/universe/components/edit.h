#pragma once

#include <flame/universe/component.h>

namespace flame
{
	struct cElement;
	struct cText;
	struct cEventReceiver;

	struct cEdit : Component
	{
		cElement* element;
		cText* text;
		cEventReceiver* event_receiver;

		uint cursor;

		cEdit() :
			Component("Edit")
		{
		}

		FLAME_UNIVERSE_EXPORTS virtual ~cEdit() override;

		FLAME_UNIVERSE_EXPORTS virtual void on_added() override;

		FLAME_UNIVERSE_EXPORTS virtual void update() override;

		FLAME_UNIVERSE_EXPORTS static cEdit* create();
	};
}
