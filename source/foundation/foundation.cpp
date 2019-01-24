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

#include <flame/foundation/foundation.h>

#define NOMINMAX
#include <Windows.h>
#include <process.h>
#include <shellapi.h>
#include <ShlObj.h>
#include <Shlwapi.h>
#include <CommCtrl.h>
#include <thumbcache.h>

void *flame_malloc(int size)
{
	return malloc(size);
}

void *flame_realloc(void *p, int size)
{
	return realloc(p, size);
}

void flame_free(void *p)
{
	free(p);
}

namespace flame
{
	static std::vector<RegisteredFunction*> pfs;

	void register_function(uint id, PF pf, int parm_count, const char *filename, int line_beg, int line_end)
	{
		assert(id);
		for (auto &r : pfs)
		{
			if (r->id == id)
				assert(0);
		}

		auto r = new RegisteredFunction;
		r->id = id;
		r->pf = pf;
		r->parameter_count = parm_count;
		r->filename = filename;
		r->line_beg = line_beg;
		r->line_end = line_end;
		pfs.push_back(r);
	}

	RegisteredFunction *find_registered_function(uint id, PF pf)
	{
		if (id)
		{
			for (auto &r : pfs)
			{
				if (r->id == id)
					return r;
			}
			return nullptr;
		}
		else
		{
			for (auto &r : pfs)
			{
				if (r->pf == pf)
					return r;
			}
			return nullptr;
		}
		return nullptr;
	}

	static void do_thread(void *p)
	{
		auto f = (Function<>*)p;
		f->exec();
		delete f;
	}

	void thread(Function<> &_f)
	{
		auto f = new Function<>(_f);
		_beginthread(do_thread, 0, f);
	}

	void *get_hinst()
	{
		return GetModuleHandle(nullptr);
	}

	Ivec2 get_screen_size()
	{
		Ivec2 ret;
		ret.x = GetSystemMetrics(SM_CXSCREEN);
		ret.y = GetSystemMetrics(SM_CYSCREEN);
		return ret;
	}

	const wchar_t *get_curr_path()
	{
		static wchar_t buf[260];
		GetCurrentDirectoryW(sizeof(buf), buf);
		return buf;
	}

	const wchar_t *get_app_path()
	{
		static wchar_t buf[260];
		GetModuleFileNameW(nullptr, buf, sizeof(buf));
		auto path = std::filesystem::path(buf).parent_path().generic_wstring();
		wcscpy(buf, path.data());
		return buf;
	}

