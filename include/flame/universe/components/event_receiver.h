#pragma once

#include <flame/universe/component.h>

namespace flame
{
	struct sEventDispatcher;
	struct cElement;

	struct cEventReceiver : Component
	{
		sEventDispatcher* dispatcher;
		cElement* element;

		bool penetrable;
		bool accept_key;
		uint drag_hash; // non-zero means it can be draged to drop

		bool hovering;
		bool focusing;
		bool active;
		bool dragging;
		EventReceiverState state;

		cEventReceiver() :
			Component("cEventReceiver")
		{
		}

		FLAME_UNIVERSE_EXPORTS void set_acceptable_drops(const std::vector<uint>& hashes);

		ListenerHub<void(void* c, KeyState action, int value)> key_listeners;
		ListenerHub<void(void* c, KeyState action, MouseKey key, const Vec2i & pos)> mouse_listeners;
		ListenerHub<void(void* c, DragAndDrop action, cEventReceiver * er, const Vec2i & pos)> drag_and_drop_listeners;

		FLAME_UNIVERSE_EXPORTS static cEventReceiver* create();
	};
}
