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

#include <flame/foundation/bitmap.h>|
#include <flame/foundation/window.h>

#ifdef FLAME_WINDOWS

#define NOMINMAX
#include <Windows.h>
#elif FLAME_ANDROID
#include <android_native_app_glue.h>
#endif

#include <assert.h>

namespace flame
{
#ifdef FLAME_WINDOWS
	inline Key Z(int code)
	{
		switch (code)
		{
		case VK_BACK:
			return Key_Backspace;
		case VK_TAB:
			return Key_Tab;
		case VK_RETURN:
			return Key_Enter;
		case VK_SHIFT:
			return Key_Shift;
		case VK_CONTROL:
			return Key_Ctrl;
		case VK_MENU:
			return Key_Alt;
		case VK_PAUSE:
			return Key_Pause;
		case VK_CAPITAL:
			return Key_CapsLock;
		case VK_ESCAPE:
			return Key_Esc;
		case VK_SPACE:
			return Key_Space;
		case VK_PRIOR:
			return Key_PgUp;
		case VK_NEXT:
			return Key_PgDn;
		case VK_END:
			return Key_End;
		case VK_HOME:
			return Key_Home;
		case VK_LEFT:
			return Key_Left;
		case VK_UP:
			return Key_Up;
		case VK_RIGHT:
			return Key_Right;
		case VK_DOWN:
			return Key_Down;
		case VK_SNAPSHOT:
			return Key_PrtSc;
		case VK_INSERT:
			return Key_Ins;
		case VK_DELETE:
			return Key_Del;
		case '0':
			return Key_0;
		case '1':
			return Key_1;
		case '2':
			return Key_2;
		case '3':
			return Key_3;
		case '4':
			return Key_4;
		case '5':
			return Key_5;
		case '6':
			return Key_6;
		case '7':
			return Key_7;
		case '8':
			return Key_8;
		case '9':
			return Key_9;
		case 'A':
			return Key_A;
		case 'B':
			return Key_B;
		case 'C':
			return Key_C;
		case 'D':
			return Key_D;
		case 'E':
			return Key_E;
		case 'F':
			return Key_F;
		case 'G':
			return Key_G;
		case 'H':
			return Key_H;
		case 'I':
			return Key_I;
		case 'J':
			return Key_J;
		case 'K':
			return Key_K;
		case 'L':
			return Key_L;
		case 'M':
			return Key_M;
		case 'N':
			return Key_N;
		case 'O':
			return Key_O;
		case 'P':
			return Key_P;
		case 'Q':
			return Key_Q;
		case 'R':
			return Key_R;
		case 'S':
			return Key_S;
		case 'T':
			return Key_T;
		case 'U':
			return Key_U;
		case 'V':
			return Key_V;
		case 'W':
			return Key_W;
		case 'X':
			return Key_X;
		case 'Y':
			return Key_Y;
		case 'Z':
			return Key_Z;
		case VK_NUMPAD0:
			return Key_Numpad0;
		case VK_NUMPAD1:
			return Key_Numpad1;
		case VK_NUMPAD2:
			return Key_Numpad2;
		case VK_NUMPAD3:
			return Key_Numpad3;
		case VK_NUMPAD4:
			return Key_Numpad4;
		case VK_NUMPAD5:
			return Key_Numpad5;
		case VK_NUMPAD6:
			return Key_Numpad6;
		case VK_NUMPAD7:
			return Key_Numpad7;
		case VK_NUMPAD8:
			return Key_Numpad8;
		case VK_NUMPAD9:
			return Key_Numpad9;
		case VK_ADD:
			return Key_Add;
		case VK_SUBTRACT:
			return Key_Subtract;
		case VK_MULTIPLY:
			return Key_Multiply;
		case VK_DIVIDE:
			return Key_Divide;
		case VK_SEPARATOR:
			return Key_Separator;
		case VK_DECIMAL:
			return Key_Decimal;
		case VK_F1:
			return Key_F1;
		case VK_F2:
			return Key_F2;
		case VK_F3:
			return Key_F3;
		case VK_F4:
			return Key_F4;
		case VK_F5:
			return Key_F5;
		case VK_F6:
			return Key_F6;
		case VK_F7:
			return Key_F7;
		case VK_F8:
			return Key_F8;
		case VK_F9:
			return Key_F9;
		case VK_F10:
			return Key_F10;
		case VK_F11:
			return Key_F11;
		case VK_F12:
			return Key_F12;
		case VK_NUMLOCK:
			return Key_NumLock;
		case VK_SCROLL:
			return Key_ScrollLock;
		default:
			return Key_Null;
		}
	}
#endif

