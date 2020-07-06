#include <flame/serialize.h>
#include "foundation_private.h"
#include "bitmap_private.h"

#ifdef FLAME_WINDOWS
#define NOMINMAX
#include <process.h>
#include <shellapi.h>
#include <ShlObj.h>
#include <Shlwapi.h>
#include <CommCtrl.h>
#include <thumbcache.h>
#include <ImageHlp.h>
#endif

namespace flame
{
	void* ListenerHubImnplPrivate::add(bool(*pf)(Capture& c), const Capture& capture, int pos)
	{
		if (pos == -1)
			pos = listeners.size();
		auto c = new Closure(pf, capture);
		listeners.emplace(listeners.begin() + pos, c);
		return c;
	}

	void ListenerHubImnplPrivate::remove(void* l)
	{
		for (auto it = listeners.begin(); it != listeners.end(); it++)
		{
			if (it->get() == l)
			{
				listeners.erase(it);
				return;
			}
		}
	}

	ListenerHubImpl* ListenerHubImpl::create()
	{
		return new ListenerHubImnplPrivate;
	}

	void* f_malloc(uint size)
	{
		return malloc(size);
	}

	void* f_realloc(void* p, uint size)
	{
		if (!p)
			return f_malloc(size);
		return realloc(p, size);
	}

	void f_free(void* p)
	{
		free(p);
	}

	Guid generate_guid()
	{
		Guid ret;
		CoCreateGuid((GUID*)&ret);
		return ret;
	}

	static std::filesystem::path engine_path = getenv("FLAME_PATH");

	void set_engine_path(const wchar_t* p)
	{
		engine_path = p;
	}

	void get_engine_path(wchar_t* dst)
	{
		auto path = engine_path.wstring();
		wcsncpy(dst, path.c_str(), path.size());
		dst[path.size()] = 0;
	}

	static void(*file_callback)(Capture&, const wchar_t*);
	static Capture file_callback_capture;

	void set_file_callback(void(*callback)(Capture& c, const wchar_t* filename), const Capture& capture)
	{
		file_callback = callback;
		file_callback_capture = capture;
	}

	void report_used_file(const wchar_t* filename)
	{
		if (file_callback)
			file_callback(file_callback_capture, filename);
	}

	void get_app_path(wchar_t* dst, bool has_name)
	{
		GetModuleFileNameW(nullptr, dst, 260);
		if (!has_name)
		{
			auto path = std::filesystem::path(dst).parent_path().wstring();
			wcsncpy(dst, path.c_str(), path.size());
			dst[path.size()] = 0;
		}
	}

	void *get_hinst()
	{
		return GetModuleHandle(nullptr);
	}

	Vec2u get_screen_size()
	{
		Vec2u ret;
		ret.x() = GetSystemMetrics(SM_CXSCREEN);
		ret.y() = GetSystemMetrics(SM_CYSCREEN);
		return ret;
	}

	void* create_event(bool signaled, bool manual)
	{
		return CreateEvent(NULL, manual, signaled, NULL);
	}

	void set_event(void* ev)
	{
		SetEvent(ev);
	}

	void reset_event(void* ev)
	{
		ResetEvent(ev);
	}

	bool wait_event(void* ev, int timeout)
	{
		return WaitForSingleObject(ev, timeout < 0 ? INFINITE : timeout) == WAIT_OBJECT_0;
	}

	void destroy_event(void* ev)
	{
		CloseHandle((HANDLE)ev);
	}

	static PIMAGE_SECTION_HEADER get_enclosing_section_header(DWORD rva, PIMAGE_NT_HEADERS64 pNTHeader)
	{
		auto section = IMAGE_FIRST_SECTION(pNTHeader);

		for (auto i = 0; i < pNTHeader->FileHeader.NumberOfSections; i++, section++)
		{
			auto size = section->Misc.VirtualSize;
			if (0 == size)
				size = section->SizeOfRawData;

			if (rva >= section->VirtualAddress && rva < section->VirtualAddress + size)
				return section;
		}

		return 0;
	}

	static LPVOID get_ptr_from_rva(DWORD rva, PIMAGE_NT_HEADERS64 pNTHeader, PBYTE imageBase)
	{
		auto pSectionHdr = get_enclosing_section_header(rva, pNTHeader);
		if (!pSectionHdr)
			return 0;

		auto delta = (INT)(pSectionHdr->VirtualAddress - pSectionHdr->PointerToRawData);
		return (PVOID)(imageBase + rva - delta);
	}

	void get_library_dependencies(const wchar_t* filename, void (*callback)(Capture& c, const char* filename), const Capture& capture)
	{
		auto path = std::filesystem::path(filename);
		auto parent_path = path.parent_path();
		std::deque<std::filesystem::path> remains;
		std::unordered_map<std::string, uint> toucheds;
		remains.push_back(path.filename());
		while (!remains.empty())
		{
			auto lib_name = remains.front();
			remains.pop_front();
			if (!std::filesystem::exists(parent_path / lib_name))
				continue;
			auto image = ImageLoad(lib_name.string().c_str(), parent_path.string().c_str());
			if (image->FileHeader->OptionalHeader.NumberOfRvaAndSizes >= 2)
			{
				auto importDesc = (PIMAGE_IMPORT_DESCRIPTOR)get_ptr_from_rva(
					image->FileHeader->OptionalHeader.DataDirectory[1].VirtualAddress,
					image->FileHeader, image->MappedAddress);
				while (true)
				{
					if (importDesc->TimeDateStamp == 0 && importDesc->Name == 0)
						break;

					auto pstr = (char*)get_ptr_from_rva(importDesc->Name, image->FileHeader, image->MappedAddress);
					if (toucheds.find(pstr) == toucheds.end())
					{
						remains.push_back(pstr);
						toucheds.emplace(pstr, 0);
						callback((Capture&)capture, pstr);
					}

					importDesc++;
				}
			}
			ImageUnload(image);
		}
	}

