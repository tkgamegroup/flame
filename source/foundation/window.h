#pragma once

#include "foundation.h"

namespace flame
{
	struct NativeWindow
	{
		ivec2 pos;
		uvec2 size;
		Rect cl_rect = Rect(0.f, 0.f, 1.f, 1.f);
		std::string title;
		uint styles;

		WindowState state;
		bool focused = false;

		CursorType cursor = CursorArrow;

		Listeners<void(MouseButton btn, bool down)> mouse_listeners;
		Listeners<void(const ivec2&)>				mouse_move_listeners;
		Listeners<void(int)>						mouse_scroll_listeners;
		Listeners<void(KeyboardKey key, bool down)> key_listeners;
		Listeners<void(wchar_t)>					char_listeners;
		Listeners<void(const uvec2&)>				resize_listeners;
		Listeners<void(bool)>						focus_listeners;
		Listeners<void(bool& closing)>				close_listeners;
		Listeners<void()>							destroy_listeners;
		
		bool has_input = false;
		ivec2 mpos = ivec2(-1);

		void* userdata = nullptr;

		bool dead = false;

		virtual void* get_hwnd() = 0;

		virtual void close() = 0;

		virtual void show(WindowState state) = 0;
		virtual void set_focus() = 0;
		virtual void set_pos(const ivec2& pos) = 0;
		virtual void set_size(const uvec2& size) = 0;
		virtual ivec2 local_to_global(const ivec2& p) = 0;
		virtual ivec2 global_to_local(const ivec2& p) = 0;
		virtual void set_title(std::string_view title) = 0;
		virtual void set_cursor(CursorType type) = 0;

		struct Create
		{
			virtual NativeWindowPtr operator()(std::string_view title, const uvec2& size, WindowStyleFlags styles, NativeWindowPtr parent = nullptr) = 0;
		};
		FLAME_FOUNDATION_API static Create& create;

		struct List
		{
			virtual const std::vector<NativeWindowPtr>& operator()() = 0;
		};
		FLAME_FOUNDATION_API static List& list;
	};

	struct MonitorInfo
	{
		ivec2 main_pos;
		ivec2 main_size;
		ivec2 work_pos;
		ivec2 work_size;
		float dpi_scale;
		bool is_primary;
		void* handle;
	};

	FLAME_FOUNDATION_API std::vector<MonitorInfo> get_monitors();
}