	enum KeyEventType
	{
		KeyEventNull,
		KeyEventDown,
		KeyEventUp
	};

	struct WindowPrivate;

	struct ApplicationPrivate : Application
	{
		std::vector<WindowPrivate*> windows;

		long long last_time;
		long long last_frame_time;
		long long counting_frame;

		std::vector<Function*> delay_events;

		ApplicationPrivate();
		~ApplicationPrivate();

		int run(PF pf, const std::vector<CommonData> &capt);
		void add_delay_event(PF pf, const std::vector<CommonData> &capt);
		void clear_delay_events();
	};

	struct WindowPrivate : Window
	{
		ApplicationPrivate *app;

#ifdef FLAME_WINDOWS
		HWND hWnd;

		HCURSOR cursors[CursorCount];
		CursorType cursor_type;
#elif FLAME_ANDROID
		android_app *android_state_;
#endif

		std::vector<Function*> key_listeners;
		std::vector<Function*> mouse_listeners;
		std::vector<Function*> resize_listeners;
		std::vector<Function*> destroy_listeners;

		bool dead;

#ifdef FLAME_WINDOWS
		inline WindowPrivate(const char *_title, const Ivec2 &_size, int _style)
		{
			title = _title;

			minimized = false;

			hWnd = 0;

			set_size(Ivec2(-1), _size, _style);

			SetWindowLongPtr(hWnd, 0, (LONG_PTR)this);

			for (auto i = 0; i < CursorCount; i++)
			{
				switch ((CursorType)i)
				{
				case CursorAppStarting:
					cursors[i] = LoadCursorA(nullptr, IDC_APPSTARTING);
					break;
				case CursorArrow:
					cursors[i] = LoadCursorA(nullptr, IDC_ARROW);
					break;
				case CursorCross:
					cursors[i] = LoadCursorA(nullptr, IDC_CROSS);
					break;
				case CursorHand:
					cursors[i] = LoadCursorA(nullptr, IDC_HAND);
					break;
				case CursorHelp:
					cursors[i] = LoadCursorA(nullptr, IDC_HELP);
					break;
				case CursorIBeam:
					cursors[i] = LoadCursorA(nullptr, IDC_IBEAM);
					break;
				case CursorNo:
					cursors[i] = LoadCursorA(nullptr, IDC_NO);
					break;
				case CursorSizeAll:
					cursors[i] = LoadCursorA(nullptr, IDC_SIZEALL);
					break;
				case CursorSizeNESW:
					cursors[i] = LoadCursorA(nullptr, IDC_SIZENESW);
					break;
				case CursorSizeNS:
					cursors[i] = LoadCursorA(nullptr, IDC_SIZENS);
					break;
				case CursorSizeNWSE:
					cursors[i] = LoadCursorA(nullptr, IDC_SIZENWSE);
					break;
				case CursorSizeWE:
					cursors[i] = LoadCursorA(nullptr, IDC_SIZEWE);
					break;
				case CursorUpArrwo:
					cursors[i] = LoadCursorA(nullptr, IDC_UPARROW);
					break;
				case CursorWait:
					cursors[i] = LoadCursorA(nullptr, IDC_WAIT);
					break;
				}
			}
			cursor_type = CursorArrow;

			dead = false;
		}
#elif FLAME_ANDROID
		inline WindowPrivate(android_app *android_state) :
			android_state_(android_state)
		{
		}
#endif

		inline ~WindowPrivate()
		{
			for (auto &f : destroy_listeners)
				f->exec();
		}

#ifdef FLAME_WINDOWS
		inline void set_cursor(CursorType type)
		{
			if (type == cursor_type)
				return;
			cursor_type = type;

			if (cursor_type == CursorNone)
				ShowCursor(false);
			else
			{
				SetCursor(cursors[type]);
				ShowCursor(true);
			}
		}

