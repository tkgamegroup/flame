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

		KeyStateFlags kbtns[Key_Count], kbtns_temp[Key_Count];
		KeyStateFlags mbtns[3], mbtns_temp[3];
		Vec2i mpos = Vec2i(-1);
		Vec2i mdisp, mdisp_temp = Vec2i(0);
		int mscrl, mscrl_temp = 0;

		Vec2i active_pos = Vec2i(0);
		
		cEventReceiverPrivate* hovering = nullptr;
		cEventReceiverPrivate* focusing = nullptr;
		cEventReceiverPrivate* active = nullptr;
		cEventReceiverPrivate* dragging = nullptr;
		cEventReceiverPrivate* key_target = nullptr;
		cEventReceiverPrivate* drag_overing = nullptr;

		cEventReceiverPrivate* next_focusing = (cEventReceiverPrivate*)INVALID_POINTER;

		std::vector<Key> keydown_inputs;
		std::vector<Key> keyup_inputs;
		std::vector<wchar_t> char_inputs;
		bool char_input_compelete = true;
		float dbclick_timer = -1.f;

//		cEventReceiverPrivate* mouse_event_checker;

		bool dirty = false;

		sEventDispatcherPrivate();
		void dispatch_mouse_single(cEventReceiverPrivate* er, bool force);
		void dispatch_mouse_recursively(EntityPrivate* e);
		void update() override;

		virtual void on_added() override;
		virtual void on_removed() override;
	};
}
