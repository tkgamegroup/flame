#pragma once

#include "receiver.h"

namespace flame
{
	struct cReceiverPrivate : cReceiver
	{
		std::vector<std::pair<uint, std::unique_ptr<Closure<void(Capture&, KeyboardKey)>>>> key_down_listeners;
		std::vector<std::pair<uint, std::unique_ptr<Closure<void(Capture&, KeyboardKey)>>>> key_up_listeners;
		std::vector<std::pair<uint, std::unique_ptr<Closure<void(Capture&, wchar_t)>>>> char_listeners;
		std::vector<std::pair<uint, std::unique_ptr<Closure<void(Capture&, const ivec2&)>>>> mouse_left_down_listeners;
		std::vector<std::pair<uint, std::unique_ptr<Closure<void(Capture&, const ivec2&)>>>> mouse_left_up_listeners;
		std::vector<std::pair<uint, std::unique_ptr<Closure<void(Capture&, const ivec2&)>>>> mouse_right_down_listeners;
		std::vector<std::pair<uint, std::unique_ptr<Closure<void(Capture&, const ivec2&)>>>> mouse_right_up_listeners;
		std::vector<std::pair<uint, std::unique_ptr<Closure<void(Capture&, const ivec2&)>>>> mouse_middle_down_listeners;
		std::vector<std::pair<uint, std::unique_ptr<Closure<void(Capture&, const ivec2&)>>>> mouse_middle_up_listeners;
		std::vector<std::pair<uint, std::unique_ptr<Closure<void(Capture&, const ivec2&, const ivec2&)>>>> mouse_move_listeners;
		std::vector<std::pair<uint, std::unique_ptr<Closure<void(Capture&, int)>>>> mouse_scroll_listeners;
		std::vector<std::pair<uint, std::unique_ptr<Closure<void(Capture&)>>>> mouse_click_listeners;
		std::vector<std::pair<uint, std::unique_ptr<Closure<void(Capture&)>>>> mouse_dbclick_listeners;

		cElementPrivate* element = nullptr;
		sDispatcherPrivate* dispatcher = nullptr;
		uint frame = 0;

		bool mute = false;
		bool ignore_occluders = false;
		uint drag_hash = 0;
		std::vector<uint> acceptable_drops;

		bool is_active();

		bool get_mute() const override { return mute; }
		void set_mute(bool v) override;

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

	template <class F>
	std::vector<std::pair<uint, Closure<F>*>> get_temp_listeners(const std::vector<std::pair<uint, std::unique_ptr<Closure<F>>>>& ls)
	{
		std::vector<std::pair<uint, Closure<F>*>> ret;
		ret.resize(ls.size());
		for (auto i = 0; i < ls.size(); i++)
		{
			ret[i].first = ls[i].first;
			ret[i].second = ls[i].second.get();
		}
		return ret;
	}
}
