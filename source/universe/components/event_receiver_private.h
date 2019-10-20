#include <flame/universe/components/event_receiver.h>

namespace flame
{
	struct cEventReceiverPrivate : cEventReceiver
	{
		std::vector<uint> acceptable_drops;

		cEventReceiverPrivate();
		~cEventReceiverPrivate();
		void on_focus(FocusType type);
		void on_key(KeyState action, uint value);
		void on_mouse(KeyState action, MouseKey key, const Vec2i& value);
		void on_drag_and_drop(DragAndDrop action, cEventReceiver* er, const Vec2i& pos);
		void on_state_changed(EventReceiverState prev_state, EventReceiverState curr_state);
		void on_component_added(Component* c) override;
		Component* copy() override;
	};
}
