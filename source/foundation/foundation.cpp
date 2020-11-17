#include <flame/serialize.h>
#include "foundation_private.h"
#include "bitmap_private.h"

#define NOMINMAX
#include <process.h>
#include <shellapi.h>
#include <ShlObj.h>
#include <Shlwapi.h>
#include <CommCtrl.h>
#include <thumbcache.h>
#include <ImageHlp.h>

namespace flame
{
	static void* (*pf_allocate)(uint size);
	static void  (*pf_deallocate)(void* p);
	static void* (*pf_reallocate)(void* p, uint size);
	static char* (*pf_stralloc)(void* p, uint size);
	static wchar_t* (*pf_wstralloc)(void* p, uint size);

	void set_allocator(void* (*allocate)(uint size), void(*deallocate)(void* p), void* (*reallocate)(void* p, uint size),
		char* (*stralloc)(void* p, uint size), wchar_t* (*wstralloc)(void* p, uint size))
	{
		pf_allocate = allocate;
		pf_deallocate = deallocate;
		pf_reallocate = reallocate;
		pf_stralloc = stralloc;
		pf_wstralloc = wstralloc;
	}

	void* f_malloc(uint size)
	{
		if (!pf_allocate)
			return malloc(size);
		return pf_allocate(size);
	}

	void* f_realloc(void* p, uint size)
	{
		if (!p)
			return f_malloc(size);
		if (!pf_reallocate)
			return realloc(p, size);
		return pf_reallocate(p, size);
	}

	void f_free(void* p)
	{
		if (!pf_deallocate)
		{
			free(p);
			return;
		}
		pf_deallocate(p);
	}

	char* f_stralloc(void* p, uint size)
	{
		if (!pf_stralloc)
		{
			auto& str = *(std::string*)p;
			str.resize(size);
			return str.data();
		}
		return pf_stralloc(p, size);
	}

	wchar_t* f_wstralloc(void* p, uint size)
	{
		if (!pf_stralloc)
		{
			auto& str = *(std::wstring*)p;
			str.resize(size);
			return str.data();
		}
		return pf_wstralloc(p, size);
	}

	Guid generate_guid()
	{
		Guid ret;
		CoCreateGuid((GUID*)&ret);
		return ret;
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

	void* get_hinst()
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

	void get_library_dependencies(const wchar_t* filename, void (*callback)(Capture& c, const wchar_t* filename), const Capture& capture)
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
						callback((Capture&)capture, (parent_path / pstr).c_str());
					}

