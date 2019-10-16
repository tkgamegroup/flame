#pragma once

#include <flame/universe/component.h>

namespace flame
{
	struct cElement;
	struct cEventReceiver;
	struct cAligner;

	struct cSplitter : Component
	{
		cEventReceiver* event_receiver;

		SplitterType type;

		cSplitter() :
			Component("Splitter")
		{
		}

		FLAME_UNIVERSE_EXPORTS virtual void on_enter_hierarchy(Component* c) override;
		FLAME_UNIVERSE_EXPORTS virtual Component* copy() override;

		FLAME_UNIVERSE_EXPORTS static cSplitter* create();
	};
}