		inline void set_size(const Ivec2 &_pos, const Ivec2 &_size, int _style)
		{
			if (_size.x > 0)
				size.x = _size.x;
			if (_size.y > 0)
				size.y = _size.y;

			bool style_changed = false;
			if (_style != -1 && _style != style)
			{
				style = _style;
				style_changed = true;
			}

			assert(!(style & WindowFullscreen) || (!(style & WindowFrame) && !(style & WindowResizable)));

			Ivec2 final_size;
			auto screen_size = get_screen_size();

			auto win32_style = WS_VISIBLE;
			if (style == 0)
				win32_style |= WS_POPUP | WS_BORDER;
			else
			{
				if (style & WindowFullscreen)
					final_size = screen_size;
				if (style & WindowFrame)
					win32_style |= WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
				if (style & WindowResizable)
					win32_style |= WS_THICKFRAME | WS_MAXIMIZEBOX;
			}

			RECT rect = { 0, 0, size.x, size.y };
			AdjustWindowRect(&rect, win32_style, false);
			final_size.x = rect.right - rect.left;
			final_size.y = rect.bottom - rect.top;

			pos.x = _pos.x == -1 ? (screen_size.x - final_size.x) / 2 : _pos.x;
			pos.y = _pos.y == -1 ? (screen_size.y - final_size.y) / 2 : _pos.y;

			if (!hWnd)
			{
				hWnd = CreateWindowA("flame_wnd", title.v, win32_style,
					pos.x, pos.y, final_size.x, final_size.y, NULL, NULL, (HINSTANCE)get_hinst(), NULL);
			}
			else
			{
				if (style_changed)
					SetWindowLong(hWnd, GWL_STYLE, win32_style);
				MoveWindow(hWnd, pos.x, pos.y, size.x, size.y, true);
			}
		}

		inline void set_maximized(bool v)
		{
			ShowWindow(hWnd, v ? SW_SHOWMAXIMIZED : SW_SHOWNORMAL);
		}
#endif

		inline Function *add_listener(Listener l, PF pf, void *thiz, const std::vector<CommonData> &_capt)
		{
			auto parm_cnt = 0;
			std::vector<Function*> *list;

			switch (l)
			{
			case ListenerKey:
				parm_cnt = KeyListenerParm::PARM_SIZE;
				list = &key_listeners;
				break;
			case ListenerMouse:
				parm_cnt = MouseListenerParm::PARM_SIZE;
				list = &mouse_listeners;
				break;
			case ListenerResize:
				parm_cnt = ResizeListenerParm::PARM_SIZE;
				list = &resize_listeners;
				break;
			case ListenerDestroy:
				parm_cnt = DestroyListenerParm::PARM_SIZE;
				list = &destroy_listeners;
				break;
			default:
				assert(0);
				return nullptr;
			}

			auto capt = _capt;
			capt.emplace(capt.begin(), thiz);
			auto f = Function::create(pf, parm_cnt, capt);
			list->push_back(f);
			return f;
		}

		inline void remove_listener(Listener l, Function *f)
		{
			std::vector<Function*> *list;

			switch (l)
			{
			case ListenerKey:
				list = &key_listeners;
				break;
			case ListenerMouse:
				list = &mouse_listeners;
				break;
			case ListenerResize:
				list = &resize_listeners;
				break;
			case ListenerDestroy:
				list = &destroy_listeners;
				break;
			default:
				assert(0);
				return;
			}

			for (auto it = list->begin(); it != list->end(); it++)
			{
				if (*it == f)
				{
					Function::destroy(f);
					list->erase(it);
					return;
				}
			}
		}
	};

	void *Window::get_native()
	{
#ifdef FLAME_WINDOWS
		return reinterpret_cast<WindowPrivate*>(this)->hWnd;
#elif FLAME_ANDROID
		return reinterpret_cast<WindowPrivate*>(this)->android_state_;
#endif
	}

#ifdef FLAME_WINDOWS
	void Window::set_cursor(CursorType type)
	{
		reinterpret_cast<WindowPrivate*>(this)->set_cursor(type);
	}

