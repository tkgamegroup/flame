#pragma once

#include <flame/universe/components/event_receiver.h>

namespace flame
{
	struct cElementPrivate;
	struct sEventDispatcherPrivate;

	struct cEventReceiverPrivate : cEventReceiver // R ~ on_*
	{
		std::vector<std::unique_ptr<Closure<bool(Capture&, KeyStateFlags action, uint value)>>> key_listeners;
		std::vector<std::unique_ptr<Closure<bool(Capture&, KeyStateFlags, MouseKey, const Vec2i&)>>> mouse_listeners;

		cElementPrivate* element = nullptr; // R ref
		sEventDispatcherPrivate* dispatcher = nullptr; // R ref
		int frame = -1;

//		std::vector<uint> acceptable_drops;
//
//		cEventReceiverPrivate();
		void send_key_event(KeyStateFlags action, uint value);
		void send_mouse_event(KeyStateFlags action, MouseKey key, const Vec2i& value);
//		void send_drag_and_drop_event(DragAndDrop action, cEventReceiver* er, const Vec2i& pos);

		void* add_key_listener(bool (*callback)(Capture& c, KeyStateFlags action, uint value), const Capture& capture) override;
		void remove_key_listener(void* lis) override;
		void* add_mouse_listener(bool (*callback)(Capture& c, KeyStateFlags action, MouseKey key, const Vec2i& pos), const Capture& capture) override;
		void remove_mouse_listener(void* lis) override;

		void on_gain_dispatcher();
		void on_lost_dispatcher();

		void on_entity_visibility_changed() override;
	};
}
