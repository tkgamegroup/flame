#pragma once

#include "../entity_private.h"
#include <flame/universe/systems/dispatcher.h>

namespace flame
{
	struct EntityPrivate;
	struct cReceiverPrivate;

	struct sDispatcherPrivate : sDispatcher
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
		
		cReceiverPrivate* hovering = nullptr;
		cReceiverPrivate* focusing = nullptr;
		cReceiverPrivate* active = nullptr;
		cReceiverPrivate* dragging = nullptr;
		cReceiverPrivate* keyboard_target = nullptr;
		cReceiverPrivate* drag_overing = nullptr;

		cReceiverPrivate* next_focusing = (cReceiverPrivate*)INVALID_POINTER;

		std::vector<KeyboardKey> key_down_inputs;
		std::vector<KeyboardKey> key_up_inputs;
		std::vector<wchar_t> char_inputs;
		bool char_input_compelete = true;
		float dbclick_timer = -1.f;

		std::vector<cReceiverPrivate*> staging_mouse_targets;

		bool dirty = false;

		cReceiverPrivate* debug_target = nullptr;

		sDispatcherPrivate();
		void dispatch_mouse_single(cReceiverPrivate* er, bool force);
		void dispatch_mouse_recursively(EntityPrivate* e);
		void update() override;

		void set_next_focusing(cReceiver* er) override { next_focusing = (cReceiverPrivate*)er; }

		void on_added() override;
		void on_removed() override;
	};
}