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

#ifdef FLAME_WINDOWS
#ifdef FLAME_WINDOW_MODULE
#define FLAME_WINDOW_EXPORTS __declspec(dllexport)
#else
#define FLAME_WINDOW_EXPORTS __declspec(dllimport)
#endif
#else
#define FLAME_WINDOW_EXPORTS
#endif

#include <flame/math.h>
#include <flame/string.h>
#include <flame/function.h>
#include <flame/input.h>

#include <vector>

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

	struct Window
	{
		Ivec2 pos;
		Ivec2 size;
		int style;
		String title;

		bool minimized;

		FLAME_WINDOW_EXPORTS void *get_native();

#ifdef FLAME_WINDOWS
		FLAME_WINDOW_EXPORTS void set_cursor(CursorType type);

		FLAME_WINDOW_EXPORTS void set_size(const Ivec2 &_pos, const Ivec2 &_size, int _style);
		FLAME_WINDOW_EXPORTS void set_maximized(bool v);
#endif

		struct KeyListenerParm : ParmPackage
		{
			enum { BASE = __COUNTER__ + 1 };

			inline KeyState &action()
			{
				return (KeyState&)d[__COUNTER__ - BASE].i1();
			}
			inline Key &key()
			{
				return (Key&)d[__COUNTER__ - BASE].i1();
			}

			enum { SIZE = __COUNTER__ - BASE };
		};
		struct CharListenerParm : ParmPackage
		{
			enum { BASE = __COUNTER__ + 1 };

			inline int &ch()
			{
				return d[__COUNTER__ - BASE].i1();
			}

			enum { SIZE = __COUNTER__ - BASE };
		};
		struct MouseListenerParm : ParmPackage
		{
			/*
				- when down/up, action is KeyStateUp or KeyStateDown, key is one of MouseKey
				- when move, action is KeyStateNull, key is Mouse_Null
				- when scroll, action is KeyStateNull, key is Mouse_Middle, pos.x is scroll value
			*/

			enum { BASE = __COUNTER__ + 1 };

			inline KeyState &action()
			{
				return (KeyState&)d[__COUNTER__ - BASE].i1();
			}
			inline MouseKey &key()
			{
				return (MouseKey&)d[__COUNTER__ - BASE].i1();
			}
			inline Ivec2 &pos()
			{
				return d[__COUNTER__ - BASE].i2();
			}

			enum { SIZE = __COUNTER__ - BASE };
		};
		struct ResizeListenerParm : ParmPackage
		{
			enum { BASE = __COUNTER__ + 1 };

			inline Ivec2 &size()
			{
				return d[__COUNTER__ - BASE].i2();
			}

			enum { SIZE = __COUNTER__ - BASE };
		};
		struct DestroyListenerParm : ParmPackage
		{
			enum { BASE = __COUNTER__ + 1 };

			enum { SIZE = __COUNTER__ - BASE };
		};

		FLAME_WINDOW_EXPORTS Function *add_key_listener(PF pf, const std::vector<CommonData> &capt);
		FLAME_WINDOW_EXPORTS void remove_key_listener(Function *f);
		FLAME_WINDOW_EXPORTS Function *add_char_listener(PF pf, const std::vector<CommonData> &capt);
		FLAME_WINDOW_EXPORTS void remove_char_listener(Function *f);
		FLAME_WINDOW_EXPORTS Function *add_mouse_listener(PF pf, const std::vector<CommonData> &capt);
		FLAME_WINDOW_EXPORTS void remove_mouse_listener(Function *f);
		FLAME_WINDOW_EXPORTS Function *add_resize_listener(PF pf, const std::vector<CommonData> &capt);
		FLAME_WINDOW_EXPORTS void remove_resize_listener(Function *f);
		FLAME_WINDOW_EXPORTS Function *add_destroy_listener(PF pf, const std::vector<CommonData> &capt);
		FLAME_WINDOW_EXPORTS void remove_destroy_listener(Function *f);

#ifdef FLAME_WINDOWS
		FLAME_WINDOW_EXPORTS bool is_modifier_pressing(Key k /* accept: Key_Shift, Key_Ctrl and Key_Alt */, int left_or_right /* 0 or 1 */);

		FLAME_WINDOW_EXPORTS static Window *create(Application *app, const char *_title, const Ivec2 &_size, int _style);
#endif

#ifdef FLAME_ANDROID
		FLAME_WINDOW_EXPORTS static Window *create(Application *app, void *android_state, void(*callback)());
#endif
		FLAME_WINDOW_EXPORTS static void destroy(Window *s);
	};

	struct Application
	{
		long long total_frame;
		long long fps;
		float elapsed_time; // second

		FLAME_WINDOW_EXPORTS int run(PF pf, const std::vector<CommonData> &capt);

		FLAME_WINDOW_EXPORTS void clear_delay_events();
		FLAME_WINDOW_EXPORTS void add_delay_event(PF pf, const std::vector<CommonData> &capt);

		FLAME_WINDOW_EXPORTS static Application *create();
		FLAME_WINDOW_EXPORTS static void destroy(Application *m);
	};
}