	void get_clipboard(wchar_t* (*str_allocator)(Capture& c, uint size), const Capture& capture)
	{
		OpenClipboard(NULL);
		auto hMemory = GetClipboardData(CF_UNICODETEXT);
		auto size = GlobalSize(hMemory) / sizeof(wchar_t) - 1;
		auto dst = str_allocator((Capture&)capture, size);
		wcsncpy(dst, (wchar_t*)GlobalLock(hMemory), size);
		dst[size] = 0;
		GlobalUnlock(hMemory);
		CloseClipboard();
	}

	void set_clipboard(const wchar_t* s)
	{
		auto size = sizeof(wchar_t) * (lstrlenW(s) + 1);
		auto hGlobalMemory = GlobalAlloc(GHND, size);
		memcpy(GlobalLock(hGlobalMemory), s, size);
		GlobalUnlock(hGlobalMemory);
		OpenClipboard(NULL);
		EmptyClipboard();
		SetClipboardData(CF_UNICODETEXT, hGlobalMemory);
		CloseClipboard();
	}

	void get_thumbnail(uint width, const wchar_t* _filename, uint* out_width, uint* out_height, char** out_data)
	{
		std::filesystem::path path(_filename);
		path.make_preferred();
		auto filename = path.wstring();

		HRESULT hr;

		IShellFolder* desktop_folder, *shell_folder;
		SHGetDesktopFolder(&desktop_folder);

		LPITEMIDLIST pidl;
		hr = desktop_folder->ParseDisplayName(NULL, NULL, (wchar_t*)filename.c_str(), NULL, &pidl, NULL);
		SHBindToParent(pidl, IID_PPV_ARGS(&shell_folder), NULL);
		auto pidl_child = ILFindLastID(pidl);

		IThumbnailProvider* thumbnail_provider;
		hr = shell_folder->GetUIObjectOf(NULL, 1, (LPCITEMIDLIST*)&pidl_child, IID_IThumbnailProvider, NULL, (void**)& thumbnail_provider);
		HBITMAP hbmp;
		WTS_ALPHATYPE alpha_type;
		if (SUCCEEDED(thumbnail_provider->GetThumbnail(width, &hbmp, &alpha_type)))
		{
			BITMAP bmp;
			GetObject(hbmp, sizeof(bmp), &bmp);
			*out_width = bmp.bmWidth;
			*out_height = bmp.bmHeight;
			*out_data = new char[bmp.bmWidth * bmp.bmHeight * 4];
			GetBitmapBits(hbmp, bmp.bmWidthBytes * bmp.bmHeight, *out_data);
			DeleteObject(hbmp);
		}

		thumbnail_provider->Release();

		desktop_folder->Release();
		shell_folder->Release();
	}