	void Window::set_size(const Ivec2 &_pos, const Ivec2 &_size, int _style)
	{
		reinterpret_cast<WindowPrivate*>(this)->set_size(_pos, _size, _style);
	}

	void Window::set_maximized(bool v)
	{
		reinterpret_cast<WindowPrivate*>(this)->set_maximized(v);
	}
#endif

	Function *Window::add_listener(Listener l, PF pf, void *thiz, const std::vector<CommonData> &capt)
	{
		return ((WindowPrivate*)this)->add_listener(l, pf, thiz, capt);
	}

	void Window::remove_listener(Listener l, Function *f)
	{
		((WindowPrivate*)this)->remove_listener(l, f);
	}

#ifdef FLAME_WINDOWS
	bool Window::is_modifier_pressing(Key k, int left_or_right)
	{
		int kc;
		switch (k)
		{
		case Key_Shift:
			kc = left_or_right == 0 ? VK_LSHIFT : VK_RSHIFT;
			break;
		case Key_Ctrl:
			kc = left_or_right == 0 ? VK_LCONTROL : VK_RCONTROL;
			break;
		case Key_Alt:
			kc = left_or_right == 0 ? VK_LMENU : VK_RMENU;
			break;
		default:
			return false;
		}
		return GetKeyState(kc) < 0;
	}

	static LRESULT CALLBACK _wnd_proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		auto w = reinterpret_cast<WindowPrivate*>(GetWindowLongPtr(hWnd, 0));

