#pragma once

#include <flame/universe/components/event_receiver.h>

namespace flame
{
	struct cElementPrivate;

	struct cEventReceiverPrivate : cEventReceiver
	{
		std::vector<std::unique_ptr<Closure<bool(Capture&, KeyStateFlags, MouseKey, const Vec2i&)>>> mouse_listeners;

		cElementPrivate* element = nullptr;
//		int frame;
//
//		std::vector<uint> acceptable_drops;
//
//		cEventReceiverPrivate();
//		~cEventReceiverPrivate();
//		void on_key(KeyStateFlags action, uint value);
		void on_mouse(KeyStateFlags action, MouseKey key, const Vec2i& value);
//		void on_drag_and_drop(DragAndDrop action, cEventReceiver* er, const Vec2i& pos);
//		void set_state(EventReceiverState state);
//		void on_hovering(bool hovering);
//		void on_focusing(bool focusing);
//		void on_event(EntityEvent e, void* t) override;

		void* add_mouse_listener(bool (*callback)(Capture& c, KeyStateFlags action, MouseKey key, const Vec2i& pos), const Capture& capture) override;
		void remove_mouse_listener(void* lis) override;

		void on_added() override;

		static cEventReceiverPrivate* create();
	};
}