	static Key vk_code_to_key(int vkCode)
	{
#ifdef FLAME_WINDOWS
		switch (vkCode)
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
			return KeyNull;
		}
#endif
		return KeyNull;
	}

	int key_to_vk_code(Key key)
	{
#ifdef FLAME_WINDOWS
		switch (key)
		{
		case Key_Backspace:
			return VK_BACK;
		case Key_Tab:
			return VK_TAB;
		case Key_Enter:
			return VK_RETURN;
		case Key_Shift:
			return VK_SHIFT;
		case Key_Ctrl:
			return VK_CONTROL;
		case Key_Alt:
			return VK_MENU;
		case Key_Pause:
			return VK_PAUSE;
		case Key_CapsLock:
			return VK_CAPITAL;
		case Key_Esc:
			return VK_ESCAPE;
		case Key_Space:
			return VK_SPACE;
		case Key_PgUp:
			return VK_PRIOR;
		case Key_PgDn:
			return VK_NEXT;
		case Key_End:
			return VK_END;
		case Key_Home:
			return VK_HOME;
		case Key_Left:
			return VK_LEFT;
		case Key_Up:
			return VK_UP;
		case Key_Right:
			return VK_RIGHT;
		case Key_Down:
			return VK_DOWN;
		case Key_PrtSc:
			return VK_SNAPSHOT;
		case Key_Ins:
			return VK_INSERT;
		case Key_Del:
			return VK_DELETE;
		case Key_0:
			return '0';
		case Key_1:
			return '1';
		case Key_2:
			return '2';
		case Key_3:
			return '3';
		case Key_4:
			return '4';
		case Key_5:
			return '5';
		case Key_6:
			return '6';
		case Key_7:
			return '7';
		case Key_8:
			return '8';
		case Key_9:
			return '9';
		case Key_A:
			return 'A';
		case Key_B:
			return 'B';
		case Key_C:
			return 'C';
		case Key_D:
			return 'D';
		case Key_E:
			return 'E';
		case Key_F:
			return 'F';
		case Key_G:
			return 'G';
		case Key_H:
			return 'H';
		case Key_I:
			return 'I';
		case Key_J:
			return 'J';
		case Key_K:
			return 'K';
		case Key_L:
			return 'L';
		case Key_M:
			return 'M';
		case Key_N:
			return 'N';
		case Key_O:
			return 'O';
		case Key_P:
			return 'P';
		case Key_Q:
			return 'Q';
		case Key_R:
			return 'R';
		case Key_S:
			return 'S';
		case Key_T:
			return 'T';
		case Key_U:
			return 'U';
		case Key_V:
			return 'V';
		case Key_W:
			return 'W';
		case Key_X:
			return 'X';
		case Key_Y:
			return 'Y';
		case Key_Z:
			return 'Z';
		case Key_Numpad0:
			return VK_NUMPAD0;
		case Key_Numpad1:
			return VK_NUMPAD1;
		case Key_Numpad2:
			return VK_NUMPAD2;
		case Key_Numpad3:
			return VK_NUMPAD3;
		case Key_Numpad4:
			return VK_NUMPAD4;
		case Key_Numpad5:
			return VK_NUMPAD5;
		case Key_Numpad6:
			return VK_NUMPAD6;
		case Key_Numpad7:
			return VK_NUMPAD7;
		case Key_Numpad8:
			return VK_NUMPAD8;
		case Key_Numpad9:
			return VK_NUMPAD9;
		case Key_Add:
			return VK_ADD;
		case Key_Subtract:
			return VK_SUBTRACT;
		case Key_Multiply:
			return VK_MULTIPLY;
		case Key_Divide:
			return VK_DIVIDE;
		case Key_Separator:
			return VK_SEPARATOR;
		case Key_Decimal:
			return VK_DECIMAL;
		case Key_F1:
			return VK_F1;
		case Key_F2:
			return VK_F2;
		case Key_F3:
			return VK_F3;
		case Key_F4:
			return VK_F4;
		case Key_F5:
			return VK_F5;
		case Key_F6:
			return VK_F6;
		case Key_F7:
			return VK_F7;
		case Key_F8:
			return VK_F8;
		case Key_F9:
			return VK_F9;
		case Key_F10:
			return VK_F10;
		case Key_F11:
			return VK_F11;
		case Key_F12:
			return VK_F12;
		case Key_NumLock:
			return VK_NUMLOCK;
		case Key_ScrollLock:
			return VK_SCROLL;
		default:
			return KeyNull;
		}
#endif
		return KeyNull;
	}

	struct GlobalKeyListener
	{
		Key key;
		bool ctrl;
		bool shift;
		bool alt;
		std::unique_ptr<Closure<void(Capture& c, KeyStateFlags action)>> callback;
	};

	static HHOOK global_key_hook = 0;
	static std::vector<std::unique_ptr<GlobalKeyListener>> global_key_listeners;

	LRESULT CALLBACK global_key_callback(int nCode, WPARAM wParam, LPARAM lParam)
	{
		auto kbhook = (KBDLLHOOKSTRUCT*)lParam;

		auto key = vk_code_to_key(kbhook->vkCode);

		for (auto& l : global_key_listeners)
		{
			if (l->key != key)
				continue;
			if (l->ctrl && !(GetAsyncKeyState(VK_LCONTROL) || GetAsyncKeyState(VK_RCONTROL)))
				continue;
			if (l->shift && !(GetAsyncKeyState(VK_LSHIFT) || GetAsyncKeyState(VK_RSHIFT)))
				continue;
			if (l->alt && !(GetAsyncKeyState(VK_LMENU) || GetAsyncKeyState(VK_RMENU)))
				continue;
			auto action = KeyStateNull;
			if (wParam == WM_KEYDOWN)
				action = KeyStateDown;
			else if (wParam == WM_KEYUP)
				action = KeyStateUp;
			l->callback->call(action);
		}

		return CallNextHookEx(global_key_hook, nCode, wParam, lParam);
	}

	void* add_global_key_listener(Key key, bool ctrl, bool shift, bool alt, void (*callback)(Capture& c, KeyStateFlags action), const Capture& capture)
	{
		auto l = new GlobalKeyListener;
		l->key = key;
		l->ctrl = ctrl;
		l->shift = shift;
		l->alt = alt;
		l->callback.reset(new Closure(callback, capture));

		global_key_listeners.emplace_back(l);

		if (!global_key_hook)
			global_key_hook = SetWindowsHookEx(WH_KEYBOARD_LL, global_key_callback, (HINSTANCE)get_hinst(), 0);

		return l;
	}

	void remove_global_key_listener(void *handle)
	{
		for (auto it = global_key_listeners.begin(); it != global_key_listeners.end(); it++)
		{
			if ((*it).get() == handle)
			{
				global_key_listeners.erase(it);
				break;
			}
		}

		if (global_key_listeners.empty())
		{
			if (global_key_hook)
			{
				UnhookWindowsHookEx(global_key_hook);
				global_key_hook = 0;
			}
		}
	}

	void send_global_key_event(KeyState action, Key key)
	{
		auto flags = -1;
		switch (action)
		{
		case KeyStateDown:
			flags = 0;
			break;
		case KeyStateUp:
			flags = KEYEVENTF_KEYUP;
			break;
		}
		if (flags != -1)
			keybd_event(key_to_vk_code(key), 0, flags, 0);
	}

	void send_global_mouse_event(KeyState action, MouseKey key)
	{
		auto flags = -1;
		switch (key)
		{
		case Mouse_Left:
			switch (action)
			{
			case KeyStateDown:
				flags = MOUSEEVENTF_LEFTDOWN;
				break;
			case KeyStateUp:
				flags = MOUSEEVENTF_LEFTUP;
				break;
			}
			break;
		case Mouse_Right:
			switch (action)
			{
			case KeyStateDown:
				flags = MOUSEEVENTF_RIGHTDOWN;
				break;
			case KeyStateUp:
				flags = MOUSEEVENTF_RIGHTUP;
				break;
			}
			break;
		case Mouse_Middle:
			switch (action)
			{
			case KeyStateDown:
				flags = MOUSEEVENTF_MIDDLEDOWN;
				break;
			case KeyStateUp:
				flags = MOUSEEVENTF_MIDDLEUP;
				break;
			}
			break;
		}
		if (flags != -1)
			mouse_event(flags, 0, 0, 0, NULL);
	}

	void shell_exec(const wchar_t* filename, wchar_t* parameters, bool wait, bool show)
	{
		SHELLEXECUTEINFOW info = {};
		info.cbSize = sizeof(SHELLEXECUTEINFOW);
		info.fMask = SEE_MASK_NOCLOSEPROCESS;
		info.lpVerb = L"open";
		info.lpFile = filename;
		info.nShow = show ? SW_SHOW : SW_HIDE;
		info.lpParameters = parameters;
		ShellExecuteExW(&info);
		if (wait)
			WaitForSingleObject(info.hProcess, INFINITE);
	}

	void exec(const wchar_t* filename, wchar_t* parameters, char* (*str_allocator)(Capture& c, uint size), const Capture& capture)
	{
		bool ok;
		HANDLE hChildStd_OUT_Rd = NULL;
		HANDLE hChildStd_OUT_Wr = NULL;
		if (str_allocator)
		{
			SECURITY_ATTRIBUTES saAttr;
			saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
			saAttr.bInheritHandle = TRUE;
			saAttr.lpSecurityDescriptor = NULL;
			ok = CreatePipe(&hChildStd_OUT_Rd, &hChildStd_OUT_Wr, &saAttr, 0);
			assert(ok);
			ok = SetHandleInformation(hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0);
			assert(ok);
		}
		else
			hChildStd_OUT_Wr = GetStdHandle(STD_OUTPUT_HANDLE);

		STARTUPINFOW start_info = {};
		start_info.cb = sizeof(STARTUPINFOW);
		start_info.hStdError = hChildStd_OUT_Wr;
		start_info.hStdOutput = hChildStd_OUT_Wr;
		start_info.dwFlags |= STARTF_USESTDHANDLES;
		PROCESS_INFORMATION proc_info = {};
		if (!CreateProcessW(filename, parameters, NULL, NULL, TRUE, 0, NULL, NULL, &start_info, &proc_info))
		{
			auto e = GetLastError();
			assert(0);
		}

		WaitForSingleObject(proc_info.hProcess, INFINITE);

		CloseHandle(proc_info.hProcess);
		CloseHandle(proc_info.hThread);

		if (str_allocator)
		{
			DWORD size;
			PeekNamedPipe(hChildStd_OUT_Rd, NULL, NULL, NULL, &size, NULL);
			auto dst = str_allocator((Capture&)capture, size);
			PeekNamedPipe(hChildStd_OUT_Rd, (void*)dst, size, NULL, NULL, NULL);
		}
	}

	void debug_break()
	{
#ifdef _DEBUG
		DebugBreak();
#endif
	}

	void get_call_frames(void** (*array_allocator)(Capture& c, uint size), const Capture& capture)
	{
		void* buf[64];
		auto n = CaptureStackBackTrace(0, array_size(buf), buf, nullptr);
		auto dst = array_allocator((Capture&)capture, n);
		memcpy(dst, buf, sizeof(void*) * n);
	}

	void get_call_frames_infos(uint frames_count, void** frames, StackFrameInfo* dst)
	{
		auto process = GetCurrentProcess();
		SymInitialize(process, nullptr, true); 

		const auto MaxFunctionNameLength = 1024;

		auto symbol = (SYMBOL_INFO*)new char[sizeof(SYMBOL_INFO) + MaxFunctionNameLength - 1];
		symbol->MaxNameLen = MaxFunctionNameLength;
		symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
		auto line = (IMAGEHLP_LINE64*)new char[sizeof(IMAGEHLP_LINE64)];
		line->SizeOfStruct = sizeof(IMAGEHLP_LINE64);
		DWORD displacement;
		for (auto i = 0; i < frames_count; i++)
		{
			auto frame = (DWORD64)frames[i];
			SymFromAddr(process, frame, nullptr, symbol);
			if (SymGetLineFromAddr64(process, (DWORD64)frame, &displacement, line))
			{
				auto& info = dst[i];
				strcpy(info.file, line->FileName);
				info.line = line->LineNumber;
				strcpy(info.function, symbol->Name);
			}
		}
		delete[] line;
		delete[] symbol;
	}

	void do_file_watch(void* event_end, bool all_changes, const std::wstring& path, void (*callback)(Capture& c, FileChangeType type, const wchar_t* filename), Capture& capture)
	{
		auto dir_handle = CreateFileW(path.c_str(), GENERIC_READ | GENERIC_WRITE | FILE_LIST_DIRECTORY, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED | FILE_FLAG_BACKUP_SEMANTICS, NULL);
		assert(dir_handle != INVALID_HANDLE_VALUE);

		BYTE notify_buf[1024];

		OVERLAPPED overlapped;
		auto event_changed = create_event(false);

		auto flags = FILE_NOTIFY_CHANGE_LAST_WRITE;
		if (all_changes)
			flags |= FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_CREATION;

		while (true)
		{
			bool ok;

			ZeroMemory(&overlapped, sizeof(OVERLAPPED));
			overlapped.hEvent = event_changed;

			ok = ReadDirectoryChangesW(dir_handle, notify_buf, sizeof(notify_buf), true, flags, NULL, &overlapped, NULL);
			assert(ok);

			if (event_end)
			{
				HANDLE events[] = {
					overlapped.hEvent,
					event_end
				};

				if (WaitForMultipleObjects(2, events, false, INFINITE) - WAIT_OBJECT_0 == 1)
					break;
			}
			else
				WaitForSingleObject(overlapped.hEvent, INFINITE);

			DWORD ret_bytes;
			ok = GetOverlappedResult(dir_handle, &overlapped, &ret_bytes, false);
			assert(ok);

			auto base = 0;
			auto p = (FILE_NOTIFY_INFORMATION*)notify_buf;
			p->FileName[p->FileNameLength / 2] = 0;
			while (true)
			{
				FileChangeType type;
				switch (p->Action)
				{
				case 0x1:
					type = FileAdded;
					break;
				case 0x2:
					type = FileRemoved;
					break;
				case 0x3:
					type = FileModified;
					break;
				case 0x4:
					type = FileRenamed;
					break;
				case 0x5:
					type = FileRenamed;
					break;
				}

				if (all_changes || type == FileModified)
					callback(capture, type, !path.empty() ? (path + L"\\" + p->FileName).c_str() : p->FileName);

				if (p->NextEntryOffset <= 0)
					break;
				base += p->NextEntryOffset;
				p = (FILE_NOTIFY_INFORMATION*)(notify_buf + base);
			}
		}

		f_free(capture._data);

		destroy_event(event_changed);
		destroy_event(dir_handle);

		if (event_end)
			destroy_event(event_end);
	}

	void* add_file_watcher(const wchar_t* _path, void (*callback)(Capture& c, FileChangeType type, const wchar_t* filename), const Capture& capture, bool all_changes, bool sync)
	{
		std::filesystem::path path = _path;

		if (!sync)
		{
			auto ev = create_event(false);

			std::thread([=]() {
				do_file_watch(ev, all_changes, path, callback, (Capture&)capture);
			}).detach();

			return ev;
		}
		else
		{
			do_file_watch(nullptr, all_changes, path, callback, (Capture&)capture);

			return nullptr;
		}

		return nullptr;
	}

	const auto all_workers = 3;
	static std::mutex mtx;
	static std::condition_variable cv;
	static auto workers = all_workers;

	static std::vector<std::unique_ptr<Closure<void(Capture&)>>> works;

	static void try_distribute_work()
	{
		if (!works.empty() && workers > 0)
		{
			mtx.lock();

			workers--;
			auto w = works.front().release();
			works.erase(works.begin());

			std::thread([=]() {
				w->call();
				delete w;
				mtx.lock();
				workers++;
				cv.notify_one();
				mtx.unlock();
				try_distribute_work();
			}).detach();

			mtx.unlock();
		}
	}

	void add_work(void (*function)(Capture& c), const Capture& capture)
	{
		mtx.lock();
		works.emplace_back(new Closure(function, capture));
		mtx.unlock();

		try_distribute_work();
	}

	void clear_all_works()
	{
		mtx.lock();
		works.clear();
		mtx.unlock();

		wait_all_works();
	}

	void wait_all_works()
	{
		std::unique_lock<std::mutex> lock(mtx);

		while (workers != all_workers)
			cv.wait(lock);
	}

	enum KeyEventType
	{
		KeyEventNull,
		KeyEventDown,
		KeyEventUp
	};