		if (w)
		{
			switch (message)
			{
			case WM_KEYDOWN:
			{
				auto v = Z(wParam);
				for (auto f : w->key_listeners)
				{
					auto &p = (Window::KeyListenerParm&)f->p;
					p.action() = KeyStateDown;
					p.value() = v;
					f->exec();
				}
			}
				break;
			case WM_KEYUP:
			{
				auto v = Z(wParam);
				for (auto f : w->key_listeners)
				{
					auto &p = (Window::KeyListenerParm&)f->p;
					p.action() = KeyStateUp;
					p.value() = v;
					f->exec();
				}
				break;
			}
			case WM_CHAR:
				for (auto f : w->key_listeners)
				{
					auto &p = (Window::KeyListenerParm&)f->p;
					p.action() = KeyStateNull;
					p.value() = wParam;
					f->exec();
				}
				break;
			case WM_LBUTTONDOWN:
			{
				auto pos = Ivec2(LOWORD(lParam), HIWORD(lParam));
				for (auto f : w->mouse_listeners)
				{
					auto &p = (Window::MouseListenerParm&)f->p;
					p.action() = KeyStateDown;
					p.key() = Mouse_Left;
					p.pos() = pos;
					f->exec();
				}
			}
				break;
			case WM_LBUTTONUP:
			{
				auto pos = Ivec2(LOWORD(lParam), HIWORD(lParam));
				for (auto f : w->mouse_listeners)
				{
					auto &p = (Window::MouseListenerParm&)f->p;
					p.action() = KeyStateUp;
					p.key() = Mouse_Left;
					p.pos() = pos;
					f->exec();
				}
			}
				break;
			case WM_MBUTTONDOWN:
			{
				auto pos = Ivec2(LOWORD(lParam), HIWORD(lParam));
				for (auto f : w->mouse_listeners)
				{
					auto &p = (Window::MouseListenerParm&)f->p;
					p.action() = KeyStateDown;
					p.key() = Mouse_Middle;
					p.pos() = pos;
					f->exec();
				}
			}
				break;
			case WM_MBUTTONUP:
			{
				auto pos = Ivec2(LOWORD(lParam), HIWORD(lParam));
				for (auto f : w->mouse_listeners)
				{
					auto &p = (Window::MouseListenerParm&)f->p;
					p.action() = KeyStateUp;
					p.key() = Mouse_Middle;
					p.pos() = pos;
					f->exec();
				}
			}
				break;
			case WM_RBUTTONDOWN:
			{
				auto pos = Ivec2(LOWORD(lParam), HIWORD(lParam));
				for (auto f : w->mouse_listeners)
				{
					auto &p = (Window::MouseListenerParm&)f->p;
					p.action() = KeyStateDown;
					p.key() = Mouse_Right;
					p.pos() = pos;
					f->exec();
				}
			}
				break;
			case WM_RBUTTONUP:
			{
				auto pos = Ivec2(LOWORD(lParam), HIWORD(lParam));
				for (auto f : w->mouse_listeners)
				{
					auto &p = (Window::MouseListenerParm&)f->p;
					p.action() = KeyStateUp;
					p.key() = Mouse_Right;
					p.pos() = pos;
					f->exec();
				}
			}
				break;
			case WM_MOUSEMOVE:
			{
				auto pos = Ivec2(LOWORD(lParam), HIWORD(lParam));
				for (auto f : w->mouse_listeners)
				{
					auto &p = (Window::MouseListenerParm&)f->p;
					p.action() = KeyStateNull;
					p.key() = Mouse_Null;
					p.pos() = pos;
					f->exec();
				}
			}
				break;
			case WM_MOUSEWHEEL:
			{
				auto v = (short)HIWORD(wParam) > 0 ? 1 : -1;
				for (auto f : w->mouse_listeners)
				{
					auto &p = (Window::MouseListenerParm&)f->p;
					p.action() = KeyStateNull;
					p.key() = Mouse_Middle;
					p.pos().x = v;
					f->exec();
				}
			}
				break;
			case WM_DESTROY:
				w->dead = true;
			case WM_SIZE:
			{
				auto size = Ivec2((int)LOWORD(lParam), (int)HIWORD(lParam));
				w->minimized = (size.x == 0 || size.y == 0);
				if (size != w->size)
				{
					w->size = size;
					for (auto f : w->resize_listeners)
					{
						auto &p = (Window::ResizeListenerParm&)f->p;
						p.size() = size;
						f->exec();
					}
				}
			}
				break;
			}
		}

		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	Window *Window::create(Application *app, const char *_title, const Ivec2 &_size, int _style)
	{
		static bool initialized = false;
		if (!initialized)
		{
			WNDCLASSEXA wcex;
			wcex.cbSize = sizeof(WNDCLASSEXA);
			wcex.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
			wcex.lpfnWndProc = _wnd_proc;
			wcex.cbClsExtra = 0;
			wcex.cbWndExtra = sizeof(void*);
			wcex.hInstance = (HINSTANCE)get_hinst();
			auto icon_image = Bitmap::create_from_file(L"ico.png");
			if (icon_image)
			{
				icon_image->swap_channel(0, 2);
				wcex.hIcon = CreateIcon(wcex.hInstance, icon_image->size.x, icon_image->size.y, 1,
					icon_image->bpp, nullptr, icon_image->data);
				Bitmap::destroy(icon_image);
			}
			else
				wcex.hIcon = 0;
			wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
			wcex.hbrBackground = 0;
			wcex.lpszMenuName = 0;
			wcex.lpszClassName = "flame_wnd";
			wcex.hIconSm = wcex.hIcon;
			RegisterClassExA(&wcex);

			initialized = true;
		}

		auto w = new WindowPrivate(_title, _size, _style);
		w->app = reinterpret_cast<ApplicationPrivate*>(app);
		(reinterpret_cast<ApplicationPrivate*>(app))->windows.push_back(w);
		return w;
	}
#endif

#ifdef FLAME_ANDROID
	static int32_t android_handle_input(android_app *state, AInputEvent* event)
	{
	    if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION)
			return 1;
		return 0;
	}

	void (*created_callback)();

	static void android_handle_cmd(android_app *state, int32_t cmd)
	{
		//auto w = (Window*)app->userData;
		switch (cmd) 
		{
			case APP_CMD_SAVE_STATE:
				//engine->app->savedState = malloc(sizeof(???));
				//*((struct ???*)engine->app->savedState) = ???;
				//engine->app->savedStateSize = sizeof(struct ???);
				break;
			case APP_CMD_INIT_WINDOW:
				if (state->window)
					created_callback();
				break;
			case APP_CMD_TERM_WINDOW:
				break;
			case APP_CMD_GAINED_FOCUS:
				break;
			case APP_CMD_LOST_FOCUS:
				break;
		}
	}

	Window *Window::create(Application *app, void *android_state, void(*callback)())
	{
        auto android_state_ = reinterpret_cast<android_app*>(android_state);
		auto w = new WindowPrivate(android_state_);
		w->app = reinterpret_cast<ApplicationPrivate*>(app);
		(reinterpret_cast<ApplicationPrivate*>(app))->windows.push_back(w);

		android_state_->userData = w;
		android_state_->onAppCmd = android_handle_cmd;
		android_state_->onInputEvent = android_handle_input;
        created_callback = callback;

		return w;
	}
