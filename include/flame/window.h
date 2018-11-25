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

#include <vector>

namespace flame
{
	struct Application;

	enum KeyState
	{
		KeyStateUp = 1 << 0,
		KeyStateDown = 1 << 1,
		KeyStateJust = 1 << 2,
	};

	enum Key
	{
		Key_Unknown,
		Key_Backspace,
		Key_Tab,
		Key_Enter,
		Key_Shift,
		Key_Ctrl,
		Key_Alt,
		Key_Pause,
		Key_CapsLock,
		Key_Esc,
		Key_Space,
		Key_PgUp,
		Key_PgDn,
		Key_End,
		Key_Home,
		Key_Left,
		Key_Up,
		Key_Right,
		Key_Down,
		Key_PrtSc,
		Key_Ins,
		Key_Del,
		Key_0,
		Key_1,
		Key_2,
		Key_3,
		Key_4,
		Key_5,
		Key_6,
		Key_7,
		Key_8,
		Key_9,
		Key_A,
		Key_B,
		Key_C,
		Key_D,
		Key_E,
		Key_F,
		Key_G,
		Key_H,
		Key_I,
		Key_J,
		Key_K,
		Key_L,
		Key_M,
		Key_N,
		Key_O,
		Key_P,
		Key_Q,
		Key_R,
		Key_S,
		Key_T,
		Key_U,
		Key_V,
		Key_W,
		Key_X,
		Key_Y,
		Key_Z,
		Key_Numpad0,
		Key_Numpad1,
		Key_Numpad2,
		Key_Numpad3,
		Key_Numpad4,
		Key_Numpad5,
		Key_Numpad6,
		Key_Numpad7,
		Key_Numpad8,
		Key_Numpad9,
		Key_Add,
		Key_Subtract,
		Key_Multiply,
		Key_Divide,
		Key_Separator,
		Key_Decimal,
		Key_F1,
		Key_F2,
		Key_F3,
		Key_F4,
		Key_F5,
		Key_F6,
		Key_F7,
		Key_F8,
		Key_F9,
		Key_F10,
		Key_F11,
		Key_F12,
		Key_NumLock,
		Key_ScrollLock
	};

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

		// "kdown":   (int key)
		// "kup":     (int key)
		// "char":    (int ansicode)
		// "mdown":   (int btn, Ivec2 pos)
		// "mup":     (int btn, Ivec2 pos)
		// "mmove":   (Ivec2 pos)
		// "mscroll": (int scroll)
		// "resize":  (Ivec2 size)
		// "destroy": ()

		FLAME_WINDOW_EXPORTS Function *add_listener(unsigned int type, PF pf, const std::vector<CommonData> &capt);
		FLAME_WINDOW_EXPORTS void remove_listener(unsigned int type, Function *f);

#ifdef FLAME_WINDOWS
		// Acceptable keys: Key_Shift, Key_Ctrl and Key_Alt.
		// left_or_right - 0: left, 1: right
		FLAME_WINDOW_EXPORTS bool is_modifier_pressing(Key k, int left_or_right);

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
