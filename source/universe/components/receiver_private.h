#pragma once

#include "../entity_private.h"
#include <flame/universe/components/receiver.h>

namespace flame
{
	struct cElementPrivate;
	struct sDispatcherPrivate;

	struct cReceiverPrivate : cReceiver
	{
		std::vector<std::unique_ptr<Closure<void(Capture&, KeyboardKey)>>> key_down_listeners;
		std::vector<std::unique_ptr<Closure<void(Capture&, KeyboardKey)>>> key_up_listeners;
		std::vector<std::unique_ptr<Closure<void(Capture&, wchar_t)>>> char_listeners;
		std::vector<std::unique_ptr<Closure<void(Capture&, const ivec2&)>>> mouse_left_down_listeners;
		std::vector<std::unique_ptr<Closure<void(Capture&, const ivec2&)>>> mouse_left_up_listeners;
		std::vector<std::unique_ptr<Closure<void(Capture&, const ivec2&)>>> mouse_right_down_listeners;
		std::vector<std::unique_ptr<Closure<void(Capture&, const ivec2&)>>> mouse_right_up_listeners;
		std::vector<std::unique_ptr<Closure<void(Capture&, const ivec2&)>>> mouse_middle_down_listeners;
		std::vector<std::unique_ptr<Closure<void(Capture&, const ivec2&)>>> mouse_middle_up_listeners;
		std::vector<std::unique_ptr<Closure<void(Capture&, const ivec2& , const ivec2&)>>> mouse_move_listeners;
		std::vector<std::unique_ptr<Closure<void(Capture&, int)>>> mouse_scroll_listeners;
		std::vector<std::unique_ptr<Closure<void(Capture&)>>> mouse_click_listeners;
		std::vector<std::unique_ptr<Closure<void(Capture&)>>> mouse_dbclick_listeners;

		cElementPrivate* element = nullptr;
		sDispatcherPrivate* dispatcher = nullptr;
		int frame = -1;

		bool ignore_occluders = false;
		uint64 drag_hash = 0;
		std::vector<uint64> acceptable_drops;

		bool is_active();

		bool get_ignore_occluders() const override { return ignore_occluders; }
		void set_ignore_occluders(bool v) override;

		void* add_key_down_listener(void (*callback)(Capture& c, KeyboardKey key), const Capture& capture) override;
		void remove_key_down_listener(void* lis) override;
		void* add_key_up_listener(void (*callback)(Capture& c, KeyboardKey key), const Capture& capture) override;
		void remove_key_up_listener(void* lis) override;
		void* add_char_listener(void (*callback)(Capture& c, wchar_t ch), const Capture& capture) override;
		void remove_char_listener(void* lis) override;
		void* add_mouse_left_down_listener(void (*callback)(Capture& c, const ivec2& pos), const Capture& capture) override;
		void remove_mouse_left_down_listener(void* lis) override;
		void* add_mouse_left_up_listener(void (*callback)(Capture& c, const ivec2& pos), const Capture& capture) override;
		void remove_mouse_left_up_listener(void* lis) override;
		void* add_mouse_right_down_listener(void (*callback)(Capture& c, const ivec2& pos), const Capture& capture) override;
		void remove_mouse_right_down_listener(void* lis) override;
		void* add_mouse_right_up_listener(void (*callback)(Capture& c, const ivec2& pos), const Capture& capture) override;
		void remove_mouse_right_up_listener(void* lis) override;
		void* add_mouse_middle_down_listener(void (*callback)(Capture& c, const ivec2& pos), const Capture& capture) override;
		void remove_mouse_middle_down_listener(void* lis) override;
		void* add_mouse_middle_up_listener(void (*callback)(Capture& c, const ivec2& pos), const Capture& capture) override;
		void remove_mouse_middle_up_listener(void* lis) override;
		void* add_mouse_move_listener(void (*callback)(Capture& c, const ivec2& disp, const ivec2& pos), const Capture& capture) override;
		void remove_mouse_move_listener(void* lis) override;
		void* add_mouse_scroll_listener(void (*callback)(Capture& c, int scroll), const Capture& capture) override;
		void remove_mouse_scroll_listener(void* lis) override;
		void* add_mouse_click_listener(void (*callback)(Capture& c), const Capture& capture) override;
		void remove_mouse_click_listener(void* lis) override;
		void* add_mouse_dbclick_listener(void (*callback)(Capture& c), const Capture& capture) override;
		void remove_mouse_dbclick_listener(void* lis) override;

		void on_key_event(KeyboardKey key, bool down) override;

		void on_added() override;
		void on_visibility_changed(bool v) override;
		void on_entered_world() override;
		void on_left_world() override;
	};
}