#endif

	void Window::destroy(Window *w)
	{
		auto &windows = reinterpret_cast<WindowPrivate*>(w)->app->windows;
		for (auto it = windows.begin(); it != windows.end(); it++)
		{
			if ((*it) == w)
			{
				windows.erase(it);
				break;
			}
		}
#ifdef FLAME_WINDOWS
		DestroyWindow(reinterpret_cast<WindowPrivate*>(w)->hWnd);
#endif
		delete reinterpret_cast<WindowPrivate*>(w);
	}

	inline ApplicationPrivate::ApplicationPrivate()
	{
	    total_frame = 0;
		fps = 0;
		elapsed_time = 0.f;
	}

	inline ApplicationPrivate::~ApplicationPrivate()
	{
		for (auto &w : windows)
		{
#ifdef FLAME_WINDOWS
			DestroyWindow(w->hWnd);
#endif
			delete w;
		}
	}

	inline int ApplicationPrivate::run(PF pf, const std::vector<CommonData> &capt)
	{
		if (windows.size() == 0)
			return 1;

		last_time = get_now_ns();
		last_frame_time = last_time;
		counting_frame = 0;
		total_frame = 0;

		auto idle_func = Function::create(pf, 0, capt);

		for (;;)
		{
#ifdef FLAME_WINDOWS
			MSG msg;
			while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
#elif FLAME_ANDROID
			int ident;
			int events;
			android_poll_source* source;

			auto w = (*windows.begin());

            while ((ident = ALooper_pollAll(0, NULL, &events, (void**)&source)) >= 0)
            {
                if (source != NULL)
                    source->process(w->android_state_, source);

                if (w->android_state_->destroyRequested != 0)
                    w->destroy_event = true;
            }
#endif

			for (auto it = windows.begin(); it != windows.end(); )
			{
				auto w = reinterpret_cast<WindowPrivate*>(*it);

				if (w->dead)
				{
					it = windows.erase(it);
					delete w;
				}
				else
					it++;
			}

			if (windows.empty())
				return 0;

            if (last_time - last_frame_time >= 1000000000)
            {
                fps = counting_frame;
                counting_frame = 0;
                last_frame_time = last_time;
            }

			idle_func->exec();

            if (!delay_events.empty())
            {
				for (auto f : delay_events)
					f->exec();
                delay_events.clear();
            }

            total_frame++;
            counting_frame++;
            auto et = last_time;
            last_time = get_now_ns();
            et = last_time - et;
            elapsed_time = et / 1000000000.f;
		}
	}

	inline void ApplicationPrivate::add_delay_event(PF pf, const std::vector<CommonData> &capt)
	{
		delay_events.emplace_back(Function::create(pf, 0, capt));
	}

	inline void ApplicationPrivate::clear_delay_events()
	{
		for (auto f : delay_events)
			Function::destroy(f);
		delay_events.clear();
	}

	int Application::run(PF pf, const std::vector<CommonData> &capt)
	{
		auto ret = ((ApplicationPrivate*)this)->run(pf, capt);
		return ret;
	}

	void Application::clear_delay_events()
	{
		((ApplicationPrivate*)this)->clear_delay_events();
	}

	void Application::add_delay_event(PF pf, const std::vector<CommonData> &capt)
	{
		((ApplicationPrivate*)this)->add_delay_event(pf, capt);
	}

	Application *Application::create()
	{
		return new ApplicationPrivate;
	}

	void Application::destroy(Application *app)
	{
		delete (ApplicationPrivate*)app;
	}
}
