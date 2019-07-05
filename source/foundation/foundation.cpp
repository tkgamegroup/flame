#include <flame/foundation/foundation.h>

#define NOMINMAX
#include <Windows.h>
#include <process.h>
#include <ImageHlp.h>
#include <shellapi.h>
#include <ShlObj.h>
#include <Shlwapi.h>
#include <CommCtrl.h>
#include <thumbcache.h>

void* flame_malloc(unsigned int size)
{
	return malloc(size);
}

void* flame_realloc(void* p, unsigned int size)
{
	return realloc(p, size);
}

void flame_free(void *p)
{
	free(p);
}

namespace flame
{
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

	Mail<std::wstring> get_curr_path()
	{
		wchar_t buf[260];
		GetCurrentDirectoryW(sizeof(buf), buf);
		auto ret = new_mail<std::wstring>();
		(*ret.p) = buf;
		return ret;
	}

	Mail<std::wstring> get_app_path()
	{
		wchar_t buf[260];
		GetModuleFileNameW(nullptr, buf, sizeof(buf));
		auto ret = new_mail<std::wstring>();
		(*ret.p) = std::fs::path(buf).parent_path().generic_wstring();
		return ret;
	}

	void com_init()
	{
		static bool inited = false;
		if (inited)
			return;
		assert(SUCCEEDED(CoInitialize(NULL)));
		inited = true;
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
		return WaitForSingleObject(ev, timeout < 0 ? INFINITE : timeout) == 0;
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

	void exec(const std::wstring& filename, const std::wstring& parameters, bool wait, bool show)
	{
		SHELLEXECUTEINFOW info = {};
		info.cbSize = sizeof(SHELLEXECUTEINFOW);
		info.fMask = SEE_MASK_NOCLOSEPROCESS;
		info.lpVerb = L"open";
		info.lpFile = filename.c_str();
		info.nShow = show ? SW_SHOW : SW_HIDE;
		info.lpParameters = parameters.c_str();
		ShellExecuteExW(&info);
		if (wait)
			WaitForSingleObject(info.hProcess, INFINITE);
	}

	Mail<std::string> exec_and_get_output(const std::wstring& filename, const std::wstring& parameters)
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
		if (!CreateProcessW(filename[0] == 0 ? nullptr : filename.c_str(), (wchar_t*)parameters.c_str(), NULL, NULL, TRUE, 0, NULL, NULL, &start_info, &proc_info))
		{
			auto e = GetLastError();
			assert(0);
		}

		WaitForSingleObject(proc_info.hProcess, INFINITE);

		CloseHandle(proc_info.hProcess);
		CloseHandle(proc_info.hThread);

		DWORD output_size;
		auto output = new_mail<std::string>();
		PeekNamedPipe(hChildStd_OUT_Rd, NULL, NULL, NULL, &output_size, NULL);
		output.p->resize(output_size);
		PeekNamedPipe(hChildStd_OUT_Rd, (void*)output.p->data(), output_size, NULL, NULL, NULL);
		return output;
	}

