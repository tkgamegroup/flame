// MIT License
// 
// Copyright (c) 2018 wjs
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

#include <flame/foundation/foundation.h>

namespace flame
{
	struct Application;

	enum WindowStyle
	{
		WindowFrame = 1 << 0,
		WindowResizable = 1 << 1,
		WindowFullscreen = 1 << 2
	};

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

		CursorCount
	};

	struct Window;
	typedef Window* WindowPtr;

	struct Window
	{
		Vec2i pos;
		Vec2u size;
		int style;
		String title;

		bool minimized;

		FLAME_FOUNDATION_EXPORTS void *get_native();

#ifdef FLAME_WINDOWS
		FLAME_FOUNDATION_EXPORTS void set_cursor(CursorType type);

		FLAME_FOUNDATION_EXPORTS void set_size(const Vec2i &_pos, const Vec2u &_size, int _style);
		FLAME_FOUNDATION_EXPORTS void set_maximized(bool v);
#endif

		FLAME_FOUNDATION_EXPORTS int add_key_listener(Function<void(void* c, KeyState action, Key key)>& listener);
		FLAME_FOUNDATION_EXPORTS int add_mouse_listener(Function<void(void* c, KeyState action, MouseKey key, const Vec2i& pos)>& listener);
		FLAME_FOUNDATION_EXPORTS int add_resize_listener(Function<void(void* c, const Vec2u& size)>& listener);
		FLAME_FOUNDATION_EXPORTS int add_destroy_listener(Function<void(void* c)>& listener);

		FLAME_FOUNDATION_EXPORTS void remove_key_listener(int id);
		FLAME_FOUNDATION_EXPORTS void remove_mouse_listener(int id);
		FLAME_FOUNDATION_EXPORTS void remove_resize_listener(int id);
		FLAME_FOUNDATION_EXPORTS void remove_destroy_listener(int id);

		FLAME_FOUNDATION_EXPORTS void close();

		FLAME_FOUNDATION_EXPORTS static Window *create(Application *app, const char *_title, const Vec2u& _size, int _style);
		FLAME_FOUNDATION_EXPORTS static void destroy(Window *s);
	};

	struct Application
	{
		long long total_frame;
		long long fps;
		float elapsed_time; // second

		FLAME_FOUNDATION_EXPORTS int run(Function<void(void* c)> &idle_func);

		FLAME_FOUNDATION_EXPORTS void clear_delay_events();
		FLAME_FOUNDATION_EXPORTS void add_delay_event(Function<void(void* c)> &event);

		FLAME_FOUNDATION_EXPORTS static Application *create();
		FLAME_FOUNDATION_EXPORTS static void destroy(Application *m);
	};
}