	void sleep(uint time)
	{
		Sleep(time < 0 ? INFINITE : time);
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

	void exec(const wchar_t *filename, const char *parameters, bool wait)
	{
		SHELLEXECUTEINFOW info = {};
		info.cbSize = sizeof(SHELLEXECUTEINFOW);
		info.fMask = SEE_MASK_NOCLOSEPROCESS;
		info.lpVerb = L"open";
		info.lpFile = filename;
		info.lpParameters = s2w(parameters).c_str();
		ShellExecuteExW(&info);
		if (wait)
			WaitForSingleObject(info.hProcess, INFINITE);

	}

	String exec_and_get_output(const wchar_t *filename, const char *parameters)
	{
		HANDLE hChildStd_OUT_Rd = NULL;
		HANDLE hChildStd_OUT_Wr = NULL;

		SECURITY_ATTRIBUTES saAttr;
		saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
		saAttr.bInheritHandle = TRUE;
		saAttr.lpSecurityDescriptor = NULL;

		assert(CreatePipe(&hChildStd_OUT_Rd, &hChildStd_OUT_Wr, &saAttr, 0));

		assert(SetHandleInformation(hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0));

		wchar_t cl_buf[1024 * 8];
		{
			auto tail = cl_buf;
			if (filename[0] == 0)
				tail = wcscpy(cl_buf, filename);
			wcscpy(tail, s2w(parameters).c_str());
		}
		cl_buf[FLAME_ARRAYSIZE(cl_buf) - 1] = 0;

		STARTUPINFOW start_info = {};
		start_info.cb = sizeof(STARTUPINFOW);
		start_info.hStdError = hChildStd_OUT_Wr;
		start_info.hStdOutput = hChildStd_OUT_Wr;
		start_info.dwFlags |= STARTF_USESTDHANDLES;
		PROCESS_INFORMATION proc_info = {};
		if (!CreateProcessW(filename[0] == 0 ? nullptr : filename, cl_buf, NULL, NULL, TRUE, 0, NULL, NULL, &start_info, &proc_info))
		{
			auto e = GetLastError();
			assert(0);
		}

		WaitForSingleObject(proc_info.hProcess, INFINITE);

		CloseHandle(proc_info.hProcess);
		CloseHandle(proc_info.hThread);

		DWORD output_size;
		String output;
		PeekNamedPipe(hChildStd_OUT_Rd, NULL, NULL, NULL, &output_size, NULL);
		output.resize(output_size);
		PeekNamedPipe(hChildStd_OUT_Rd, (void*)output.v, output_size, NULL, NULL, NULL);
		return output;
	}

	StringW get_clipboard()
	{
		OpenClipboard(NULL);
		auto hMemory = GetClipboardData(CF_UNICODETEXT);
		StringW output;
		output.resize(GlobalSize(hMemory) / sizeof(wchar_t) - 1);
		memcpy(output.v, GlobalLock(hMemory), sizeof(wchar_t) * output.size);
		GlobalUnlock(hMemory);
		CloseClipboard();
		return output;
	}

	void set_clipboard(const StringW &s)
	{
		auto size = sizeof(wchar_t) * (s.size + 1);
		auto hGlobalMemory = GlobalAlloc(GHND, size);
		memcpy(GlobalLock(hGlobalMemory), s.v, size);
		GlobalUnlock(hGlobalMemory);
		OpenClipboard(NULL);
		EmptyClipboard();
		SetClipboardData(CF_UNICODETEXT, hGlobalMemory);
		CloseClipboard();
	}

	void open_explorer_and_select(const wchar_t *filename)
	{
		auto pidl = ILCreateFromPathW(filename);
		if (pidl)
		{
			SHOpenFolderAndSelectItems(pidl, 0, 0, 0);
			ILFree(pidl);
		}
	}

	void move_to_trashbin(const wchar_t *filename)
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

	void read_process_memory(void *process, void *address, int size, void *dst)
	{
		SIZE_T ret_byte;
		assert(ReadProcessMemory(process, address, dst, size, &ret_byte));
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
			return Key_Null;
		}
#endif
		return Key_Null;
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
		bool use_modifier_shift;
		bool use_modifier_ctrl;
		bool use_modifier_alt;
		Function<GlobalKeyParm> callback;
	};

	static HHOOK global_key_hook = 0;
	static std::map<int, std::vector<std::unique_ptr<GlobalKeyListener>>> global_key_listeners;

	LRESULT CALLBACK global_key_callback(int nCode, WPARAM wParam, LPARAM lParam)
	{
		auto kbhook = (KBDLLHOOKSTRUCT*)lParam;

		auto it = global_key_listeners.find(vk_code_to_key(kbhook->vkCode));
		if (it != global_key_listeners.end())
		{
			for (auto &l : it->second)
			{
				if (l->use_modifier_shift && !(GetAsyncKeyState(VK_LSHIFT) || GetAsyncKeyState(VK_RSHIFT)))
					continue;
				if (l->use_modifier_ctrl && !(GetAsyncKeyState(VK_LCONTROL) || GetAsyncKeyState(VK_RCONTROL)))
					continue;
				if (l->use_modifier_alt && !(GetAsyncKeyState(VK_LMENU) || GetAsyncKeyState(VK_RMENU)))
					continue;
				auto action = KeyStateNull;
				if (wParam == WM_KEYDOWN)
					action = KeyStateDown;
				else if (wParam == WM_KEYUP)
					action = KeyStateUp;
				l->callback.p.action() = action;
				l->callback.exec();
			}
		}

		return CallNextHookEx(global_key_hook, nCode, wParam, lParam);
	}

