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

		Entity* pass; // invali pointer - pass all, nullptr [default] - pass nothing, or pass pointing and its children
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

		FLAME_UNIVERSE_EXPORTS void set_acceptable_drops(uint drop_count, const uint* drops);

		ListenerHub<void(void* c, KeyStateFlags action, int value)>								key_listeners;
		ListenerHub<void(void* c, KeyStateFlags action, MouseKey key, const Vec2i & pos)>		mouse_listeners;
		ListenerHub<void(void* c, DragAndDrop action, cEventReceiver * er, const Vec2i & pos)>	drag_and_drop_listeners;

		FLAME_UNIVERSE_EXPORTS void on_key(KeyStateFlags action, uint value);
		FLAME_UNIVERSE_EXPORTS void on_mouse(KeyStateFlags action, MouseKey key, const Vec2i& value);
		FLAME_UNIVERSE_EXPORTS void on_drag_and_drop(DragAndDrop action, cEventReceiver* er, const Vec2i& pos);

		FLAME_UNIVERSE_EXPORTS static cEventReceiver* create();
	};
}
