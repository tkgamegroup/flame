#pragma once

#include <flame/universe/component.h>

namespace flame
{
	struct cElement;
	struct cEventDispatcher;

	struct cEventReceiver : Component
	{
		cElement* element;
		cEventDispatcher* event_dispatcher;

		bool penetrable;

		bool hovering;
		bool focusing;
		bool active;
		bool dragging;
		EventReceiverState state;

		uint drag_hash; // non-zero means it can be draged to drop

		cEventReceiver() :
			Component("EventReceiver")
		{
		}

		FLAME_UNIVERSE_EXPORTS void set_acceptable_drops(const std::vector<uint>& hashes);

		Listeners<void(void* c, FocusType type)> focus_listeners;
		Listeners<void(void* c, KeyState action, int value)> key_listeners;
		Listeners<void(void* c, KeyState action, MouseKey key, const Vec2i & pos)> mouse_listeners;
		Listeners<void(void* c, DragAndDrop action, cEventReceiver * er, const Vec2i & pos)> drag_and_drop_listeners;
		Listeners<void(void* c, EventReceiverState prev_state, EventReceiverState curr_state)> state_changed_listeners;

		FLAME_UNIVERSE_EXPORTS virtual void on_component_added(Component* c) override;
		FLAME_UNIVERSE_EXPORTS virtual Component* copy() override;

		FLAME_UNIVERSE_EXPORTS static cEventReceiver* create();
	};
}
