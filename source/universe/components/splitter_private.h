#pragma once

#include "../entity_private.h"
#include <flame/universe/components/splitter.h>

namespace flame
{
	struct cReceiverPrivate;

	struct cSplitterPrivate : cSplitter
	{
		//cLayoutPrivate* layout = nullptr;
		cReceiverPrivate* bar_receiver = nullptr;

		void* bar_state_listener = nullptr;
		void* bar_mouse_listener = nullptr;

		void on_gain_bar_receiver();
		void on_lost_bar_receiver();

		//void on_child_message(Message msg, void* p) override;
	};
}
