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
		ivec2 mpos = ivec2(-1);
		ivec2 mdisp, mdisp_temp = ivec2(0);
		int mscrl, mscrl_temp = 0;

		ivec2 active_pos = ivec2(0);
		
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

		void set_next_focusing(cEventReceiver* er) override { next_focusing = (cEventReceiverPrivate*)er; }

		void on_added() override;
		void on_removed() override;
	};
}
