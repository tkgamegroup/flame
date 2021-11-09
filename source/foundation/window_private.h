#pragma once

#include "system_private.h"
#include "window.h"

namespace flame
{
	struct NativeWindowPrivate : NativeWindow
	{
		HWND hWnd = 0;

		HCURSOR cursors[Cursor_Count];

		std::list<std::function<void(KeyboardKey)>> key_down_listeners;
		std::list<std::function<void(KeyboardKey)>> key_up_listeners;
		std::list<std::function<void(wchar_t)>> char_listeners;
		std::list<std::function<void(const ivec2&)>> mouse_left_down_listeners;
		std::list<std::function<void(const ivec2&)>> mouse_left_up_listeners;
		std::list<std::function<void(const ivec2&)>> mouse_right_down_listeners;
		std::list<std::function<void(const ivec2&)>> mouse_right_up_listeners;
		std::list<std::function<void(const ivec2&)>> mouse_middle_down_listeners;
		std::list<std::function<void(const ivec2&)>> mouse_middle_up_listeners;
		std::list<std::function<void(const ivec2&)>> mouse_move_listeners;
		std::list<std::function<void(int)>> mouse_scroll_listeners;
		std::list<std::function<void(const uvec2&)>> resize_listeners;
		std::list<std::function<void()>> destroy_listeners;

		bool dead = false;

		NativeWindowPrivate(const std::string& _title, const uvec2& _size, uint _style, NativeWindowPrivate* parent);
		~NativeWindowPrivate();

		void release() override;

		void* get_native() override;

		void set_pos(const ivec2& pos) override;
		void set_size(const uvec2& size) override;
		ivec2 global_to_local(const ivec2& p) override;
		void set_title(const std::string& _title) override;
		void set_cursor(CursorType type) override;

		void* add_key_down_listener(const std::function<void(KeyboardKey)>& lis) override;
		void remove_key_down_listener(void* lis) override;
		void* add_key_up_listener(const std::function<void(KeyboardKey)>& lis) override;
		void remove_key_up_listener(void* lis) override;
		void* add_char_listener(const std::function<void(wchar_t)>& lis) override;
		void remove_char_listener(void* lis) override;
		void* add_mouse_left_down_listener(const std::function<void(const ivec2&)>& lis) override;
		void remove_mouse_left_down_listener(void* lis) override;
		void* add_mouse_left_up_listener(const std::function<void(const ivec2&)>& lis) override;
		void remove_mouse_left_up_listener(void* lis) override;
		void* add_mouse_right_down_listener(const std::function<void(const ivec2&)>& lis) override;
		void remove_mouse_right_down_listener(void* lis) override;
		void* add_mouse_right_up_listener(const std::function<void(const ivec2&)>& lis) override;
		void remove_mouse_right_up_listener(void* lis) override;
		void* add_mouse_middle_down_listener(const std::function<void(const ivec2&)>& lis) override;
		void remove_mouse_middle_down_listener(void* lis) override;
		void* add_mouse_middle_up_listener(const std::function<void(const ivec2&)>& lis) override;
		void remove_mouse_middle_up_listener(void* lis) override;
		void* add_mouse_move_listener(const std::function<void(const ivec2&)>& lis) override;
		void remove_mouse_move_listener(void* lis) override;
		void* add_mouse_scroll_listener(const std::function<void(int)>& lis) override;
		void remove_mouse_scroll_listener(void* lis) override;
		void* add_resize_listener(const std::function<void(const uvec2&)>& lis) override;
		void remove_resize_listener(void* lis) override;
		void* add_destroy_listener(const std::function<void()>& lis) override;
		void remove_destroy_listener(void* lis) override;
	};
}
