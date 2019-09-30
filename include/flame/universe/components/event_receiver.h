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

		FLAME_UNIVERSE_EXPORTS void* add_focus_listener(void (*listener)(void* c, FocusType type), const Mail<>& capture);
		FLAME_UNIVERSE_EXPORTS void* add_key_listener(void (*listener)(void* c, KeyState action, int value), const Mail<>& capture);
		FLAME_UNIVERSE_EXPORTS void* add_mouse_listener(void (*listener)(void* c, KeyState action, MouseKey key, const Vec2f& pos), const Mail<>& capture);
		FLAME_UNIVERSE_EXPORTS void* add_drag_and_drop_listener(void (*listener)(void* c, DragAndDrop action, cEventReceiver* er, const Vec2f& pos), const Mail<>& capture);
		FLAME_UNIVERSE_EXPORTS void* add_state_changed_listener(void (*listener)(void* c, EventReceiverState prev_state, EventReceiverState curr_state), const Mail<>& capture);

		FLAME_UNIVERSE_EXPORTS void remove_focus_listener(void* ret_by_add);
		FLAME_UNIVERSE_EXPORTS void remove_key_listener(void* ret_by_add);
		FLAME_UNIVERSE_EXPORTS void remove_mouse_listener(void* ret_by_add);
		FLAME_UNIVERSE_EXPORTS void remove_drag_and_drop_listener(void* ret_by_add);
		FLAME_UNIVERSE_EXPORTS void remove_state_changed_listener(void* ret_by_add);

		FLAME_UNIVERSE_EXPORTS void on_focus(FocusType type);
		FLAME_UNIVERSE_EXPORTS void on_key(KeyState action, uint value);
		FLAME_UNIVERSE_EXPORTS void on_mouse(KeyState action, MouseKey key, const Vec2f& value);
		FLAME_UNIVERSE_EXPORTS void on_drag_and_drop(DragAndDrop action, cEventReceiver* er, const Vec2f& pos);
		FLAME_UNIVERSE_EXPORTS void on_state_changed(EventReceiverState prev_state, EventReceiverState curr_state);

		FLAME_UNIVERSE_EXPORTS virtual void start() override;
		FLAME_UNIVERSE_EXPORTS virtual void update() override;
		FLAME_UNIVERSE_EXPORTS virtual Component* copy() override;

		FLAME_UNIVERSE_EXPORTS static cEventReceiver* create();
	};
}
