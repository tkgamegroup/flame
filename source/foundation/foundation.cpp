#include <flame/serialize.h>
#include <flame/foundation/bitmap.h>

#ifdef FLAME_WINDOWS
#define NOMINMAX
#include <Windows.h>
#include <process.h>
#include <ImageHlp.h>
#include <shellapi.h>
#include <ShlObj.h>
#include <Shlwapi.h>
#include <CommCtrl.h>
#include <thumbcache.h>
#elif FLAME_ANDROID
#include <android_native_app_glue.h>
#endif

namespace flame
{
	struct ListenerHubImnplPrivate: ListenerHubImpl
	{
		std::vector<std::unique_ptr<Closure<void(void* c)>>> listeners;
	};

	ListenerHubImpl* ListenerHubImpl::create()
	{
		return new ListenerHubImnplPrivate;
	}

	void ListenerHubImpl::destroy(ListenerHubImpl* h)
	{
		delete (ListenerHubImnplPrivate*)h;
	}

	uint ListenerHubImpl::count()
	{
		return ((ListenerHubImnplPrivate*)this)->listeners.size();
	}

	Closure<void(void*)>& ListenerHubImpl::item(uint idx)
	{
		return *((ListenerHubImnplPrivate*)this)->listeners[idx].get();
	}

	void* ListenerHubImpl::add_plain(void(*pf)(void* c), const Mail<>& capture)
	{
		auto c = new Closure<void(void* c)>;
		c->function = pf;
		c->capture = capture;
		((ListenerHubImnplPrivate*)this)->listeners.emplace_back(c);
		return c;
	}