#ifdef FLAME_WINDOWS
	static LRESULT CALLBACK wnd_proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		auto w = (WindowPrivate*)GetWindowLongPtr(hWnd, 0);
		if (w)
			w->_wnd_proc(message, wParam, lParam);

		return DefWindowProc(hWnd, message, wParam, lParam);
	}
#endif

#ifdef FLAME_WINDOWS
	WindowPrivate::WindowPrivate(const std::string& title, const Vec2u& size, uint style, WindowPrivate* parent)
	{
		static bool initialized = false;
		if (!initialized)
		{
			WNDCLASSEXA wcex;
			wcex.cbSize = sizeof(WNDCLASSEXA);
			wcex.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
			wcex.lpfnWndProc = wnd_proc;
			wcex.cbClsExtra = 0;
			wcex.cbWndExtra = sizeof(void*);
			wcex.hInstance = (HINSTANCE)get_hinst();
			wcex.hIcon = 0;
			auto icon_fn = std::filesystem::path(getenv("FLAME_PATH")) / L"art\\ico.png";
			if (std::filesystem::exists(icon_fn))
			{
				report_used_file(icon_fn.c_str());
				auto icon_image = BitmapPrivate::_create(icon_fn);
				icon_image->_swap_channel(0, 2);
				wcex.hIcon = CreateIcon(wcex.hInstance, icon_image->get_width(), icon_image->get_height(), 1,
					icon_image->get_channel() * icon_image->get_byte_per_channel() * 8, nullptr, icon_image->get_data());
				icon_image->release();
			}
			wcex.hCursor = NULL;
			wcex.hbrBackground = 0;
			wcex.lpszMenuName = 0;
			wcex.lpszClassName = "flame_wnd";
			wcex.hIconSm = wcex.hIcon;
			RegisterClassExA(&wcex);

			initialized = true;
		}

		_title = title;

		_size = size;
		_style = style;

		assert(!(style & WindowFullscreen) || (!(style & WindowFrame) && !(style & WindowResizable)));

		Vec2u final_size;
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
			RECT rect = { 0, 0, size.x(), size.y() };
			AdjustWindowRect(&rect, win32_style, false);
			final_size = Vec2u(rect.right - rect.left, rect.bottom - rect.top);
		}
		_pos.x() = (screen_size.x() - final_size.x()) / 2;
		_pos.y() = (screen_size.y() - final_size.y()) / 2;
		_hWnd = CreateWindowEx(win32_ex_style, "flame_wnd", title.c_str(), win32_style,
			_pos.x(), _pos.y(), final_size.x(), final_size.y(), parent ? parent->_hWnd : NULL, NULL, (HINSTANCE)get_hinst(), NULL);
		{
			RECT rect;
			GetClientRect(_hWnd, &rect);
			_size = Vec2u(rect.right - rect.left, rect.bottom - rect.top);
		}

		SetWindowLongPtr(_hWnd, 0, (LONG_PTR)this);

		for (auto i = 0; i < Cursor_Count; i++)
		{
			switch ((CursorType)i)
			{
			case CursorAppStarting:
				_cursors[i] = LoadCursorA(nullptr, IDC_APPSTARTING);
				break;
			case CursorArrow:
				_cursors[i] = LoadCursorA(nullptr, IDC_ARROW);
				break;
			case CursorCross:
				_cursors[i] = LoadCursorA(nullptr, IDC_CROSS);
				break;
			case CursorHand:
				_cursors[i] = LoadCursorA(nullptr, IDC_HAND);
				break;
			case CursorHelp:
				_cursors[i] = LoadCursorA(nullptr, IDC_HELP);
				break;
			case CursorIBeam:
				_cursors[i] = LoadCursorA(nullptr, IDC_IBEAM);
				break;
			case CursorNo:
				_cursors[i] = LoadCursorA(nullptr, IDC_NO);
				break;
			case CursorSizeAll:
				_cursors[i] = LoadCursorA(nullptr, IDC_SIZEALL);
				break;
			case CursorSizeNESW:
				_cursors[i] = LoadCursorA(nullptr, IDC_SIZENESW);
				break;
			case CursorSizeNS:
				_cursors[i] = LoadCursorA(nullptr, IDC_SIZENS);
				break;
			case CursorSizeNWSE:
				_cursors[i] = LoadCursorA(nullptr, IDC_SIZENWSE);
				break;
			case CursorSizeWE:
				_cursors[i] = LoadCursorA(nullptr, IDC_SIZEWE);
				break;
			case CursorUpArrwo:
				_cursors[i] = LoadCursorA(nullptr, IDC_UPARROW);
				break;
			case CursorWait:
				_cursors[i] = LoadCursorA(nullptr, IDC_WAIT);
				break;
			}
		}

		_pending_size = size;

		set_cursor(CursorArrow);
		_looper->windows.emplace_back(this);
	}
