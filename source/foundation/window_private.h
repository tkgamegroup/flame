#pragma once

#include "system_private.h"
#include "window.h"

namespace flame
{
	struct NativeWindowPrivate : NativeWindow
	{
		HWND hWnd = 0;

		HCURSOR cursors[Cursor_Count];

		bool dead = false;

		~NativeWindowPrivate();

		void* get_hwnd() override { return hWnd; }

		void close() override;

		void set_visible(bool v) override;
		void set_pos(const ivec2& pos) override;
		void set_size(const uvec2& size) override;
		ivec2 global_to_local(const ivec2& p) override;
		void set_title(std::string_view _title) override;
		void set_cursor(CursorType type) override;
	};
}
