#pragma once

#include <flame/universe/systems/event_dispatcher.h>

namespace flame
{
	struct EntityPrivate;
	struct cEventReceiverPrivate;

	struct sEventDispatcherPrivate : sEventDispatcher
	{
		Window* window = nullptr;
		void* key_listener = nullptr;
		void* mouse_listener = nullptr;
		void* destroy_listener = nullptr;

		//		KeyStateFlags key_states[Key_Count];
		KeyStateFlags mbtns[3];
		Vec2f mpos = Vec2f(0.f);
		Vec2f mdisp = Vec2f(0.f);
		Vec2f active_pos = Vec2f(0.f);
		//		int mouse_scroll;
		
		cEventReceiverPrivate* hovering = nullptr;
		cEventReceiverPrivate* focusing = nullptr;
		//		FocusingState focusing_state;
		//		cEventReceiver* key_receiving;
		//		cEventReceiver* drag_overing;
		//
		//		cEventReceiver* next_focusing;

//		std::vector<Key> keydown_inputs;
//		std::vector<Key> keyup_inputs;
//		std::vector<wchar_t> char_inputs;
//		bool char_input_compelete;
//		float dbclick_timer;
//		void* ev_dbclick;

//		cEventReceiverPrivate* mouse_event_checker;
//
		bool dirty = false;

//		sEventDispatcherPrivate();
//		~sEventDispatcherPrivate();
//		void on_receiver_removed(cEventReceiver* er);
		void dispatch_mouse_single(cEventReceiverPrivate* er, bool force);
		void dispatch_mouse_recursively(EntityPrivate* e);
		void update() override;
//		void after_update() override;

		virtual void on_added() override;
		virtual void on_removed() override;

		static sEventDispatcherPrivate* create();
	};
}