#elif FLAME_ANDROID
	WindowPrivate::WindowPrivate(android_app* android_state) :
		android_state(android_state)
	{
	}
#endif
	WindowPrivate::~WindowPrivate()
	{
		for (auto& l : _destroy_listeners)
			l->call();
	}

	void WindowPrivate::_wnd_proc(UINT message, WPARAM wParam, LPARAM lParam)
	{
		auto resize = [=]() {
			if (_size != _pending_size)
			{
				_size = _pending_size;
				for (auto& l : _resize_listeners)
					l->call(_size);
			}
		};

		switch (message)
		{
		case WM_KEYDOWN:
		{
			auto v = vk_code_to_key(wParam);
			if (v > 0)
			{
				for (auto& l : _key_listeners)
					l->call(KeyStateDown, v);
			}
		}
			break;
		case WM_KEYUP:
		{
			auto v = vk_code_to_key(wParam);
			if (v > 0)
			{
				for (auto& l : _key_listeners)
					l->call(KeyStateUp, v);
			}
		}
			break;
		case WM_CHAR:
			for (auto& l : _key_listeners)
				l->call(KeyStateNull, (Key)wParam);
			break;
		case WM_LBUTTONDOWN:
		{
			SetCapture(_hWnd);
			auto pos = Vec2i((int)LOWORD(lParam), (int)HIWORD(lParam));
			for (auto& l : _mouse_listeners)
				l->call(KeyStateDown, Mouse_Left, pos);
		}
			break;
		case WM_LBUTTONUP:
		{
			ReleaseCapture();
			auto pos = Vec2i((int)LOWORD(lParam), (int)HIWORD(lParam));
			for (auto& l : _mouse_listeners)
				l->call(KeyStateUp, Mouse_Left, pos);
		}
			break;
		case WM_MBUTTONDOWN:
		{
			auto pos = Vec2i((int)LOWORD(lParam), (int)HIWORD(lParam));
			for (auto& l : _mouse_listeners)
				l->call(KeyStateDown, Mouse_Middle, pos);
		}
			break;
		case WM_MBUTTONUP:
		{
			auto pos = Vec2i((int)LOWORD(lParam), (int)HIWORD(lParam));
			for (auto& l : _mouse_listeners)
				l->call(KeyStateUp, Mouse_Middle, pos);
		}
			break;
		case WM_RBUTTONDOWN:
		{
			auto pos = Vec2i((int)LOWORD(lParam), (int)HIWORD(lParam));
			for (auto& l : _mouse_listeners)
				l->call(KeyStateDown, Mouse_Right, pos);
		}
			break;
		case WM_RBUTTONUP:
		{
			auto pos = Vec2i((int)LOWORD(lParam), (int)HIWORD(lParam));
			for (auto& l : _mouse_listeners)
				l->call(KeyStateUp, Mouse_Right, pos);
		}
			break;
		case WM_MOUSEMOVE:
		{
			auto pos = Vec2i((int)LOWORD(lParam), (int)HIWORD(lParam));
			for (auto& l : _mouse_listeners)
				l->call(KeyStateNull, Mouse_Null, pos);
		}
			break;
		case WM_MOUSEWHEEL:
		{
			auto v = Vec2i((int)HIWORD(wParam) > 0 ? 1 : -1, 0);
			for (auto& l : _mouse_listeners)
				l->call(KeyStateNull, Mouse_Middle, v);
		}
			break;
		case WM_DESTROY:
			_dead = true;
		case WM_ENTERSIZEMOVE:
			_sizing = true;
			SetTimer(_hWnd, 0, 100, NULL);
			break;
		case WM_EXITSIZEMOVE:
			_sizing = false;
			KillTimer(_hWnd, 0);
			resize();
			break;
		case WM_TIMER:
			if (wParam == 0)
				resize();
			_looper->_one_frame();
			break;
		case WM_SIZE:
			_pending_size = Vec2u((int)LOWORD(lParam), (int)HIWORD(lParam));
			if (!_sizing)
				resize();
			break;
		case WM_SETCURSOR:
			SetCursor(_cursors[_cursor_type]);
			break;
		}
	}

	void* WindowPrivate::_get_native() 
	{
#ifdef FLAME_WINDOWS
		return _hWnd;
#elif FLAME_ANDROID
		return android_state;
#endif
	}

	void WindowPrivate::_set_pos(const Vec2i& pos)
	{
		 _pos = pos;
		SetWindowPos(_hWnd, HWND_TOP, pos.x(), pos.y(), 0, 0, SWP_NOSIZE | SWP_NOZORDER);
	}

	void WindowPrivate::_set_size(const Vec2u& size)
	{
	}

	void WindowPrivate::_set_title(const std::string& title)
	{
		_title = title;
		SetWindowTextA(_hWnd, title.c_str());
	}

	void WindowPrivate::_set_cursor(CursorType type) 
	{
#ifdef FLAME_WINDOWS
		if (type == _cursor_type)
			return;

		if (_cursor_type == CursorNone)
			ShowCursor(true);
		if (type == CursorNone)
			ShowCursor(false);

		_cursor_type = type;
#endif
	}

	void WindowPrivate::_close()
	{
		DestroyWindow(_hWnd);
		_dead = true;
	}

	void* WindowPrivate::add_key_listener(void (*callback)(Capture& c, KeyStateFlags action, int value), const Capture& capture)
	{
		auto c = new Closure(callback, capture);
		_key_listeners.emplace_back(c);
		return c;
	}

	void WindowPrivate::remove_key_listener(void* lis)
	{
		erase_if(_key_listeners, (decltype(_key_listeners[0].get()))lis);
	}

	void* WindowPrivate::add_mouse_listener(void (*callback)(Capture& c, KeyStateFlags action, MouseKey key, const Vec2i& pos), const Capture& capture)
	{
		auto c = new Closure(callback, capture);
		_mouse_listeners.emplace_back(c);
		return c;
	}

	void WindowPrivate::remove_mouse_listener(void* lis)
	{
		erase_if(_mouse_listeners, (decltype(_mouse_listeners[0].get()))lis);
	}

	void* WindowPrivate::add_resize_listener(void (*callback)(Capture& c, const Vec2u& size), const Capture& capture)
	{
		auto c = new Closure(callback, capture);
		_resize_listeners.emplace_back(c);
		return c;
	}

	void WindowPrivate::remove_resize_listener(void* lis)
	{
		erase_if(_resize_listeners, (decltype(_resize_listeners[0].get()))lis);
	}

	void* WindowPrivate::add_destroy_listener(void (*callback)(Capture& c), const Capture& capture)
	{
		auto c = new Closure(callback, capture);
		_destroy_listeners.emplace_back(c);
		return c;
	}

	void WindowPrivate::remove_destroy_listener(void* lis)
	{
		erase_if(_destroy_listeners, (decltype(_destroy_listeners[0].get()))lis);
	}

	Window* Window::create(const char* title, const Vec2u& size, WindowStyleFlags style, Window* parent)
	{
		return new WindowPrivate(title, size, style, (WindowPrivate*)parent);
	}

