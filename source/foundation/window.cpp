#include "bitmap_private.h"
#include "window_private.h"

namespace flame
{
	static LRESULT CALLBACK wnd_proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		auto w = (NativeWindowPrivate*)GetWindowLongPtr(hWnd, 0);
		if (w)
		{
			switch (message)
			{
			case WM_KEYDOWN:
			case WM_SYSKEYDOWN:
			{
				auto v = vk_code_to_key(wParam);
				if (v > 0)
				{
					for (auto& l : w->key_down_listeners)
						l(v);
				}
			}
			return true;
			case WM_KEYUP:
			case WM_SYSKEYUP:
			{
				auto v = vk_code_to_key(wParam);
				if (v > 0)
				{
					for (auto& l : w->key_up_listeners)
						l(v);
				}
			}
			return true;
			case WM_CHAR:
				for (auto& l : w->char_listeners)
					l(wParam);
				return true;
			case WM_LBUTTONDOWN:
			{
				SetCapture(hWnd);
				auto pos = ivec2((int)LOWORD(lParam), (int)HIWORD(lParam));
				for (auto& l : w->mouse_left_down_listeners)
					l(pos);
			}
			return true;
			case WM_LBUTTONUP:
			{
				ReleaseCapture();
				auto pos = ivec2((int)LOWORD(lParam), (int)HIWORD(lParam));
				for (auto& l : w->mouse_left_up_listeners)
					l(pos);
			}
			return true;
			case WM_RBUTTONDOWN:
			{
				auto pos = ivec2((int)LOWORD(lParam), (int)HIWORD(lParam));
				for (auto& l : w->mouse_right_down_listeners)
					l(pos);
			}
			return true;
			case WM_RBUTTONUP:
			{
				auto pos = ivec2((int)LOWORD(lParam), (int)HIWORD(lParam));
				for (auto& l : w->mouse_right_up_listeners)
					l(pos);
			}
			return true;
			case WM_MBUTTONDOWN:
			{
				auto pos = ivec2((int)LOWORD(lParam), (int)HIWORD(lParam));
				for (auto& l : w->mouse_middle_down_listeners)
					l(pos);
			}
			return true;
			case WM_MBUTTONUP:
			{
				auto pos = ivec2((int)LOWORD(lParam), (int)HIWORD(lParam));
				for (auto& l : w->mouse_middle_up_listeners)
					l(pos);
			}
			return true;
			case WM_MOUSEMOVE:
			{
				auto pos = ivec2((int)LOWORD(lParam), (int)HIWORD(lParam));
				for (auto& l : w->mouse_move_listeners)
					l(pos);
			}
			return true;
			case WM_MOUSEWHEEL:
			{
				auto v = (short)HIWORD(wParam) > 0 ? 1 : -1;
				for (auto& l : w->mouse_scroll_listeners)
					l(v);
			}
			return true;
			case WM_DESTROY:
				w->dead = true;
				return true;
			case WM_SIZE:
				w->size = uvec2((int)LOWORD(lParam), (int)HIWORD(lParam));
				for (auto& l : w->resize_listeners)
					l(w->size);
				return true;
			case WM_MOVE:
				w->pos = ivec2((int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam));
				return true;
			case WM_SETCURSOR:
				if (LOWORD(lParam) == HTCLIENT)
				{
					SetCursor(w->cursors[w->cursor]);
					return true;
				}
				break;
			default:
				return DefWindowProcW(hWnd, message, wParam, lParam);
			}
		}
		return DefWindowProcW(hWnd, message, wParam, lParam);
	}

	NativeWindowPrivate::NativeWindowPrivate(const std::string& _title, const uvec2& _size, uint _style, NativeWindowPrivate* parent)
	{
		static bool initialized = false;
		if (!initialized)
		{
			WNDCLASSEXW wcex;
			wcex.cbSize = sizeof(WNDCLASSEXW);
			wcex.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
			wcex.lpfnWndProc = wnd_proc;
			wcex.cbClsExtra = 0;
			wcex.cbWndExtra = sizeof(void*);
			wcex.hInstance = (HINSTANCE)get_hinst();
			wcex.hIcon = 0;
			auto icon_fn = std::filesystem::path(L"assets\\ico.png");
			auto engine_path = getenv("FLAME_PATH");
			if (engine_path)
				icon_fn = engine_path / icon_fn;
			if (std::filesystem::exists(icon_fn))
			{
				UniPtr<Bitmap> icon_image(Bitmap::create(icon_fn));
				icon_image->swap_channel(0, 2);
				wcex.hIcon = CreateIcon(wcex.hInstance, icon_image->size.x, icon_image->size.y, 1, icon_image->bpp, nullptr, icon_image->data);
			}
			wcex.hCursor = NULL;
			wcex.hbrBackground = 0;
			wcex.lpszMenuName = 0;
			wcex.lpszClassName = L"flame_wnd";
			wcex.hIconSm = wcex.hIcon;
			RegisterClassExW(&wcex);

			initialized = true;
		}

		title = _title;

		size = _size;
		style = _style;

		cursor = CursorArrow;

		fassert(!(style & WindowFullscreen) || (!(style & WindowFrame) && !(style & WindowResizable)));

		uvec2 final_size;
		auto screen_size = get_screen_size();

		auto win32_style = WS_VISIBLE;
		if (style == 0)
			win32_style |= WS_POPUP | WS_BORDER;
		else
		{
			if (style & WindowFrame)
				win32_style |= WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
			if (style & WindowResizable)
				win32_style |= WS_THICKFRAME | WS_MAXIMIZEBOX;
			if (style & WindowFullscreen)
				final_size = screen_size;
			if (style & WindowMaximized)
				win32_style |= WS_MAXIMIZE;
		}

		auto win32_ex_style = 0L;
		if (style & WindowTopmost)
			win32_ex_style |= WS_EX_TOPMOST;

		{
			RECT rect = { 0, 0, size.x, size.y };
			AdjustWindowRect(&rect, win32_style, false);
			final_size = uvec2(rect.right - rect.left, rect.bottom - rect.top);
		}
		pos.x = (screen_size.x - final_size.x) / 2;
		pos.y = (screen_size.y - final_size.y) / 2;
		hWnd = CreateWindowExA(win32_ex_style, "flame_wnd", title.c_str(), win32_style,
			pos.x, pos.y, final_size.x, final_size.y, parent ? parent->hWnd : NULL, NULL, (HINSTANCE)get_hinst(), NULL);
		assert(IsWindowUnicode(hWnd));
		{
			RECT rect;
			GetClientRect(hWnd, &rect);
			size = uvec2(rect.right - rect.left, rect.bottom - rect.top);
		}

		SetWindowLongPtr(hWnd, 0, (LONG_PTR)this);

		cursors[CursorAppStarting] = LoadCursorA(nullptr, IDC_APPSTARTING);
		cursors[CursorArrow] = LoadCursorA(nullptr, IDC_ARROW);
		cursors[CursorCross] = LoadCursorA(nullptr, IDC_CROSS);
		cursors[CursorHand] = LoadCursorA(nullptr, IDC_HAND);
		cursors[CursorHelp] = LoadCursorA(nullptr, IDC_HELP);
		cursors[CursorIBeam] = LoadCursorA(nullptr, IDC_IBEAM);
		cursors[CursorNo] = LoadCursorA(nullptr, IDC_NO);
		cursors[CursorSizeAll] = LoadCursorA(nullptr, IDC_SIZEALL);
		cursors[CursorSizeNESW] = LoadCursorA(nullptr, IDC_SIZENESW);
		cursors[CursorSizeNS] = LoadCursorA(nullptr, IDC_SIZENS);
		cursors[CursorSizeNWSE] = LoadCursorA(nullptr, IDC_SIZENWSE);
		cursors[CursorSizeWE] = LoadCursorA(nullptr, IDC_SIZEWE);
		cursors[CursorUpArrwo] = LoadCursorA(nullptr, IDC_UPARROW);
		cursors[CursorWait] = LoadCursorA(nullptr, IDC_WAIT);

		set_cursor(CursorArrow);
	}

	NativeWindowPrivate::~NativeWindowPrivate()
	{
		for (auto& l : destroy_listeners)
			l();
	}

	void NativeWindowPrivate::release()
	{
		DestroyWindow(hWnd);
		dead = true;
	}

	void* NativeWindowPrivate::get_native()
	{
		return hWnd;
	}

	void NativeWindowPrivate::set_pos(const ivec2& _pos)
	{
		pos = _pos;
		SetWindowPos(hWnd, HWND_TOP, pos.x, pos.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
	}

	void NativeWindowPrivate::set_size(const uvec2& size)
	{
	}

	ivec2 NativeWindowPrivate::global_to_local(const ivec2& p)
	{
		POINT pt;
		pt.x = p.x;
		pt.y = p.y;
		ScreenToClient(hWnd, &pt);
		return ivec2(pt.x, pt.y);
	}

	void NativeWindowPrivate::set_title(const std::string& _title)
	{
		title = _title;
		SetWindowTextA(hWnd, title.c_str());
	}

	void NativeWindowPrivate::set_cursor(CursorType type)
	{
		if (type == cursor)
			return;

		if (cursor == CursorNone)
			ShowCursor(true);
		cursor = type;
		if (type == CursorNone)
			ShowCursor(false);
	}

	void* NativeWindowPrivate::add_key_down_listener(const std::function<void(KeyboardKey)>& lis)
	{
		return &key_down_listeners.emplace_back(lis);
	}

	void NativeWindowPrivate::remove_key_down_listener(void* lis)
	{
		std::erase_if(key_down_listeners, [&](const auto& i) {
			return &i == lis;
		});
	}

	void* NativeWindowPrivate::add_key_up_listener(const std::function<void(KeyboardKey)>& lis)
	{
		return &key_up_listeners.emplace_back(lis);
	}

	void NativeWindowPrivate::remove_key_up_listener(void* lis)
	{
		std::erase_if(key_up_listeners, [&](const auto& i) {
			return &i == lis;
		});
	}

	void* NativeWindowPrivate::add_char_listener(const std::function<void(wchar_t)>& lis)
	{
		return &char_listeners.emplace_back(lis);
	}

	void NativeWindowPrivate::remove_char_listener(void* lis)
	{
		std::erase_if(char_listeners, [&](const auto& i) {
			return &i == lis;
		});
	}

	void* NativeWindowPrivate::add_mouse_left_down_listener(const std::function<void(const ivec2&)>& lis)
	{
		return &mouse_left_down_listeners.emplace_back(lis);
	}

	void NativeWindowPrivate::remove_mouse_left_down_listener(void* lis)
	{
		std::erase_if(mouse_left_down_listeners, [&](const auto& i) {
			return &i == lis;
		});
	}

	void* NativeWindowPrivate::add_mouse_left_up_listener(const std::function<void(const ivec2&)>& lis)
	{
		return &mouse_left_up_listeners.emplace_back(lis);
	}

	void NativeWindowPrivate::remove_mouse_left_up_listener(void* lis)
	{
		std::erase_if(mouse_left_up_listeners, [&](const auto& i) {
			return &i == lis;
		});
	}

	void* NativeWindowPrivate::add_mouse_right_down_listener(const std::function<void(const ivec2&)>& lis)
	{
		return &mouse_right_down_listeners.emplace_back(lis);
	}

	void NativeWindowPrivate::remove_mouse_right_down_listener(void* lis)
	{
		std::erase_if(mouse_right_down_listeners, [&](const auto& i) {
			return &i == lis;
		});
	}

	void* NativeWindowPrivate::add_mouse_right_up_listener(const std::function<void(const ivec2&)>& lis)
	{
		return &mouse_right_up_listeners.emplace_back(lis);
	}

	void NativeWindowPrivate::remove_mouse_right_up_listener(void* lis)
	{
		std::erase_if(mouse_right_up_listeners, [&](const auto& i) {
			return &i == lis;
		});
	}

	void* NativeWindowPrivate::add_mouse_middle_down_listener(const std::function<void(const ivec2&)>& lis)
	{
		return &mouse_middle_down_listeners.emplace_back(lis);
	}

	void NativeWindowPrivate::remove_mouse_middle_down_listener(void* lis)
	{
		std::erase_if(mouse_middle_down_listeners, [&](const auto& i) {
			return &i == lis;
		});
	}

	void* NativeWindowPrivate::add_mouse_middle_up_listener(const std::function<void(const ivec2&)>& lis)
	{
		return &mouse_middle_up_listeners.emplace_back(lis);
	}

	void NativeWindowPrivate::remove_mouse_middle_up_listener(void* lis)
	{
		std::erase_if(mouse_middle_up_listeners, [&](const auto& i) {
			return &i == lis;
		});
	}

	void* NativeWindowPrivate::add_mouse_move_listener(const std::function<void(const ivec2&)>& lis)
	{
		return &mouse_move_listeners.emplace_back(lis);
	}

	void NativeWindowPrivate::remove_mouse_move_listener(void* lis)
	{
		std::erase_if(mouse_move_listeners, [&](const auto& i) {
			return &i == lis;
		});
	}

	void* NativeWindowPrivate::add_mouse_scroll_listener(const std::function<void(int)>& lis)
	{
		return &mouse_scroll_listeners.emplace_back(lis);
	}

	void NativeWindowPrivate::remove_mouse_scroll_listener(void* lis)
	{
		std::erase_if(mouse_scroll_listeners, [&](const auto& i) {
			return &i == lis;
		});
	}

	void* NativeWindowPrivate::add_resize_listener(const std::function<void(const uvec2&)>& lis)
	{
		return &resize_listeners.emplace_back(lis);
	}

	void NativeWindowPrivate::remove_resize_listener(void* lis)
	{
		std::erase_if(resize_listeners, [&](const auto& i) {
			return &i == lis;
		});
	}

	void* NativeWindowPrivate::add_destroy_listener(const std::function<void()>& lis)
	{
		return &destroy_listeners.emplace_back(lis);
	}

	void NativeWindowPrivate::remove_destroy_listener(void* lis)
	{
		std::erase_if(destroy_listeners, [&](const auto& i) {
			return &i == lis;
		});
	}

	NativeWindow* NativeWindow::create(const std::string& title, const uvec2& size, WindowStyleFlags style, NativeWindow* parent)
	{
		auto ret = new NativeWindowPrivate(title, size, style, (NativeWindowPrivate*)parent);
		windows.emplace_back(ret);
		return ret;
	}
}
