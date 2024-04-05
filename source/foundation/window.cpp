#include "bitmap_private.h"
#include "window_private.h"

namespace flame
{
#define GET_X_LPARAM(lp) ((int)(short)LOWORD(lp))
#define GET_Y_LPARAM(lp) ((int)(short)HIWORD(lp))

	static LRESULT CALLBACK wnd_proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		auto w = (NativeWindowPrivate*)GetWindowLongPtr(hWnd, 0);
		if (w)
		{
			switch (message)
			{
			case WM_KEYDOWN:
			case WM_SYSKEYDOWN:
				if (auto v = vk_code_to_key(wParam); v != KeyboardKey_Count)
				{
					w->has_input = true;
					w->key_listeners.call(v, true);
				}
				return true;
			case WM_KEYUP:
			case WM_SYSKEYUP:
				if (auto v = vk_code_to_key(wParam); v != KeyboardKey_Count)
				{
					w->has_input = true;
					w->key_listeners.call(v, false);
				}
				return true;
			case WM_CHAR:
				w->has_input = true;
				w->char_listeners.call(wParam);
				return true;
			case WM_LBUTTONDOWN:
				SetCapture(hWnd);
				w->has_input = true;
				w->mpos = ivec2((int)GET_X_LPARAM(lParam), (int)HIWORD(lParam));
				w->mouse_listeners.call(Mouse_Left, true);
				return true;
			case WM_LBUTTONUP:
				ReleaseCapture();
				w->has_input = true;
				w->mpos = ivec2(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
				w->mouse_listeners.call(Mouse_Left, false);
				return true;
			case WM_RBUTTONDOWN:
				w->has_input = true;
				w->mpos = ivec2(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
				w->mouse_listeners.call(Mouse_Right, true);
				return true;
			case WM_RBUTTONUP:
				w->has_input = true;
				w->mpos = ivec2(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
				w->mouse_listeners.call(Mouse_Right, false);
				return true;
			case WM_MBUTTONDOWN:
				w->has_input = true;
				w->mpos = ivec2(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
				w->mouse_listeners.call(Mouse_Middle, true);
				return true;
			case WM_MBUTTONUP:
				w->has_input = true;
				w->mpos = ivec2(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
				w->mouse_listeners.call(Mouse_Middle, false);
				return true;
			case WM_MOUSEMOVE:
				w->has_input = true;
				w->mpos = ivec2(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
				w->mouse_move_listeners.call(w->mpos);
				return true;
			case WM_MOUSEWHEEL:
				w->has_input = true;
				w->mouse_scroll_listeners.call(GET_Y_LPARAM(wParam) > 0 ? 1 : -1);
				return true;
			case WM_DESTROY:
				w->has_input = true;
				w->dead = true;
				return true;
			case WM_SIZE:
				w->has_input = true;
				w->size = uvec2(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
				w->resize_listeners.call(w->size);
				return true;
			case WM_SETFOCUS:
				w->focus_listeners.call(true);
				return true;
			case WM_KILLFOCUS:
				w->focus_listeners.call(false);
				return true;
			case WM_MOVE:
				w->has_input = true;
				w->pos = ivec2(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
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

	static std::vector<NativeWindowPtr> windows;

	NativeWindowPrivate::~NativeWindowPrivate()
	{
		destroy_listeners.call();
		std::erase_if(windows, [&](auto w) {
			return w == this;
		});
	}

	void NativeWindowPrivate::close()
	{
		DestroyWindow(hWnd);
		dead = true;
	}

	void NativeWindowPrivate::set_pos(const ivec2& _pos)
	{
		pos = _pos;
		SetWindowPos(hWnd, HWND_TOP, pos.x, pos.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
	}

	void NativeWindowPrivate::set_size(const uvec2& _size)
	{
		size = _size;
		SetWindowPos(hWnd, HWND_TOP, 0, 0, size.x, size.y, SWP_NOMOVE | SWP_NOZORDER);
	}

	ivec2 NativeWindowPrivate::global_to_local(const ivec2& p)
	{
		POINT pt;
		pt.x = p.x;
		pt.y = p.y;
		ScreenToClient(hWnd, &pt);
		return ivec2(pt.x, pt.y);
	}

	void NativeWindowPrivate::set_title(std::string_view _title)
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

	struct NativeWindowCreate : NativeWindow::Create
	{
		NativeWindowPtr operator()(std::string_view title, const uvec2& size, WindowStyleFlags style, NativeWindowPtr parent) override
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
				{
					std::unique_ptr<Bitmap> icon_image(Bitmap::create(Path::get(L"flame\\icon.png")));
					icon_image->swap_channel(0, 2);
					wcex.hIcon = CreateIcon(wcex.hInstance, icon_image->extent.x, icon_image->extent.y, 1, icon_image->bpp, nullptr, icon_image->data);
				}
				wcex.hCursor = NULL;
				wcex.hbrBackground = 0;
				wcex.lpszMenuName = 0;
				wcex.lpszClassName = L"flame_wnd";
				wcex.hIconSm = wcex.hIcon;
				RegisterClassExW(&wcex);

				initialized = true;
			}

			assert(!(style & WindowFullscreen) || (!(style & WindowFrame) && !(style & WindowResizable)));

			auto ret = new NativeWindowPrivate;
			ret->title = title;
			ret->style = style;

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
			ret->pos = ivec2(screen_size - final_size) / 2;
			ret->hWnd = CreateWindowExA(win32_ex_style, "flame_wnd", title.data(), win32_style,
				ret->pos.x, ret->pos.y, final_size.x, final_size.y, parent ? parent->hWnd : NULL, NULL, (HINSTANCE)get_hinst(), NULL);
			//assert(IsWindowUnicode(ret->hWnd));
			{
				RECT rect;
				GetClientRect(ret->hWnd, &rect);
				ret->size = uvec2(rect.right - rect.left, rect.bottom - rect.top);
			}

			SetWindowLongPtr(ret->hWnd, 0, (LONG_PTR)ret);

			ret->cursors[CursorAppStarting] = LoadCursorA(nullptr, IDC_APPSTARTING);
			ret->cursors[CursorArrow] = LoadCursorA(nullptr, IDC_ARROW);
			ret->cursors[CursorCross] = LoadCursorA(nullptr, IDC_CROSS);
			ret->cursors[CursorHand] = LoadCursorA(nullptr, IDC_HAND);
			ret->cursors[CursorHelp] = LoadCursorA(nullptr, IDC_HELP);
			ret->cursors[CursorIBeam] = LoadCursorA(nullptr, IDC_IBEAM);
			ret->cursors[CursorNo] = LoadCursorA(nullptr, IDC_NO);
			ret->cursors[CursorSizeAll] = LoadCursorA(nullptr, IDC_SIZEALL);
			ret->cursors[CursorSizeNESW] = LoadCursorA(nullptr, IDC_SIZENESW);
			ret->cursors[CursorSizeNS] = LoadCursorA(nullptr, IDC_SIZENS);
			ret->cursors[CursorSizeNWSE] = LoadCursorA(nullptr, IDC_SIZENWSE);
			ret->cursors[CursorSizeWE] = LoadCursorA(nullptr, IDC_SIZEWE);
			ret->cursors[CursorUpArrwo] = LoadCursorA(nullptr, IDC_UPARROW);
			ret->cursors[CursorWait] = LoadCursorA(nullptr, IDC_WAIT);

			ret->set_cursor(CursorArrow);

			windows.emplace_back(ret);
			return ret;
		}
	}NativeWindow_create;
	NativeWindow::Create& NativeWindow::create = NativeWindow_create;

	struct NativeWindowList : NativeWindow::List
	{
		const std::vector<NativeWindowPtr>& operator()() override
		{
			return windows;
		}
	}NativeWindow_list;
	NativeWindow::List& NativeWindow::list = NativeWindow_list;
}
