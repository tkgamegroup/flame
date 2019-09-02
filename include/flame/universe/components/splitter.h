#pragma once

#include <flame/universe/component.h>

namespace flame
{
	struct cElement;
	struct cEventReceiver;
	struct cAligner;

	struct cSplitter : Component
	{
		cElement* element;
		cEventReceiver* event_receiver;

		SplitterType type;
		cElement* left_element;
		cAligner* left_aligner;
		cElement* right_element;
		cAligner* right_aligner;

		cSplitter() :
			Component("Splitter")
		{
		}

		FLAME_UNIVERSE_EXPORTS virtual ~cSplitter() override;

		FLAME_UNIVERSE_EXPORTS virtual void start() override;

		FLAME_UNIVERSE_EXPORTS static cSplitter* create();
	};
}