					importDesc++;
				}
			}
			ImageUnload(image);
		}

		f_free(capture._data);
	}

	void get_clipboard(void* str)
	{
		OpenClipboard(NULL);
		auto hMemory = GetClipboardData(CF_UNICODETEXT);
		auto size = GlobalSize(hMemory) / sizeof(wchar_t) - 1;
		auto dst = f_wstralloc(str, size);
		wcscpy(dst, (wchar_t*)GlobalLock(hMemory));
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

		IShellFolder* desktop_folder, * shell_folder;
		SHGetDesktopFolder(&desktop_folder);

		LPITEMIDLIST pidl;
		hr = desktop_folder->ParseDisplayName(NULL, NULL, (wchar_t*)filename.c_str(), NULL, &pidl, NULL);
		SHBindToParent(pidl, IID_PPV_ARGS(&shell_folder), NULL);
		auto pidl_child = ILFindLastID(pidl);

		IThumbnailProvider* thumbnail_provider;
		hr = shell_folder->GetUIObjectOf(NULL, 1, (LPCITEMIDLIST*)&pidl_child, IID_IThumbnailProvider, NULL, (void**)&thumbnail_provider);
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

	static KeyboardKey vk_code_to_key(int vkCode)
	{
		switch (vkCode)
		{
		case VK_BACK:
			return Keyboard_Backspace;
		case VK_TAB:
			return Keyboard_Tab;
		case VK_RETURN:
			return Keyboard_Enter;
		case VK_SHIFT:
			return Keyboard_Shift;
		case VK_CONTROL:
			return Keyboard_Ctrl;
		case VK_MENU:
			return Keyboard_Alt;
		case VK_PAUSE:
			return Keyboard_Pause;
		case VK_CAPITAL:
			return Keyboard_CapsLock;
		case VK_ESCAPE:
			return Keyboard_Esc;
		case VK_SPACE:
			return Keyboard_Space;
		case VK_PRIOR:
			return Keyboard_PgUp;
		case VK_NEXT:
			return Keyboard_PgDn;
		case VK_END:
			return Keyboard_End;
		case VK_HOME:
			return Keyboard_Home;
		case VK_LEFT:
			return Keyboard_Left;
		case VK_UP:
			return Keyboard_Up;
		case VK_RIGHT:
			return Keyboard_Right;
		case VK_DOWN:
			return Keyboard_Down;
		case VK_SNAPSHOT:
			return Keyboard_PrtSc;
		case VK_INSERT:
			return Keyboard_Ins;
		case VK_DELETE:
			return Keyboard_Del;
		case '0':
			return Keyboard_0;
		case '1':
			return Keyboard_1;
		case '2':
			return Keyboard_2;
		case '3':
			return Keyboard_3;
		case '4':
			return Keyboard_4;
		case '5':
			return Keyboard_5;
		case '6':
			return Keyboard_6;
		case '7':
			return Keyboard_7;
		case '8':
			return Keyboard_8;
		case '9':
			return Keyboard_9;
		case 'A':
			return Keyboard_A;
		case 'B':
			return Keyboard_B;
		case 'C':
			return Keyboard_C;
		case 'D':
			return Keyboard_D;
		case 'E':
			return Keyboard_E;
		case 'F':
			return Keyboard_F;
		case 'G':
			return Keyboard_G;
		case 'H':
			return Keyboard_H;
		case 'I':
			return Keyboard_I;
		case 'J':
			return Keyboard_J;
		case 'K':
			return Keyboard_K;
		case 'L':
			return Keyboard_L;
		case 'M':
			return Keyboard_M;
		case 'N':
			return Keyboard_N;
		case 'O':
			return Keyboard_O;
		case 'P':
			return Keyboard_P;
		case 'Q':
			return Keyboard_Q;
		case 'R':
			return Keyboard_R;
		case 'S':
			return Keyboard_S;
		case 'T':
			return Keyboard_T;
		case 'U':
			return Keyboard_U;
		case 'V':
			return Keyboard_V;
		case 'W':
			return Keyboard_W;
		case 'X':
			return Keyboard_X;
		case 'Y':
			return Keyboard_Y;
		case 'Z':
			return Keyboard_Z;
		case VK_NUMPAD0:
			return Keyboard_Numpad0;
		case VK_NUMPAD1:
			return Keyboard_Numpad1;
		case VK_NUMPAD2:
			return Keyboard_Numpad2;
		case VK_NUMPAD3:
			return Keyboard_Numpad3;
		case VK_NUMPAD4:
			return Keyboard_Numpad4;
		case VK_NUMPAD5:
			return Keyboard_Numpad5;
		case VK_NUMPAD6:
			return Keyboard_Numpad6;
		case VK_NUMPAD7:
			return Keyboard_Numpad7;
		case VK_NUMPAD8:
			return Keyboard_Numpad8;
		case VK_NUMPAD9:
			return Keyboard_Numpad9;
		case VK_ADD:
			return Keyboard_Add;
		case VK_SUBTRACT:
			return Keyboard_Subtract;
		case VK_MULTIPLY:
			return Keyboard_Multiply;
		case VK_DIVIDE:
			return Keyboard_Divide;
		case VK_SEPARATOR:
			return Keyboard_Separator;
		case VK_DECIMAL:
			return Keyboard_Decimal;
		case VK_F1:
			return Keyboard_F1;
		case VK_F2:
			return Keyboard_F2;
		case VK_F3:
			return Keyboard_F3;
		case VK_F4:
			return Keyboard_F4;
		case VK_F5:
			return Keyboard_F5;
		case VK_F6:
			return Keyboard_F6;
		case VK_F7:
			return Keyboard_F7;
		case VK_F8:
			return Keyboard_F8;
		case VK_F9:
			return Keyboard_F9;
		case VK_F10:
			return Keyboard_F10;
		case VK_F11:
			return Keyboard_F11;
		case VK_F12:
			return Keyboard_F12;
		case VK_NUMLOCK:
			return Keyboard_NumLock;
		case VK_SCROLL:
			return Keyboard_ScrollLock;
		}
		return KeyboardKey_Count;
	}

	int key_to_vk_code(KeyboardKey key)
	{
		switch (key)
		{
		case Keyboard_Backspace:
			return VK_BACK;
		case Keyboard_Tab:
			return VK_TAB;
		case Keyboard_Enter:
			return VK_RETURN;
		case Keyboard_Shift:
			return VK_SHIFT;
		case Keyboard_Ctrl:
			return VK_CONTROL;
		case Keyboard_Alt:
			return VK_MENU;
		case Keyboard_Pause:
			return VK_PAUSE;
		case Keyboard_CapsLock:
			return VK_CAPITAL;
		case Keyboard_Esc:
			return VK_ESCAPE;
		case Keyboard_Space:
			return VK_SPACE;
		case Keyboard_PgUp:
			return VK_PRIOR;
		case Keyboard_PgDn:
			return VK_NEXT;
		case Keyboard_End:
			return VK_END;
		case Keyboard_Home:
			return VK_HOME;
		case Keyboard_Left:
			return VK_LEFT;
		case Keyboard_Up:
			return VK_UP;
		case Keyboard_Right:
			return VK_RIGHT;
		case Keyboard_Down:
			return VK_DOWN;
		case Keyboard_PrtSc:
			return VK_SNAPSHOT;
		case Keyboard_Ins:
			return VK_INSERT;
		case Keyboard_Del:
			return VK_DELETE;
		case Keyboard_0:
			return '0';
		case Keyboard_1:
			return '1';
		case Keyboard_2:
			return '2';
		case Keyboard_3:
			return '3';
		case Keyboard_4:
			return '4';
		case Keyboard_5:
			return '5';
		case Keyboard_6:
			return '6';
		case Keyboard_7:
			return '7';
		case Keyboard_8:
			return '8';
		case Keyboard_9:
			return '9';
		case Keyboard_A:
			return 'A';
		case Keyboard_B:
			return 'B';
		case Keyboard_C:
			return 'C';
		case Keyboard_D:
			return 'D';
		case Keyboard_E:
			return 'E';
		case Keyboard_F:
			return 'F';
		case Keyboard_G:
			return 'G';
		case Keyboard_H:
			return 'H';
		case Keyboard_I:
			return 'I';
		case Keyboard_J:
			return 'J';
		case Keyboard_K:
			return 'K';
		case Keyboard_L:
			return 'L';
		case Keyboard_M:
			return 'M';
		case Keyboard_N:
			return 'N';
		case Keyboard_O:
			return 'O';
		case Keyboard_P:
			return 'P';
		case Keyboard_Q:
			return 'Q';
		case Keyboard_R:
			return 'R';
		case Keyboard_S:
			return 'S';
		case Keyboard_T:
			return 'T';
		case Keyboard_U:
			return 'U';
		case Keyboard_V:
			return 'V';
		case Keyboard_W:
			return 'W';
		case Keyboard_X:
			return 'X';
		case Keyboard_Y:
			return 'Y';
		case Keyboard_Z:
			return 'Z';
		case Keyboard_Numpad0:
			return VK_NUMPAD0;
		case Keyboard_Numpad1:
			return VK_NUMPAD1;
		case Keyboard_Numpad2:
			return VK_NUMPAD2;
		case Keyboard_Numpad3:
			return VK_NUMPAD3;
		case Keyboard_Numpad4:
			return VK_NUMPAD4;
		case Keyboard_Numpad5:
			return VK_NUMPAD5;
		case Keyboard_Numpad6:
			return VK_NUMPAD6;
		case Keyboard_Numpad7:
			return VK_NUMPAD7;
		case Keyboard_Numpad8:
			return VK_NUMPAD8;
		case Keyboard_Numpad9:
			return VK_NUMPAD9;
		case Keyboard_Add:
			return VK_ADD;
		case Keyboard_Subtract:
			return VK_SUBTRACT;
		case Keyboard_Multiply:
			return VK_MULTIPLY;
		case Keyboard_Divide:
			return VK_DIVIDE;
		case Keyboard_Separator:
			return VK_SEPARATOR;
		case Keyboard_Decimal:
			return VK_DECIMAL;
		case Keyboard_F1:
			return VK_F1;
		case Keyboard_F2:
			return VK_F2;
		case Keyboard_F3:
			return VK_F3;
		case Keyboard_F4:
			return VK_F4;
		case Keyboard_F5:
			return VK_F5;
		case Keyboard_F6:
			return VK_F6;
		case Keyboard_F7:
			return VK_F7;
		case Keyboard_F8:
			return VK_F8;
		case Keyboard_F9:
			return VK_F9;
		case Keyboard_F10:
			return VK_F10;
		case Keyboard_F11:
			return VK_F11;
		case Keyboard_F12:
			return VK_F12;
		case Keyboard_NumLock:
			return VK_NUMLOCK;
		case Keyboard_ScrollLock:
			return VK_SCROLL;
		}
		return KeyboardKey_Count;
	}

	struct GlobalKeyListener
	{
		KeyboardKey key;
		bool down;
		bool ctrl;
		bool shift;
		bool alt;
		std::unique_ptr<Closure<void(Capture& c)>> callback;
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
			if (l->down && wParam == WM_KEYUP)
				continue;
			l->callback->call();
		}

		return CallNextHookEx(global_key_hook, nCode, wParam, lParam);
	}

	void* add_global_key_listener(KeyboardKey key, void (*callback)(Capture& c), const Capture& capture, bool down, bool ctrl, bool shift, bool alt)
	{
		auto l = new GlobalKeyListener;
		l->key = key;
		l->down = down;
		l->ctrl = ctrl;
		l->shift = shift;
		l->alt = alt;
		l->callback.reset(new Closure(callback, capture));

		global_key_listeners.emplace_back(l);

		if (!global_key_hook)
			global_key_hook = SetWindowsHookEx(WH_KEYBOARD_LL, global_key_callback, (HINSTANCE)get_hinst(), 0);

		return l;
	}

	void remove_global_key_listener(void* handle)
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

	void send_global_keyboard_event(KeyboardKey key, bool down)
	{
		keybd_event(key_to_vk_code(key), 0, down ? 0 : KEYEVENTF_KEYUP, 0);
	}

	void send_global_mouse_event(MouseKey key, bool down)
	{
		auto flags = -1;
		switch (key)
		{
		case Mouse_Left:
			flags = down ? MOUSEEVENTF_LEFTDOWN : MOUSEEVENTF_LEFTUP;
			break;
		case Mouse_Right:
			flags = down ? MOUSEEVENTF_RIGHTDOWN : MOUSEEVENTF_RIGHTUP;
			break;
		case Mouse_Middle:
			flags = down ? MOUSEEVENTF_MIDDLEDOWN : MOUSEEVENTF_MIDDLEUP;
			break;
		}
		mouse_event(flags, 0, 0, 0, NULL);
	}

	void set_mouse_pos(const Vec2i& pos)
	{
		SetCursorPos(pos.x(), pos.y());
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

	void exec(const wchar_t* filename, wchar_t* parameters, void* str)
	{
		bool ok;
		HANDLE hChildStd_OUT_Rd = NULL;
		HANDLE hChildStd_OUT_Wr = NULL;
		if (str)
		{
			SECURITY_ATTRIBUTES saAttr;
			saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
			saAttr.bInheritHandle = TRUE;
			saAttr.lpSecurityDescriptor = NULL;
			ok = CreatePipe(&hChildStd_OUT_Rd, &hChildStd_OUT_Wr, &saAttr, 0);
			fassert(ok);
			ok = SetHandleInformation(hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0);
			fassert(ok);
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
			fassert(0);
		}

		WaitForSingleObject(proc_info.hProcess, INFINITE);

		CloseHandle(proc_info.hProcess);
		CloseHandle(proc_info.hThread);

		if (str)
		{
			DWORD size;
			PeekNamedPipe(hChildStd_OUT_Rd, NULL, NULL, NULL, &size, NULL);
			auto dst = f_stralloc(str, size);
			PeekNamedPipe(hChildStd_OUT_Rd, (void*)dst, size, NULL, NULL, NULL);
		}
	}

	void debug_break()
	{
#ifdef _DEBUG
		DebugBreak();
#endif
	}

#ifdef _DEBUG
	static std::vector<std::unique_ptr<Closure<void(Capture&)>>> assert_callbacks;
#endif

	void* add_assert_callback(void (*callback)(Capture& c), const Capture& capture)
	{
#ifdef _DEBUG
		auto c = new Closure(callback, capture);
		assert_callbacks.emplace_back(c);
		return c;
#endif
	}

	void remove_assert_callback(void* ret)
	{
#ifdef _DEBUG
		std::erase_if(assert_callbacks, [&](const auto& i) {
			return i == (decltype(i))ret;
		});
#endif
	}

	void raise_assert(const char* expression, const char* file, uint line)
	{
#ifdef _DEBUG
		for (auto& c : assert_callbacks)
			c->call();
		_wassert(s2w(expression).c_str(), s2w(file).c_str(), line);
#endif
	}

	void get_call_frames(void** (*array_allocator)(Capture& c, uint size), const Capture& capture)
	{
		void* buf[64];
		auto n = CaptureStackBackTrace(0, size(buf), buf, nullptr);
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
		fassert(dir_handle != INVALID_HANDLE_VALUE);

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
			fassert(ok);

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
			fassert(ok);

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

	static LRESULT CALLBACK wnd_proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		auto w = (WindowPrivate*)GetWindowLongPtr(hWnd, 0);
		if (w)
			w->wnd_proc(message, wParam, lParam);

		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	WindowPrivate::WindowPrivate(const std::string& _title, const Vec2u& _size, uint _style, WindowPrivate* parent)
	{
		static bool initialized = false;
		if (!initialized)
		{
			WNDCLASSEXA wcex;
			wcex.cbSize = sizeof(WNDCLASSEXA);
			wcex.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
			wcex.lpfnWndProc = ::flame::wnd_proc;
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
				auto icon_image = BitmapPrivate::create(icon_fn);
				icon_image->swap_channel(0, 2);
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

		title = _title;

		size = _size;
		style = _style;

		fassert(!(style & WindowFullscreen) || (!(style & WindowFrame) && !(style & WindowResizable)));

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
		pos.x() = (screen_size.x() - final_size.x()) / 2;
		pos.y() = (screen_size.y() - final_size.y()) / 2;
		hWnd = CreateWindowEx(win32_ex_style, "flame_wnd", title.c_str(), win32_style,
			pos.x(), pos.y(), final_size.x(), final_size.y(), parent ? parent->hWnd : NULL, NULL, (HINSTANCE)get_hinst(), NULL);
		{
			RECT rect;
			GetClientRect(hWnd, &rect);
			size = Vec2u(rect.right - rect.left, rect.bottom - rect.top);
		}

		SetWindowLongPtr(hWnd, 0, (LONG_PTR)this);

		for (auto i = 0; i < Cursor_Count; i++)
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

		pending_size = size;

		set_cursor(CursorArrow);
		_looper.windows.emplace_back(this);
	}

	WindowPrivate::~WindowPrivate()
	{
		for (auto& l : destroy_listeners)
			l->call();
	}

	void WindowPrivate::wnd_proc(UINT message, WPARAM wParam, LPARAM lParam)
	{
		auto resize = [=]() {
			if (size != pending_size)
			{
				size = pending_size;
				for (auto& l : resize_listeners)
					l->call(size);
			}
		};

		switch (message)
		{
		case WM_KEYDOWN:
		{
			auto v = vk_code_to_key(wParam);
			if (v > 0)
			{
				for (auto& l : key_down_listeners)
					l->call(v);
			}
		}
			break;
		case WM_KEYUP:
		{
			auto v = vk_code_to_key(wParam);
			if (v > 0)
			{
				for (auto& l : key_up_listeners)
					l->call(v);
			}
		}
			break;
		case WM_CHAR:
			for (auto& l : char_listeners)
				l->call(wParam);
			break;
		case WM_LBUTTONDOWN:
		{
			SetCapture(hWnd);
			auto pos = Vec2i((int)LOWORD(lParam), (int)HIWORD(lParam));
			for (auto& l : mouse_left_down_listeners)
				l->call(pos);
		}
			break;
		case WM_LBUTTONUP:
		{
			ReleaseCapture();
			auto pos = Vec2i((int)LOWORD(lParam), (int)HIWORD(lParam));
			for (auto& l : mouse_left_up_listeners)
				l->call(pos);
		}
			break;
		case WM_RBUTTONDOWN:
		{
			auto pos = Vec2i((int)LOWORD(lParam), (int)HIWORD(lParam));
			for (auto& l : mouse_right_down_listeners)
				l->call(pos);
		}
			break;
		case WM_RBUTTONUP:
		{
			auto pos = Vec2i((int)LOWORD(lParam), (int)HIWORD(lParam));
			for (auto& l : mouse_right_up_listeners)
				l->call(pos);
		}
			break;
		case WM_MBUTTONDOWN:
		{
			auto pos = Vec2i((int)LOWORD(lParam), (int)HIWORD(lParam));
			for (auto& l : mouse_middle_down_listeners)
				l->call(pos);
		}
			break;
		case WM_MBUTTONUP:
		{
			auto pos = Vec2i((int)LOWORD(lParam), (int)HIWORD(lParam));
			for (auto& l : mouse_middle_up_listeners)
				l->call(pos);
		}
			break;
		case WM_MOUSEMOVE:
		{
			auto pos = Vec2i((int)LOWORD(lParam), (int)HIWORD(lParam));
			for (auto& l : mouse_move_listeners)
				l->call(pos);
		}
			break;
		case WM_MOUSEWHEEL:
		{
			auto v = (short)HIWORD(wParam) > 0 ? 1 : -1;
			for (auto& l : mouse_scroll_listeners)
				l->call(v);
		}
			break;
		case WM_DESTROY:
			dead = true;
		case WM_ENTERSIZEMOVE:
			sizing = true;
			SetTimer(hWnd, 0, 100, NULL);
			break;
		case WM_EXITSIZEMOVE:
			sizing = false;
			KillTimer(hWnd, 0);
			resize();
			break;
		case WM_TIMER:
			if (wParam == 0)
				resize();
			_looper.one_frame();
			break;
		case WM_SIZE:
			pending_size = Vec2u((int)LOWORD(lParam), (int)HIWORD(lParam));
			if (!sizing)
				resize();
			break;
		case WM_MOVE:
			pos = Vec2i((int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam));
			break;
		case WM_SETCURSOR:
			SetCursor(cursors[cursor_type]);
			break;
		}
	}

	void* WindowPrivate::get_native() 
	{
		return hWnd;
	}

	void WindowPrivate::set_pos(const Vec2i& _pos)
	{
		 pos = _pos;
		SetWindowPos(hWnd, HWND_TOP, pos.x(), pos.y(), 0, 0, SWP_NOSIZE | SWP_NOZORDER);
	}

	void WindowPrivate::set_size(const Vec2u& size)
	{
	}

	Vec2i WindowPrivate::global_to_local(const Vec2i& p)
	{
		POINT pt;
		pt.x = p.x();
		pt.y = p.y();
		ScreenToClient(hWnd, &pt);
		return Vec2i(pt.x, pt.y);
	}

	void WindowPrivate::set_title(const std::string& _title)
	{
		title = _title;
		SetWindowTextA(hWnd, title.c_str());
	}

	void WindowPrivate::set_cursor(CursorType type) 
	{
		if (type == cursor_type)
			return;

		if (cursor_type == CursorNone)
			ShowCursor(true);
		if (type == CursorNone)
			ShowCursor(false);

		cursor_type = type;
	}

	void WindowPrivate::close()
	{
		DestroyWindow(hWnd);
		dead = true;
	}

	void* WindowPrivate::add_key_down_listener(void (*callback)(Capture& c, KeyboardKey key), const Capture& capture)
	{
		auto c = new Closure(callback, capture);
		key_down_listeners.emplace_back(c);
		return c;
	}

	void WindowPrivate::remove_key_down_listener(void* lis)
	{
		std::erase_if(key_down_listeners, [&](const auto& i) {
			return i == (decltype(i))lis;
		});
	}

	void* WindowPrivate::add_key_up_listener(void (*callback)(Capture& c, KeyboardKey key), const Capture& capture)
	{
		auto c = new Closure(callback, capture);
		key_up_listeners.emplace_back(c);
		return c;
	}

	void WindowPrivate::remove_key_up_listener(void* lis)
	{
		std::erase_if(key_up_listeners, [&](const auto& i) {
			return i == (decltype(i))lis;
		});
	}

	void* WindowPrivate::add_char_listener(void (*callback)(Capture& c, char ch), const Capture& capture)
	{
		auto c = new Closure(callback, capture);
		char_listeners.emplace_back(c);
		return c;
	}

	void WindowPrivate::remove_char_listener(void* lis)
	{
		std::erase_if(char_listeners, [&](const auto& i) {
			return i == (decltype(i))lis;
		});
	}

	void* WindowPrivate::add_mouse_left_down_listener(void (*callback)(Capture& c, const Vec2i& pos), const Capture& capture)
	{
		auto c = new Closure(callback, capture);
		mouse_left_down_listeners.emplace_back(c);
		return c;
	}

	void WindowPrivate::remove_mouse_left_down_listener(void* lis)
	{
		std::erase_if(mouse_left_down_listeners, [&](const auto& i) {
			return i == (decltype(i))lis;
		});
	}

	void* WindowPrivate::add_mouse_left_up_listener(void (*callback)(Capture& c, const Vec2i& pos), const Capture& capture)
	{
		auto c = new Closure(callback, capture);
		mouse_left_up_listeners.emplace_back(c);
		return c;
	}

	void WindowPrivate::remove_mouse_left_up_listener(void* lis)
	{
		std::erase_if(mouse_left_up_listeners, [&](const auto& i) {
			return i == (decltype(i))lis;
		});
	}

	void* WindowPrivate::add_mouse_right_down_listener(void (*callback)(Capture& c, const Vec2i& pos), const Capture& capture)
	{
		auto c = new Closure(callback, capture);
		mouse_right_down_listeners.emplace_back(c);
		return c;
	}

	void WindowPrivate::remove_mouse_right_down_listener(void* lis)
	{
		std::erase_if(mouse_right_down_listeners, [&](const auto& i) {
			return i == (decltype(i))lis;
		});
	}

	void* WindowPrivate::add_mouse_right_up_listener(void (*callback)(Capture& c, const Vec2i& pos), const Capture& capture)
	{
		auto c = new Closure(callback, capture);
		mouse_right_up_listeners.emplace_back(c);
		return c;
	}

	void WindowPrivate::remove_mouse_right_up_listener(void* lis)
	{
		std::erase_if(mouse_right_up_listeners, [&](const auto& i) {
			return i == (decltype(i))lis;
		});
	}

	void* WindowPrivate::add_mouse_middle_down_listener(void (*callback)(Capture& c, const Vec2i& pos), const Capture& capture)
	{
		auto c = new Closure(callback, capture);
		mouse_middle_down_listeners.emplace_back(c);
		return c;
	}

	void WindowPrivate::remove_mouse_middle_down_listener(void* lis)
	{
		std::erase_if(mouse_middle_down_listeners, [&](const auto& i) {
			return i == (decltype(i))lis;
		});
	}

	void* WindowPrivate::add_mouse_middle_up_listener(void (*callback)(Capture& c, const Vec2i& pos), const Capture& capture)
	{
		auto c = new Closure(callback, capture);
		mouse_middle_up_listeners.emplace_back(c);
		return c;
	}

	void WindowPrivate::remove_mouse_middle_up_listener(void* lis)
	{
		std::erase_if(mouse_middle_up_listeners, [&](const auto& i) {
			return i == (decltype(i))lis;
		});
	}

	void* WindowPrivate::add_mouse_move_listener(void (*callback)(Capture& c, const Vec2i& pos), const Capture& capture)
	{
		auto c = new Closure(callback, capture);
		mouse_move_listeners.emplace_back(c);
		return c;
	}

	void WindowPrivate::remove_mouse_move_listener(void* lis)
	{
		std::erase_if(mouse_move_listeners, [&](const auto& i) {
			return i == (decltype(i))lis;
		});
	}

	void* WindowPrivate::add_mouse_scroll_listener(void (*callback)(Capture& c, int scroll), const Capture& capture)
	{
		auto c = new Closure(callback, capture);
		mouse_scroll_listeners.emplace_back(c);
		return c;
	}

	void WindowPrivate::remove_mouse_scroll_listener(void* lis)
	{
		std::erase_if(mouse_scroll_listeners, [&](const auto& i) {
			return i == (decltype(i))lis;
		});
	}

	void* WindowPrivate::add_resize_listener(void (*callback)(Capture& c, const Vec2u& size), const Capture& capture)
	{
		auto c = new Closure(callback, capture);
		resize_listeners.emplace_back(c);
		return c;
	}

	void WindowPrivate::remove_resize_listener(void* lis)
	{
		std::erase_if(resize_listeners, [&](const auto& i) {
			return i == (decltype(i))lis;
		});
	}

	void* WindowPrivate::add_destroy_listener(void (*callback)(Capture& c), const Capture& capture)
	{
		auto c = new Closure(callback, capture);
		destroy_listeners.emplace_back(c);
		return c;
	}

	void WindowPrivate::remove_destroy_listener(void* lis)
	{
		std::erase_if(destroy_listeners, [&](const auto& i) {
			return i == (decltype(i))lis;
		});
	}

	Window* Window::create(const char* title, const Vec2u& size, WindowStyleFlags style, Window* parent)
	{
		return new WindowPrivate(title, size, style, (WindowPrivate*)parent);
	}

	int LooperPrivate::loop(void (*_frame_callback)(Capture& c, float delta_time), const Capture& capture)
	{
		if (!_frame_callback)
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

		frame_callback = _frame_callback;
		frame_capture = capture;

		last_time = get_now_ns();
		frame = 0;

		for (;;)
		{
			MSG msg;
			while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			if (!one_frame())
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

	bool LooperPrivate::one_frame()
	{
		for (auto it = windows.begin(); it != windows.end(); )
		{
			auto w = it->get();

			if (w->dead)
				it = windows.erase(it);
			else
				it++;
		}

		if (windows.empty())
		{
			f_free(frame_capture._data);
			return false;
		}

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
					e->rest.v.time -= delta_time;
					if (e->rest.v.time <= 0)
						excute = true;
				}
				if (excute)
				{
					e->capture._current = INVALID_POINTER;
					e->callback(e->capture);
					if (e->capture._current == INVALID_POINTER)
					{
						it = events.erase(it);
						continue;
					}
					e->rest = e->interval;
				}
				it++;
			}
		}

		frame_callback(frame_capture, delta_time);

		frame++;
		auto et = last_time;
		last_time = get_now_ns();
		et = last_time - et;
		delta_time = et / 1000000000.f;
		total_time += delta_time;
		fps_counting++;
		fps_delta += delta_time;
		if (fps_delta >= 1.f)
		{
			fps = fps_counting;
			fps_counting = 0;
			fps_delta = 0.f;
		}

		return true;
	}

	void* LooperPrivate::add_event(void (*callback)(Capture& c), const Capture& capture, CountDown interval, uint id)
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

	void LooperPrivate::reset_event(void* _ev)
	{
		auto ev = (Event*)_ev;
		ev->rest = ev->interval;
	}

	void LooperPrivate::remove_event(void* ev)
	{
		std::lock_guard<std::recursive_mutex> lock(event_mtx);
		for (auto it = events.begin(); it != events.end(); it++)
		{
			if ((*it).get() == ev)
			{
				if ((*it)->capture._current != nullptr)
					(*it)->capture._current = INVALID_POINTER;
				else
					events.erase(it);
				break;
			}
		}
	}

	void LooperPrivate::remove_events(int id)
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

	LooperPrivate _looper;

	Looper& looper()
	{
		return _looper;
	}

	void SchedulePrivate::Event::add_to_looper(SchedulePrivate* s)
	{
		rest = duration;
		callback(capture, -1.f, duration);
		auto e = this;
		_looper.add_event([](Capture& c) {
			auto e = c.data<Event*>();
			e->rest -= _looper.delta_time;
			auto end = e->rest <= 0.f;
			e->callback(e->capture, e->duration - max(e->rest, 0.f), e->duration);
			if (!end)
				c._current = nullptr;
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
				if (index < thiz->items.size())
					thiz->items[index]->excute(thiz);
				else if (thiz->once)
					delete thiz;
			}
		}, Capture().set_data(&e).set_thiz(s));
	}

	void SchedulePrivate::Event::excute(SchedulePrivate* s)
	{
		if (delay > 0.f)
		{
			_looper.add_event([](Capture& c) {
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
				_looper.add_event([](Capture& c) {
					c.thiz<Event>()->add_to_looper(c.data<SchedulePrivate*>());
				}, Capture().set_data(&s).set_thiz(e.get()), e->delay);
			}
			else
				e->add_to_looper(s);
		}
	}

	void SchedulePrivate::add_event(float delay, float duration, void(*callback)(Capture& c, float time, float duration), const Capture& capture)
	{
		auto e = new Event(curr_group, curr_group ? curr_group->events.size() : items.size());
		e->delay = delay;
		e->duration = duration;
		e->callback = callback;
		e->capture = capture;
		if (curr_group)
			curr_group->events.emplace_back(e);
		else
			items.emplace_back(e);
	}

	void SchedulePrivate::begin_group()
	{
		if (curr_group)
			return;
		auto g = new Group(items.size());
		items.emplace_back(g);
		curr_group = g;
	}

	void SchedulePrivate::end_group()
	{
		if (!curr_group)
			return;
		if (curr_group->events.empty())
			items.erase(items.end() - 1);
		curr_group = nullptr;
	}

	void SchedulePrivate::start()
	{
		if (items.empty())
			return;
		items.front()->excute(this);
	}

	void SchedulePrivate::stop()
	{

	}

	Schedule* Schedule::create(bool once)
	{
		return new SchedulePrivate(once);
	}
}