	void *add_global_key_listener(Key key, bool modifier_shift, bool modifier_ctrl, bool modifier_alt, Function<GlobalKeyParm> &callback)
	{
		auto l = new GlobalKeyListener;
		l->callback = callback;
		l->use_modifier_shift = modifier_shift;
		l->use_modifier_ctrl = modifier_ctrl;
		l->use_modifier_alt = modifier_alt;

		auto it = global_key_listeners.find(key);
		if (it == global_key_listeners.end())
			it = global_key_listeners.emplace(key, std::vector<std::unique_ptr<GlobalKeyListener>>()).first;
		it->second.emplace_back(l);

		if (global_key_hook == 0)
			global_key_hook = SetWindowsHookEx(WH_KEYBOARD_LL, global_key_callback, (HINSTANCE)get_hinst(), 0);

		return l;
	}

	void remove_global_key_listener(int key, void *handle)
	{
		auto it = global_key_listeners.find(key);
		if (it == global_key_listeners.end())
			return;

		for (auto _it = it->second.begin(); _it != it->second.end(); _it++)
		{
			if ((*_it).get() == handle)
			{
				it->second.erase(_it);
				break;
			}
		}

		if (it->second.empty())
			global_key_listeners.erase(it);

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

	void do_file_watch(FileWatcher *filewatcher, bool only_content, const wchar_t *path, Function<FileWatcherParm> &callback)
	{
		auto dir_handle = CreateFileW(path, GENERIC_READ | GENERIC_WRITE | FILE_LIST_DIRECTORY, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED | FILE_FLAG_BACKUP_SEMANTICS, NULL);
		assert(dir_handle != INVALID_HANDLE_VALUE);

		BYTE notify_buf[1024];

		OVERLAPPED overlapped;
		auto hEvent = CreateEvent(NULL, false, false, NULL);

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
				{
					callback.p.type() = type;
					callback.p.filename() = p->FileName;
					callback.exec();
				}

				if (p->NextEntryOffset <= 0)
					break;
				base += p->NextEntryOffset;
				p = (FILE_NOTIFY_INFORMATION*)(notify_buf + base);
			}
		}

		CloseHandle(hEvent);
		CloseHandle(dir_handle);
	}

	FLAME_PACKAGE_BEGIN(FileWatcherThreadC)
		FLAME_PACKAGE_ITEM(wcharptr, filepath, p)
		FLAME_PACKAGE_ITEM(FileWatcherPtr, filewatcher, p)
		FLAME_PACKAGE_ITEM(voidptr, pcallback, p) /* do convert yourself */
	FLAME_PACKAGE_END

	FileWatcher *add_file_watcher(const wchar_t *path, Function<FileWatcherParm> &callback, int options)
	{
		if (options & FileWatcherAsynchronous)
		{
			auto w = new FileWatcher;
			w->options = options;
			w->hEventExpired = CreateEvent(NULL, false, false, NULL);

			auto pcallback = new Function<FileWatcherParm>;
			*pcallback = callback;

			thread(Function<>([](Package &p) {
				auto c = p.get_capture<FileWatcherThreadC>();
				do_file_watch(c.filewatcher(), c.filewatcher()->options & FileWatcherMonitorOnlyContentChanged, c.filepath(), (*(Function<FileWatcherParm>*)c.pcallback()));
				delete c.filewatcher();
				delete c.pcallback();
			}, { (wcharptr)path, w, pcallback }));

			return w;
		}
		else
		{
			do_file_watch(nullptr, options & FileWatcherMonitorOnlyContentChanged, path, callback);

			return nullptr;
		}

		return nullptr;
	}

	void remove_file_watcher(FileWatcher *w)
	{
		SetEvent(w->hEventExpired);
	}

	void get_thumbnai(int width, const wchar_t *_filename, int *out_width, int *out_height, char **out_data)
	{
		std::filesystem::path path(_filename);
		path.make_preferred();
		auto filename = path.wstring();

		HRESULT hr;

		IShellFolder *desktop_folder, *shell_folder;
		SHGetDesktopFolder(&desktop_folder);

		LPITEMIDLIST pidl;
		hr = desktop_folder->ParseDisplayName(NULL, NULL, (wchar_t*)filename.c_str(), NULL, &pidl, NULL);
		SHBindToParent(pidl, IID_PPV_ARGS(&shell_folder), NULL);
		auto pidl_child = ILFindLastID(pidl);

		IThumbnailProvider *thumbnail_provider;
		hr = shell_folder->GetUIObjectOf(NULL, 1, (LPCITEMIDLIST *)&pidl_child, IID_IThumbnailProvider, NULL, (void **)&thumbnail_provider);
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
