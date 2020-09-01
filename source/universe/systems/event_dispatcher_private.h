#pragma once

#include <flame/universe/systems/event_dispatcher.h>

namespace flame
{
	struct EntityPrivate;
	struct cEventReceiverPrivate;

	struct sEventDispatcherPrivate : sEventDispatcher
	{
		Window* window = nullptr;
		void* key_down_listener = nullptr;
		void* key_up_listener = nullptr;
		void* char_listener = nullptr;
		void* mouse_left_down_listener = nullptr;
		void* mouse_left_up_listener = nullptr;
		void* mouse_right_down_listener = nullptr;
		void* mouse_right_up_listener = nullptr;
		void* mouse_middle_down_listener = nullptr;
		void* mouse_middle_up_listener = nullptr;
		void* mouse_move_listener = nullptr;
		void* mouse_scroll_listener = nullptr;
		void* destroy_listener = nullptr;

		std::pair<bool, bool> kbtns[KeyboardKey_Count];
		std::pair<bool, bool> kbtns_temp[KeyboardKey_Count];
		std::pair<bool, bool> mbtns[MouseKeyCount];
		std::pair<bool, bool> mbtns_temp[MouseKeyCount];
		Vec2i mpos = Vec2i(-1);
		Vec2i mdisp, mdisp_temp = Vec2i(0);
		int mscrl, mscrl_temp = 0;

		Vec2i active_pos = Vec2i(0);
		
		cEventReceiverPrivate* hovering = nullptr;
		cEventReceiverPrivate* focusing = nullptr;
		cEventReceiverPrivate* active = nullptr;
		cEventReceiverPrivate* dragging = nullptr;
		cEventReceiverPrivate* keyboard_target = nullptr;
		cEventReceiverPrivate* drag_overing = nullptr;

		cEventReceiverPrivate* next_focusing = (cEventReceiverPrivate*)INVALID_POINTER;

		std::vector<KeyboardKey> key_down_inputs;
		std::vector<KeyboardKey> key_up_inputs;
		std::vector<wchar_t> char_inputs;
		bool char_input_compelete = true;
		float dbclick_timer = -1.f;

		std::vector<cEventReceiverPrivate*> staging_mouse_targets;

		bool dirty = false;

		cEventReceiverPrivate* debug_target = nullptr;

		sEventDispatcherPrivate();
		void dispatch_mouse_single(cEventReceiverPrivate* er, bool force);
		void dispatch_mouse_recursively(EntityPrivate* e);
		void update() override;

		virtual void on_added() override;
		virtual void on_removed() override;
	};
}
