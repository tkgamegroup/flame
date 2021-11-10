#pragma once

#include "foundation.h"

namespace flame
{
	enum WindowStyleFlags
	{
		WindowFrame = 1 << 0,
		WindowResizable = 1 << 1,
		WindowFullscreen = 1 << 2,
		WindowMaximized = 1 << 3,
		WindowTopmost = 1 << 4
	};

	inline WindowStyleFlags operator| (WindowStyleFlags a, WindowStyleFlags b) { return (WindowStyleFlags)((int)a | (int)b); }

	enum CursorType
	{
		CursorNone = -1,
		CursorAppStarting, // arrow and small hourglass
		CursorArrow,
		CursorCross, // unknown
		CursorHand,
		CursorHelp,
		CursorIBeam,
		CursorNo,
		CursorSizeAll,
		CursorSizeNESW,
		CursorSizeNS,
		CursorSizeNWSE,
		CursorSizeWE,
		CursorUpArrwo,
		CursorWait,

		Cursor_Count
	};

	struct NativeWindow
	{
		virtual void release() = 0;

		virtual void* get_native() = 0;

		ivec2 pos;
		uvec2 size;
		std::string title;
		uint style;

		CursorType cursor;

		virtual void set_pos(const ivec2& pos) = 0;
		virtual void set_size(const uvec2& size) = 0;
		virtual ivec2 global_to_local(const ivec2& p) = 0;
		virtual void set_title(std::string_view title) = 0;
		virtual void set_cursor(CursorType type) = 0;

		virtual void* add_key_down_listener(const std::function<void(KeyboardKey)>& lis) = 0;
		virtual void remove_key_down_listener(void* lis) = 0;
		virtual void* add_key_up_listener(const std::function<void(KeyboardKey)>& lis) = 0;
		virtual void remove_key_up_listener(void* lis) = 0;
		virtual void* add_char_listener(const std::function<void(wchar_t)>& lis) = 0;
		virtual void remove_char_listener(void* lis) = 0;
		virtual void* add_mouse_left_down_listener(const std::function<void(const ivec2&)>& lis) = 0;
		virtual void remove_mouse_left_down_listener(void* lis) = 0;
		virtual void* add_mouse_left_up_listener(const std::function<void(const ivec2&)>& lis) = 0;
		virtual void remove_mouse_left_up_listener(void* lis) = 0;
		virtual void* add_mouse_right_down_listener(const std::function<void(const ivec2&)>& lis) = 0;
		virtual void remove_mouse_right_down_listener(void* lis) = 0;
		virtual void* add_mouse_right_up_listener(const std::function<void(const ivec2&)>& lis) = 0;
		virtual void remove_mouse_right_up_listener(void* lis) = 0;
		virtual void* add_mouse_middle_down_listener(const std::function<void(const ivec2&)>& lis) = 0;
		virtual void remove_mouse_middle_down_listener(void* lis) = 0;
		virtual void* add_mouse_middle_up_listener(const std::function<void(const ivec2&)>& lis) = 0;
		virtual void remove_mouse_middle_up_listener(void* lis) = 0;
		virtual void* add_mouse_move_listener(const std::function<void(const ivec2&)>& lis) = 0;
		virtual void remove_mouse_move_listener(void* lis) = 0;
		virtual void* add_mouse_scroll_listener(const std::function<void(int)>& lis) = 0;
		virtual void remove_mouse_scroll_listener(void* lis) = 0;
		virtual void* add_resize_listener(const std::function<void(const uvec2&)>& lis) = 0;
		virtual void remove_resize_listener(void* lis) = 0;
		virtual void* add_destroy_listener(const std::function<void()>& lis) = 0;
		virtual void remove_destroy_listener(void* lis) = 0;

		void* userdata = nullptr;

		FLAME_FOUNDATION_EXPORTS static NativeWindow* create(std::string_view title, const uvec2& size, WindowStyleFlags style, NativeWindow* parent = nullptr);
	};
}
