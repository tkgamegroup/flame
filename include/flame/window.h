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

		enum Listener
		{
			ListenerKey,
			ListenerMouse,
			ListenerResize,
			ListenerDestroy
		};

		FLAME_PARM_PACKAGE_BEGIN(KeyListenerParm)
			/*
				- when key down/up, action is KeyStateDown or KeyStateUp, value is Key
				- when char, action is KeyStateNull, value is ch
			*/
			FLAME_PARM_PACKAGE_PARM(KeyState, action, i1)
			FLAME_PARM_PACKAGE_PARM(int, value, i1)

			FLAME_PARM_PACKAGE_SEPARATOR

			FLAME_PARM_PACKAGE_DEFAULT_CAPT(voidptr, thiz, p)
		FLAME_PARM_PACKAGE_END

		FLAME_PARM_PACKAGE_BEGIN(MouseListenerParm)
			/*
				- when down/up, action is KeyStateDown or KeyStateUp, key is MouseKey
				- when move, action is KeyStateNull, key is Mouse_Null
				- when scroll, action is KeyStateNull, key is Mouse_Middle, pos.x is scroll value
			*/
			FLAME_PARM_PACKAGE_PARM(KeyState, action, i1)
			FLAME_PARM_PACKAGE_PARM(MouseKey, key, i1)
			FLAME_PARM_PACKAGE_PARM(Ivec2, pos, i2)

			FLAME_PARM_PACKAGE_SEPARATOR

			FLAME_PARM_PACKAGE_DEFAULT_CAPT(voidptr, thiz, p)
		FLAME_PARM_PACKAGE_END

		FLAME_PARM_PACKAGE_BEGIN(ResizeListenerParm)
			FLAME_PARM_PACKAGE_PARM(Ivec2, size, i2)

			FLAME_PARM_PACKAGE_SEPARATOR

			FLAME_PARM_PACKAGE_DEFAULT_CAPT(voidptr, thiz, p)
		FLAME_PARM_PACKAGE_END

		FLAME_PARM_PACKAGE_BEGIN(DestroyListenerParm)
			FLAME_PARM_PACKAGE_SEPARATOR

			FLAME_PARM_PACKAGE_DEFAULT_CAPT(voidptr, thiz, p)
		FLAME_PARM_PACKAGE_END

		FLAME_WINDOW_EXPORTS Function *add_listener(Listener l, PF pf, void *thiz, const std::vector<CommonData> &capt);
		FLAME_WINDOW_EXPORTS void remove_listener(Listener l, Function *f);

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
