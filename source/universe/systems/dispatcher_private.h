#pragma once

#include "../entity_private.h"
#include "dispatcher.h"

namespace flame
{
	struct sDispatcherPrivate : sDispatcher
	{
		NativeWindow* window = nullptr;

		sImguiPrivate* imgui = nullptr;

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
		float dbclick_timer = -1.f;

		std::vector<cReceiverPrivate*> mouse_targets;

		bool dirty = false;

		sDispatcherPrivate();

		void on_added() override;
		void setup(NativeWindow* window) override;

		void dispatch_mouse_single(cReceiverPrivate* er, bool force);
		void dispatch_mouse_recursively(EntityPrivate* e);
		void update() override;

		bool get_keyboard_state(KeyboardKey k) const override { return kbtns[k].first; }
		bool get_mouse_state(MouseKey k) const override { return mbtns[k].first; }
		ivec2 get_mouse_pos() const override { return mpos; }
		cReceiverPtr get_hovering() const override { return hovering; }
		cReceiverPtr get_focusing() const override { return focusing; }
		cReceiverPtr get_active() const override { return active; }
		void set_next_focusing(cReceiverPtr er) override { next_focusing = er; }
	};
}