#ifdef FLAME_ANDROID
	static int32_t android_handle_input(android_app* state, AInputEvent* event)
	{
		if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION)
			return 1;
		return 0;
	}

	void (*created_callback)();

	static void android_handle_cmd(android_app* state, int32_t cmd)
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

	Window* Window::create(Application* app, void* android_state, void(*callback)())
	{
		auto android_state = reinterpret_cast<android_app*>(android_state);
		auto w = new WindowPrivate(android_state);
		w->app = reinterpret_cast<ApplicationPrivate*>(app);
		((ApplicationPrivate*)app)->windows.push_back(w);

		android_state->userData = w;
		android_state->onAppCmd = android_handle_cmd;
		android_state->onInputEvent = android_handle_input;
		created_callback = callback;

		return w;
	}
#endif

	int LooperPrivate::_loop(void (*frame_callback)(Capture& c, float delta_time), const Capture& capture)
	{
		if (!frame_callback)
		{
			for (;;)
			{
				MSG msg;
				while (GetMessage(&msg, NULL, 0, 0))
				{
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
			}
		}

		if (windows.empty())
			return 1;

		_frame_callback = frame_callback;
		_frame_capture = capture;

		_last_time = get_now_ns();
		_frame = 0;

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
					source->process(w->android_state, source);

				if (w->android_state->destroyRequested != 0)
					w->destroy_event = true;
			}
#endif
			if (!_one_frame())
				break;
		}
	}

	struct Event
	{
		uint id;
		CountDown interval;
		CountDown rest;
		void(*callback)(Capture& c);
		Capture capture;

		~Event()
		{
			f_free(capture._data);
		}
	};

	static std::list<std::unique_ptr<Event>> events;
	static std::recursive_mutex event_mtx;

	bool LooperPrivate::_one_frame()
	{
		for (auto it = windows.begin(); it != windows.end(); )
		{
			auto w = it->get();

			if (w->_dead)
				it = windows.erase(it);
			else
				it++;
		}

		if (windows.empty())
		{
			f_free(_frame_capture._data);
			return false;
		}

		_frame_callback(_frame_capture, _delta_time);

		_frame++;
		auto et = _last_time;
		_last_time = get_now_ns();
		et = _last_time - et;
		_delta_time = et / 1000000000.f;
		_total_time += _delta_time;

		return true;
	}

	void* LooperPrivate::_add_event(void (*callback)(Capture& c), const Capture& capture, CountDown interval, uint id)
	{
		event_mtx.lock();
		auto e = new Event;
		e->id = id;
		e->interval = interval;
		e->rest = interval;
		e->callback = callback;
		e->capture = capture;
		events.emplace_back(e);
		event_mtx.unlock();
		return e;
	}

	void LooperPrivate::_reset_event(void* _ev)
	{
		auto ev = (Event*)_ev;
		ev->rest = ev->interval;
	}

	void LooperPrivate::_remove_event(void* ev)
	{
		std::lock_guard<std::recursive_mutex> lock(event_mtx);
		for (auto it = events.begin(); it != events.end(); it++)
		{
			if ((*it).get() == ev)
			{
				events.erase(it);
				break;
			}
		}
	}

	void LooperPrivate::_remove_events(int id)
	{
		std::lock_guard<std::recursive_mutex> lock(event_mtx);
		if (id == -1)
			events.clear();
		else
		{
			for (auto it = events.begin(); it != events.end();)
			{
				if ((*it)->id == id)
					it = events.erase(it);
				else
					it++;
			}
		}
	}

	void LooperPrivate::_process_events()
	{
		std::lock_guard<std::recursive_mutex> lock(event_mtx);
		for (auto it = events.begin(); it != events.end();)
		{
			auto& e = *it;
			auto excute = false;
			if (e->rest.is_frame)
			{
				if (e->rest.v.frames == 0)
					excute = true;
				else
					e->rest.v.frames--;
			}
			else
			{
				e->rest.v.time -= _delta_time;
				if (e->rest.v.time <= 0)
					excute = true;
			}
			if (excute)
			{
				e->capture._current = nullptr;
				e->callback(e->capture);
				if (e->capture._current != INVALID_POINTER)
				{
					it = events.erase(it);
					continue;
				}
				e->rest = e->interval;
			}
			it++;
		}
	}

	LooperPrivate* _looper = new LooperPrivate;

	Looper* get_looper()
	{
		return _looper;
	}

	void SchedulePrivate::Event::add_to_looper(SchedulePrivate* s)
	{
		rest = duration;
		callback(capture, -1.f, duration);
		auto e = this;
		_looper->_add_event([](Capture& c) {
			auto e = c.data<Event*>();
			e->rest -= _looper->_delta_time;
			auto end = e->rest <= 0.f;
			e->callback(e->capture, e->duration - max(e->rest, 0.f), e->duration);
			if (!end)
				c._current = INVALID_POINTER;
			else
			{
				auto thiz = c.thiz<SchedulePrivate>();
				auto index = e->index;
				auto g = e->group;
				if (g)
				{
					g->complete_count++;
					if (g->complete_count < g->events.size())
						return;
					index = g->index;
				}
				index++;
				if (index < thiz->_items.size())
					thiz->_items[index]->excute(thiz);
				else if (thiz->_once)
					delete thiz;
			}
		}, Capture().set_data(&e).set_thiz(s));
	}

	void SchedulePrivate::Event::excute(SchedulePrivate* s)
	{
		if (delay > 0.f)
		{
			_looper->_add_event([](Capture& c) {
				c.thiz<Event>()->add_to_looper(c.data<SchedulePrivate*>());
			}, Capture().set_data(&s).set_thiz(this), delay);
		}
		else
			add_to_looper(s);
	}

	void SchedulePrivate::Group::excute(SchedulePrivate* s)
	{
		complete_count = 0;
		for (auto& e : events)
		{
			if (e->delay > 0.f)
			{
				_looper->_add_event([](Capture& c) {
					c.thiz<Event>()->add_to_looper(c.data<SchedulePrivate*>());
				}, Capture().set_data(&s).set_thiz(e.get()), e->delay);
			}
			else
				e->add_to_looper(s);
		}
	}

	void SchedulePrivate::_add_event(float delay, float duration, void(*callback)(Capture& c, float time, float duration), const Capture& capture)
	{
		auto e = new Event(_curr_group, _curr_group ? _curr_group->events.size() : _items.size());
		e->delay = delay;
		e->duration = duration;
		e->callback = callback;
		e->capture = capture;
		if (_curr_group)
			_curr_group->events.emplace_back(e);
		else
			_items.emplace_back(e);
	}

	void SchedulePrivate::_begin_group()
	{
		if (_curr_group)
			return;
		auto g = new Group(_items.size());
		_items.emplace_back(g);
		_curr_group = g;
	}

	void SchedulePrivate::_end_group()
	{
		if (!_curr_group)
			return;
		if (_curr_group->events.empty())
			_items.erase(_items.end() - 1);
		_curr_group = nullptr;
	}

	void SchedulePrivate::_start()
	{
		if (_items.empty())
			return;
		_items.front()->excute(this);
	}

	void SchedulePrivate::_stop()
	{

	}

	Schedule* Schedule::create(bool once)
	{
		return new SchedulePrivate(once);
	}
}