	Mail<std::string> compile_to_dll(const std::vector<std::wstring>& sources, const std::vector<std::wstring>& libraries, const std::wstring& out)
	{
		std::wstring cl(L"\"");
		cl += s2w(VS_LOCATION);
		cl += L"/VC/Auxiliary/Build/vcvars64.bat\"";

		cl += L" & cl ";
		for (auto& s : sources)
			cl += s + L" ";
		cl += L"-LD -MD -EHsc -Zi -std:c++17 -I ../include -link -DEBUG ";
		for (auto& l : libraries)
			cl += l + L" ";

		cl += L" -out:" + out;

		return exec_and_get_output(L"", cl.c_str());
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

	Mail<std::vector<std::string>> get_module_dependancies(const std::wstring& module_name)
	{
		PLOADED_IMAGE image = ImageLoad(w2s(module_name).c_str(), std::fs::path(module_name).parent_path().string().c_str());

		auto ret = new_mail<std::vector<std::string>>();
		if (image->FileHeader->OptionalHeader.NumberOfRvaAndSizes >= 2) 
		{
			PIMAGE_IMPORT_DESCRIPTOR importDesc = (PIMAGE_IMPORT_DESCRIPTOR)get_ptr_from_rva(
					image->FileHeader->OptionalHeader.DataDirectory[1].VirtualAddress,
					image->FileHeader, image->MappedAddress);
			while (true)
			{
				if ((importDesc->TimeDateStamp == 0) && (importDesc->Name == 0))
					break;

				ret.p->push_back((char*)get_ptr_from_rva(importDesc->Name,
					image->FileHeader,
					image->MappedAddress));
				importDesc++;
			}
		}
		ImageUnload(image);
		return ret;
	}

	void* load_module(const std::wstring& module_name)
	{
		return LoadLibraryW(module_name.c_str());
	}

	void free_module(void* library)
	{
		FreeLibrary((HMODULE)library);
	}

	Mail<std::wstring> get_clipboard()
	{
		OpenClipboard(NULL);
		auto hMemory = GetClipboardData(CF_UNICODETEXT);
		auto output = new_mail<std::wstring>();
		output.p->resize(GlobalSize(hMemory) / sizeof(wchar_t) - 1);
		memcpy(output.p->data(), GlobalLock(hMemory), sizeof(wchar_t)*output.p->size());
		GlobalUnlock(hMemory);
		CloseClipboard();
		return output;
	}

	void set_clipboard(const std::wstring& s)
	{
		auto size = sizeof(wchar_t) * (s.size() + 1);
		auto hGlobalMemory = GlobalAlloc(GHND, size);
		memcpy(GlobalLock(hGlobalMemory), s.data(), size);
		GlobalUnlock(hGlobalMemory);
		OpenClipboard(NULL);
		EmptyClipboard();
		SetClipboardData(CF_UNICODETEXT, hGlobalMemory);
		CloseClipboard();
	}

	void open_explorer_and_select(const std::wstring& filename)
	{
		auto pidl = ILCreateFromPathW(filename.c_str());
		if (pidl)
		{
			SHOpenFolderAndSelectItems(pidl, 0, 0, 0);
			ILFree(pidl);
		}
	}

	void move_to_trashbin(const std::wstring& filename)
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

	void get_thumbnai(uint width, const std::wstring& _filename, uint* out_width, uint* out_height, char** out_data)
	{
		std::fs::path path(_filename);
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
		hr = shell_folder->GetUIObjectOf(NULL, 1, (LPCITEMIDLIST*)& pidl_child, IID_IThumbnailProvider, NULL, (void**)& thumbnail_provider);
		HBITMAP hbmp;
		WTS_ALPHATYPE alpha_type;
		thumbnail_provider->GetThumbnail(width, &hbmp, &alpha_type);
		thumbnail_provider->Release();

		BITMAP bmp;
		GetObject(hbmp, sizeof(bmp), &bmp);
		*out_width = bmp.bmWidth;
		*out_height = bmp.bmHeight;
		*out_data = new char[bmp.bmWidth * bmp.bmHeight * 4];
		GetBitmapBits(hbmp, bmp.bmWidthBytes * bmp.bmHeight, *out_data);
		DeleteObject(hbmp);

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

	struct FileWatcher
	{
		int options;
		void *hEventExpired;

		~FileWatcher()
		{
			CloseHandle(hEventExpired);
		}
	};

	void do_file_watch(FileWatcher *filewatcher, bool only_content, const std::wstring& _path, void (*callback)(void* c, FileChangeType type, const std::wstring& filename), const Mail<>& capture)
	{
		auto path = std::wstring(_path);

		if (path.empty())
		{
			auto curr_path = get_curr_path();
			path = *curr_path.p;
			delete_mail(curr_path);
		}
		auto dir_handle = CreateFileW(path.c_str(), GENERIC_READ | GENERIC_WRITE | FILE_LIST_DIRECTORY, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED | FILE_FLAG_BACKUP_SEMANTICS, NULL);
		assert(dir_handle != INVALID_HANDLE_VALUE);

		BYTE notify_buf[1024];

		OVERLAPPED overlapped;
		auto hEvent = create_event(false);

		auto flags = (only_content ? 0 : FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_CREATION) | FILE_NOTIFY_CHANGE_LAST_WRITE;

		while (true)
		{
			ZeroMemory(&overlapped, sizeof(OVERLAPPED));
			overlapped.hEvent = hEvent;

			assert(ReadDirectoryChangesW(dir_handle, notify_buf, sizeof(notify_buf), true, flags, NULL, &overlapped, NULL));

			if (filewatcher)
			{
				HANDLE events[] = {
					overlapped.hEvent,
					filewatcher->hEventExpired
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

				if (!(only_content && (type != FileModified)))
					callback(capture.p, type, !path.empty() ? (path + L"\\" + p->FileName).c_str() : p->FileName);

				if (p->NextEntryOffset <= 0)
					break;
				base += p->NextEntryOffset;
				p = (FILE_NOTIFY_INFORMATION*)(notify_buf + base);
			}
		}

		delete_mail(capture);

		CloseHandle(hEvent);
		CloseHandle(dir_handle);
	}

	FileWatcher *add_file_watcher(const std::wstring& path, void (*callback)(void* c, FileChangeType type, const std::wstring& filename), const Mail<>& capture, int options)
	{
		if (options & FileWatcherAsynchronous)
		{
			auto w = new FileWatcher;
			w->options = options;
			w->hEventExpired = CreateEvent(NULL, false, false, NULL);

			std::thread([=]() {
				do_file_watch(w, w->options & FileWatcherMonitorOnlyContentChanged, path, callback, capture);
				delete w;
			}).detach();

			return w;
		}
		else
		{
			do_file_watch(nullptr, options & FileWatcherMonitorOnlyContentChanged, path, callback, capture);

			return nullptr;
		}

		return nullptr;
	}

	void remove_file_watcher(FileWatcher *w)
	{
		set_event(w->hEventExpired);
	}

	/*
	const auto all_workers = 3;
	static std::mutex mtx;
	static std::condition_variable cv;
	static auto workers = all_workers;

	static std::vector<Function*> works;

	void try_distribute();

	static void do_work(CommonData *d)
	{
		((Function*)d[0].p)->exec();

		Function::destroy((Function*)d[0].p);
		Function::destroy((Function*)d[1].p);

		mtx.lock();
		workers++;
		cv.notify_one();
		mtx.unlock();

		try_distribute();
	}

	void try_distribute()
	{
		mtx.lock();
		if (!works.empty() && workers > 0)
		{
			workers--;
			auto w = works.front();
			works.erase(works.begin());

			auto f_thread = Function::create(do_work, "p:work p:thread", "", 0);
			f_thread->datas[0].p = w;
			f_thread->datas[1].p = f_thread;
			f_thread->exec_in_new_thread();
		}
		mtx.unlock();
	}

	void add_work(PF pf, char *capture_fmt, ...)
	{
		va_list ap;
		va_start(ap, capture_fmt);
		auto f = Function::create(pf, "", capture_fmt, ap);
		va_end(ap);

		mtx.lock();
		works.push_back(f);
		mtx.unlock();

		try_distribute();
	}

	void clear_works()
	{
		std::unique_lock<std::mutex> lk(mtx);

		works.clear();
		while (workers != all_workers)
			cv.wait(lk);
	}
	*/
}