	void ListenerHubImpl::remove_plain(void* c)
	{
		auto& listeners = ((ListenerHubImnplPrivate*)this)->listeners;
		for (auto it = listeners.begin(); it != listeners.end(); it++)
		{
			if (it->get() == c)
			{
				listeners.erase(it);
				return;
			}
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

	StringW get_curr_path()
	{
		wchar_t buf[260];
		GetCurrentDirectoryW(sizeof(buf), buf);
		return StringW(buf);
	}

	StringW get_app_path()
	{
		wchar_t buf[260];
		GetModuleFileNameW(nullptr, buf, sizeof(buf));
		return StringW(std::filesystem::path(buf).parent_path().wstring());
	}

	void set_curr_path(const wchar_t* p)
	{
		SetCurrentDirectoryW(p);
	}

	void read_process_memory(void* process, void* address, uint size, void* dst)
	{
		SIZE_T ret_byte;
		assert(ReadProcessMemory(process, address, dst, size, &ret_byte));
	}

	void sleep(int time)
	{
		Sleep(time < 0 ? INFINITE : time);
	}

	void* create_event(bool signaled)
	{
		return CreateEvent(NULL, false, signaled, NULL);
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

	void do_simple_dispatch_loop()
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

	bool is_file_occupied(const wchar_t* filename)
	{
		auto file = CreateFileW(filename, GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
		if (GetLastError() == 0)
		{
			CloseHandle(file);
			return false;
		}
		return true;
	}

	void exec(const wchar_t* filename, const wchar_t* parameters, bool wait, bool show)
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

	StringA exec_and_get_output(const wchar_t* filename, wchar_t* parameters)
	{
		HANDLE hChildStd_OUT_Rd = NULL;
		HANDLE hChildStd_OUT_Wr = NULL;

		SECURITY_ATTRIBUTES saAttr;
		saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
		saAttr.bInheritHandle = TRUE;
		saAttr.lpSecurityDescriptor = NULL;

		assert(CreatePipe(&hChildStd_OUT_Rd, &hChildStd_OUT_Wr, &saAttr, 0));

		assert(SetHandleInformation(hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0));

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

		DWORD output_size;
		StringA output;
		PeekNamedPipe(hChildStd_OUT_Rd, NULL, NULL, NULL, &output_size, NULL);
		output.resize(output_size);
		PeekNamedPipe(hChildStd_OUT_Rd, (void*)output.v, output_size, NULL, NULL, NULL);
		return output;
	}

	void exec_and_redirect_to_std_output(const wchar_t* filename, wchar_t* parameters)
	{
		STARTUPINFOW start_info = {};
		start_info.cb = sizeof(STARTUPINFOW);
		start_info.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
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
	}


	StringA compile_to_dll(uint source_count, const wchar_t* const* sources, uint library_count, const wchar_t* const* libraries, const wchar_t* out)
	{
		auto cl = wsfmt(L"\"%s/VC/Auxiliary/Build/vcvars64.bat\" & cl ", s2w(VS_LOCATION));

		for (auto i = 0; i < source_count; i++)
		{
			cl += sources[i];
			cl += L" ";
		}
		cl += L"-LD -MD -EHsc -Zi -std:c++17 -I ../include -link -DEBUG ";
		for (auto i = 0; i < library_count; i++)
		{
			cl += libraries[i];
			cl += L" ";
		}

		cl += L" -out:";
		cl += out;

		return exec_and_get_output(nullptr, cl.data());
	}

	static PIMAGE_SECTION_HEADER get_enclosing_section_header(DWORD rva, PIMAGE_NT_HEADERS64 pNTHeader)
	{
		PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION(pNTHeader);

		for (auto i = 0; i < pNTHeader->FileHeader.NumberOfSections; i++, section++)
		{
			DWORD size = section->Misc.VirtualSize;
			if (0 == size)
				size = section->SizeOfRawData;

			if ((rva >= section->VirtualAddress) && (rva < (section->VirtualAddress + size)))
				return section;
		}

		return 0;
	}

	static LPVOID get_ptr_from_rva(DWORD rva, PIMAGE_NT_HEADERS64 pNTHeader, PBYTE imageBase)
	{
		PIMAGE_SECTION_HEADER pSectionHdr;
		INT delta;

		pSectionHdr = get_enclosing_section_header(rva, pNTHeader);
		if (!pSectionHdr)
			return 0;

		delta = (INT)(pSectionHdr->VirtualAddress - pSectionHdr->PointerToRawData);
		return (PVOID)(imageBase + rva - delta);
	}

	Array<StringA> get_module_dependancies(const wchar_t* module_name)
	{
		PLOADED_IMAGE image = ImageLoad(w2s(module_name).c_str(), std::filesystem::path(module_name).parent_path().string().c_str());

		auto ret = Array<StringA>();
		if (image->FileHeader->OptionalHeader.NumberOfRvaAndSizes >= 2) 
		{
			PIMAGE_IMPORT_DESCRIPTOR importDesc = (PIMAGE_IMPORT_DESCRIPTOR)get_ptr_from_rva(
					image->FileHeader->OptionalHeader.DataDirectory[1].VirtualAddress,
					image->FileHeader, image->MappedAddress);
			while (true)
			{
				if ((importDesc->TimeDateStamp == 0) && (importDesc->Name == 0))
					break;

				ret.push_back((char*)get_ptr_from_rva(importDesc->Name,
					image->FileHeader,
					image->MappedAddress));
				importDesc++;
			}
		}
		ImageUnload(image);
		return ret;
	}

	void* get_module_from_address(void* addr)
	{
		HMODULE module = NULL;
		GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCSTR)addr, &module);
		return module;
	}

	StringW get_module_name(void* module)
	{
		wchar_t buf[260];
		GetModuleFileNameW((HMODULE)module, buf, sizeof(buf));
		return StringW(buf);
	}

	void* load_module(const wchar_t* module_name)
	{
		return LoadLibraryW(module_name);
	}

	void* get_module_func(void* module, const char* name)
	{
		return GetProcAddress((HMODULE)module, name);
	}

	void free_module(void* library)
	{
		FreeLibrary((HMODULE)library);
	}

	StringW get_clipboard()
	{
		OpenClipboard(NULL);
		auto hMemory = GetClipboardData(CF_UNICODETEXT);
		StringW output;
		output.resize(GlobalSize(hMemory) / sizeof(wchar_t) - 1);
		memcpy(output.v, GlobalLock(hMemory), sizeof(wchar_t)*output.s);
		GlobalUnlock(hMemory);
		CloseClipboard();
		return output;
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

	void open_explorer_and_select(const wchar_t* filename)
	{
		auto pidl = ILCreateFromPathW(filename);
		if (pidl)
		{
			SHOpenFolderAndSelectItems(pidl, 0, 0, 0);
			ILFree(pidl);
		}
	}

	void move_to_trashbin(const wchar_t* filename)
	{
		SHFILEOPSTRUCTW sh_op;
		sh_op.hwnd = 0;
		sh_op.wFunc = FO_DELETE;
		std::wstring dz_str(filename);
		dz_str += L'\0';
		sh_op.pFrom = dz_str.c_str();
		sh_op.pTo = L"\0";
		sh_op.fFlags = FOF_ALLOWUNDO;
		sh_op.fAnyOperationsAborted = false;
		sh_op.hNameMappings = nullptr;
		sh_op.lpszProgressTitle = L"Deleting..";
		auto result = SHFileOperationW(&sh_op);
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

	Key vk_code_to_key(int vkCode)
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

	bool is_modifier_pressing(Key k, int left_or_right)
	{
#ifdef FLAME_WINDOWS
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
#else
		return false;
#endif
	}

	struct GlobalKeyListener
	{
		Key key;
		bool modifier_shift;
		bool modifier_ctrl;
		bool modifier_alt;
		void (*callback)(void* c, KeyState action);
		Mail<> capture;
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
			if (l->modifier_shift && !(GetAsyncKeyState(VK_LSHIFT) || GetAsyncKeyState(VK_RSHIFT)))
				continue;
			if (l->modifier_ctrl && !(GetAsyncKeyState(VK_LCONTROL) || GetAsyncKeyState(VK_RCONTROL)))
				continue;
			if (l->modifier_alt && !(GetAsyncKeyState(VK_LMENU) || GetAsyncKeyState(VK_RMENU)))
				continue;
			auto action = KeyStateNull;
			if (wParam == WM_KEYDOWN)
				action = KeyStateDown;
			else if (wParam == WM_KEYUP)
				action = KeyStateUp;
			l->callback(l->capture.p, action);
		}

		return CallNextHookEx(global_key_hook, nCode, wParam, lParam);
	}

	void* add_global_key_listener(Key key, bool modifier_shift, bool modifier_ctrl, bool modifier_alt, void (*callback)(void* c, KeyState action), const Mail<>& capture)
	{
		auto l = new GlobalKeyListener;
		l->key = key;
		l->modifier_shift = modifier_shift;
		l->modifier_ctrl = modifier_ctrl;
		l->modifier_alt = modifier_alt;
		l->callback = callback;
		l->capture = capture;

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
				delete_mail((*it)->capture);
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

	void do_file_watch(void* event_end, bool all_changes, const std::wstring& path, void (*callback)(void* c, FileChangeType type, const wchar_t* filename), const Mail<>& capture)
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
			ZeroMemory(&overlapped, sizeof(OVERLAPPED));
			overlapped.hEvent = event_changed;

			assert(ReadDirectoryChangesW(dir_handle, notify_buf, sizeof(notify_buf), true, flags, NULL, &overlapped, NULL));

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
			assert(GetOverlappedResult(dir_handle, &overlapped, &ret_bytes, false) == 1);

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
					callback(capture.p, type, !path.empty() ? (path + L"\\" + p->FileName).c_str() : p->FileName);

				if (p->NextEntryOffset <= 0)
					break;
				base += p->NextEntryOffset;
				p = (FILE_NOTIFY_INFORMATION*)(notify_buf + base);
			}
		}

		delete_mail(capture);

		destroy_event(event_changed);
		destroy_event(dir_handle);

		if (event_end)
			destroy_event(event_end);
	}

	void* add_file_watcher(const wchar_t* path, void (*callback)(void* c, FileChangeType type, const wchar_t* filename), const Mail<>& capture, bool all_changes, bool sync)
	{
		if (!sync)
		{
			auto ev = create_event(false);

			std::thread([=]() {
				do_file_watch(ev, all_changes, path, callback, capture);
			}).detach();

			return ev;
		}
		else
		{
			do_file_watch(nullptr, all_changes, path, callback, capture);

			return nullptr;
		}

		return nullptr;
	}

	const auto all_workers = 3;
	static std::mutex mtx;
	static std::condition_variable cv;
	static auto workers = all_workers;

	static std::vector<std::pair<void(*)(void* c), Mail<>>> works;

	static void try_distribute_work()
	{
		if (!works.empty() && workers > 0)
		{
			mtx.lock();

			workers--;
			auto w = works.front();
			works.erase(works.begin());

			std::thread([=]() {
				w.first(w.second.p);
				delete_mail(w.second);
				mtx.lock();
				workers++;
				cv.notify_one();
				mtx.unlock();
				try_distribute_work();
			}).detach();

			mtx.unlock();
		}
	}

	void add_work(void (*function)(void* c), const Mail<>& capture)
	{
		mtx.lock();
		works.emplace_back(function, capture);
		mtx.unlock();

		try_distribute_work();
	}

	void clear_all_works()
	{
		mtx.lock();
		for (auto& c : works)
			delete_mail(c.second);
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

	struct WindowPrivate : Window
	{
		std::string title;

#ifdef FLAME_WINDOWS
		HWND hWnd;

		HCURSOR cursors[CursorCount];
		CursorType cursor_type;
#elif FLAME_ANDROID
		android_app* android_state;
#endif

		bool dead;

#ifdef FLAME_WINDOWS
		WindowPrivate(const std::string& _title, const Vec2u& _size, uint _style)
		{
			title = _title;

			minimized = false;

			hWnd = 0;

			set_size(Vec2i(-1), _size, _style);

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

			key_listeners.impl = ListenerHubImpl::create();
			mouse_listeners.impl = ListenerHubImpl::create();
			resize_listeners.impl = ListenerHubImpl::create();
			destroy_listeners.impl = ListenerHubImpl::create();

			dead = false;
		}
#elif FLAME_ANDROID
		WindowPrivate(android_app* android_state) :
			android_state(android_state)
		{
		}
#endif

		~WindowPrivate()
		{
			destroy_listeners.call();

			ListenerHubImpl::destroy(key_listeners.impl);
			ListenerHubImpl::destroy(mouse_listeners.impl);
			ListenerHubImpl::destroy(resize_listeners.impl);
			ListenerHubImpl::destroy(destroy_listeners.impl);
		}

#ifdef FLAME_WINDOWS
		void set_cursor(CursorType type)
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

		void set_size(const Vec2i& _pos, const Vec2u& _size, int _style)
		{
			if (_size.x() > 0)
				size.x() = _size.x();
			if (_size.y() > 0)
				size.y() = _size.y();

			bool style_changed = false;
			if (_style != -1 && _style != style)
			{
				style = _style;
				style_changed = true;
			}

			assert(!(style & WindowFullscreen) || (!(style & WindowFrame) && !(style & WindowResizable)));

			Vec2u final_size;
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

			RECT rect = { 0, 0, size.x(), size.y() };
			AdjustWindowRect(&rect, win32_style, false);
			final_size.x() = rect.right - rect.left;
			final_size.y() = rect.bottom - rect.top;

			pos.x() = _pos.x() == -1 ? (screen_size.x() - final_size.x()) / 2 : _pos.x();
			pos.y() = _pos.y() == -1 ? (screen_size.y() - final_size.y()) / 2 : _pos.y();

			if (!hWnd)
			{
				hWnd = CreateWindowA("flame_wnd", title.c_str(), win32_style,
					pos.x(), pos.y(), final_size.x(), final_size.y(), NULL, NULL, (HINSTANCE)get_hinst(), NULL);
			}
			else
			{
				if (style_changed)
					SetWindowLong(hWnd, GWL_STYLE, win32_style);
				MoveWindow(hWnd, pos.x(), pos.y(), size.x(), size.y(), true);
			}
		}

		void set_maximized(bool v)
		{
			ShowWindow(hWnd, v ? SW_SHOWMAXIMIZED : SW_SHOWNORMAL);
		}
#endif

	};

	void* Window::get_native()
	{
#ifdef FLAME_WINDOWS
		return reinterpret_cast<WindowPrivate*>(this)->hWnd;
#elif FLAME_ANDROID
		return reinterpret_cast<WindowPrivate*>(this)->android_state;
#endif
	}

	const char* Window::title()
	{
		return ((WindowPrivate*)this)->title.c_str();
	}

	const void Window::set_title(const char* _title)
	{
		((WindowPrivate*)this)->title = _title;
	}

#ifdef FLAME_WINDOWS
	void Window::set_cursor(CursorType type)
	{
		reinterpret_cast<WindowPrivate*>(this)->set_cursor(type);
	}

	void Window::set_pos(const Vec2i& _pos)
	{
		pos = _pos;
		SetWindowPos(reinterpret_cast<WindowPrivate*>(this)->hWnd, HWND_TOP, pos.x(), pos.y(), 0, 0, SWP_NOSIZE | SWP_NOZORDER);
	}

	void Window::set_size(const Vec2i& _pos, const Vec2u& _size, int _style)
	{
		reinterpret_cast<WindowPrivate*>(this)->set_size(_pos, _size, _style);
	}

	void Window::set_maximized(bool v)
	{
		reinterpret_cast<WindowPrivate*>(this)->set_maximized(v);
	}
#endif

	void Window::close()
	{
		((WindowPrivate*)this)->dead = true;
	}

#ifdef FLAME_WINDOWS
	static LRESULT CALLBACK _wnd_proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		auto w = reinterpret_cast<WindowPrivate*>(GetWindowLongPtr(hWnd, 0));

		if (w)
		{
			switch (message)
			{
			case WM_KEYDOWN:
			{
				auto v = vk_code_to_key(wParam);
				if (v > 0)
					w->key_listeners.call(KeyStateDown, v);
			}
			break;
			case WM_KEYUP:
			{
				auto v = vk_code_to_key(wParam);
				if (v > 0)
					w->key_listeners.call(KeyStateUp, v);
			}
			break;
			case WM_CHAR:
				w->key_listeners.call(KeyStateNull, (Key)wParam);
				break;
			case WM_LBUTTONDOWN:
				w->mouse_listeners.call(KeyStateDown, Mouse_Left, Vec2i(LOWORD(lParam), HIWORD(lParam)));
			break;
			case WM_LBUTTONUP:
				w->mouse_listeners.call(KeyStateUp, Mouse_Left, Vec2i(LOWORD(lParam), HIWORD(lParam)));
			break;
			case WM_MBUTTONDOWN:
				w->mouse_listeners.call(KeyStateDown, Mouse_Middle, Vec2i(LOWORD(lParam), HIWORD(lParam)));
			break;
			case WM_MBUTTONUP:
				w->mouse_listeners.call(KeyStateUp, Mouse_Middle, Vec2i(LOWORD(lParam), HIWORD(lParam)));
			break;
			case WM_RBUTTONDOWN:
				w->mouse_listeners.call(KeyStateDown, Mouse_Right, Vec2i(LOWORD(lParam), HIWORD(lParam)));
			break;
			case WM_RBUTTONUP:
				w->mouse_listeners.call(KeyStateUp, Mouse_Right, Vec2i(LOWORD(lParam), HIWORD(lParam)));
			break;
			case WM_MOUSEMOVE:
				w->mouse_listeners.call(KeyStateNull, Mouse_Null, Vec2i(LOWORD(lParam), HIWORD(lParam)));
			break;
			case WM_MOUSEWHEEL:
				w->mouse_listeners.call(KeyStateNull, Mouse_Middle, Vec2i((short)HIWORD(wParam) > 0 ? 1 : -1, 0));
			break;
			case WM_DESTROY:
				w->dead = true;
			case WM_SIZE:
			{
				auto size = Vec2u((int)LOWORD(lParam), (int)HIWORD(lParam));
				w->minimized = (size.x() == 0 || size.y() == 0);
				if (size != w->size)
				{
					w->size = size;
					w->resize_listeners.call(size);
				}
			}
			break;
			}
		}

		return DefWindowProc(hWnd, message, wParam, lParam);
	}
#endif

	static std::vector<WindowPrivate*> windows;

	Window* Window::create(const char* title, const Vec2u& size, uint style)
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
				wcex.hIcon = CreateIcon(wcex.hInstance, icon_image->size.x(), icon_image->size.y(), 1,
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

		auto w = new WindowPrivate(title, size, style);
		windows.push_back(w);
		return w;
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
		(reinterpret_cast<ApplicationPrivate*>(app))->windows.push_back(w);

		android_state->userData = w;
		android_state->onAppCmd = android_handle_cmd;
		android_state->onInputEvent = android_handle_input;
		created_callback = callback;

		return w;
	}
#endif

	void Window::destroy(Window* w)
	{
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

	static Looper _looper;

	static ulonglong last_time;

	int Looper::loop(void (*idle_func)(void* c), const Mail<>& capture)
	{
		if (windows.size() == 0)
			return 1;

		last_time = get_now_ns();
		frame = 0;

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
			{
				delete_mail(capture);
				return 0;
			}

			idle_func(capture.p);

			frame++;
			auto et = last_time;
			last_time = get_now_ns();
			et = last_time - et;
			delta_time = et / 1000000000.f;
			total_time += delta_time;
		}
	}

	struct Event
	{
		uint id;
		bool repeatly;
		float interval;
		float rest;
		std::unique_ptr<Closure<void(void* c)>> event;
		void (*ending)(void* c);

		~Event()
		{
			if (ending)
				ending(event->capture.p);
		}
	};

	static std::list<std::unique_ptr<Event>> events;

	void* Looper::add_event(void (*event)(void* c), const Mail<>& capture, void (*ending)(void* c), bool repeatly, float interval, uint id, bool only)
	{
		if (only)
		{
			for (auto& e : events)
			{
				if (id == e->id)
					return nullptr;
			}
		}
		auto e = new Event;
		e->id = id;
		e->repeatly = repeatly;
		e->interval = interval;
		e->rest = interval;
		auto c = new Closure<void(void* c)>;
		c->function = event;
		c->capture = capture;
		e->event.reset(c);
		e->ending = ending;
		events.emplace_back(e);
		return c;
	}

	void Looper::remove_event(void* ret_by_add)
	{
		for (auto it = events.begin(); it != events.end(); it++)
		{
			if ((*it)->event.get() == ret_by_add)
			{
				it = events.erase(it);
				return;
			}
		}
	}

	void Looper::clear_events(int id)
	{
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

	void Looper::process_events()
	{
		for (auto it = events.begin(); it != events.end();)
		{
			auto& e = *it;
			e->rest -= delta_time;
			if (e->rest <= 0)
			{
				e->event->call();
				if (!e->repeatly)
				{
					it = events.erase(it);
					continue;
				}
				e->rest = e->interval;
			}
			it++;
		}
	}

	Looper& looper()
	{
		return _looper;
	}
}
