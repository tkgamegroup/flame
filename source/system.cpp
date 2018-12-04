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

#include <flame/bitmap.h>
#include <flame/system.h>

#define NOMINMAX
#include <Windows.h>
#include <shellapi.h>
#include <ShlObj.h>
#include <Shlwapi.h>
#include <CommCtrl.h>
#include <thumbcache.h>

#include <map>
#include <experimental/filesystem>
#include <assert.h>

namespace filesystem = std::experimental::filesystem;

namespace flame
{
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
		auto path = filesystem::path(buf).parent_path().generic_wstring();
		wcscpy(buf, path.data());
		return buf;
	}

	void exec(const wchar_t *filename, const wchar_t *parameters, bool wait)
	{
		SHELLEXECUTEINFOW info = {};
		info.cbSize = sizeof(SHELLEXECUTEINFOW);
		info.fMask = SEE_MASK_NOCLOSEPROCESS;
		info.lpVerb = L"open";
		info.lpFile = filename;
		info.lpParameters = parameters;
		ShellExecuteExW(&info);
		if (wait)
			WaitForSingleObject(info.hProcess, INFINITE);

	}

	String exec_and_get_output(const wchar_t *filename, const wchar_t *parameters)
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
			wcscpy(tail, parameters);
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

	//struct FileWatcher
	//{
	//	void *hEventExpired;
	//};

	//static void do_file_watcher(CommonData *d)
	//{
	//	auto &filepath = *(const wchar_t **)&d[0].p();
	//	auto &w = *(FileWatcher**)&d[1].p();
	//	auto &f = *(Function**)&d[2].p();

	//	auto dir_handle = CreateFileW(filepath, GENERIC_READ | GENERIC_WRITE |
	//		FILE_LIST_DIRECTORY, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
	//		OPEN_EXISTING,
	//		FILE_FLAG_OVERLAPPED | FILE_FLAG_BACKUP_SEMANTICS,
	//		NULL);
	//	assert(dir_handle != INVALID_HANDLE_VALUE);

	//	BYTE notify_buf[1024];

	//	OVERLAPPED overlapped = {};
	//	auto hEvent = CreateEvent(NULL, false, false, NULL);

	//	auto flags = FILE_NOTIFY_CHANGE_FILE_NAME |
	//		FILE_NOTIFY_CHANGE_DIR_NAME |
	//		FILE_NOTIFY_CHANGE_CREATION |
	//		FILE_NOTIFY_CHANGE_LAST_WRITE;

	//	while (true)
	//	{
	//		ZeroMemory(&overlapped, sizeof(OVERLAPPED));
	//		overlapped.hEvent = hEvent;

	//		assert(ReadDirectoryChangesW(dir_handle, notify_buf, sizeof(notify_buf), true, flags,
	//			NULL, &overlapped, NULL));

	//		HANDLE events[] = {
	//			overlapped.hEvent,
	//			w->hEventExpired
	//		};

	//		if (WaitForMultipleObjects(2, events, false, INFINITE) - WAIT_OBJECT_0 == 1)
	//		{
	//			CloseHandle(dir_handle);
	//			delete w;
	//			Function::destroy(f);
	//			break;
	//		}

	//		DWORD ret_bytes;
	//		assert(GetOverlappedResult(dir_handle, &overlapped, &ret_bytes, false) == 1);

	//		auto base = 0;
	//		auto p = (FILE_NOTIFY_INFORMATION*)notify_buf;
	//		while (true)
	//		{
	//			FileChangeType type;
	//			switch (p->Action)
	//			{
	//			case 0x1:
	//				type = FileAdded;
	//				break;
	//			case 0x2:
	//				type = FileRemoved;
	//				break;
	//			case 0x3:
	//				type = FileModified;
	//				break;
	//			case 0x4:
	//				type = FileRenamed;
	//				break;
	//			case 0x5:
	//				type = FileRenamed;
	//				break;
	//			}
	//			f->datas[0].i1() = type;
	//			f->datas[1].p() = p->FileName;
	//			f->exec();

	//			if (p->NextEntryOffset <= 0)
	//				break;
	//			base += p->NextEntryOffset;
	//			p = (FILE_NOTIFY_INFORMATION*)(notify_buf + base);
	//		}
	//	}
	//}

	//FileWatcher *add_file_watcher(FileWatcherMode mode, const wchar_t *filepath, PF pf, const std::vector<CommonData> &capt)
	//{
	//	auto w = new FileWatcher;
	//	w->hEventExpired = CreateEvent(NULL, false, false, NULL);

	//	auto callback = Function::create(pf, "i p", capt);

	//	auto f_thread = Function::create(do_file_watcher, "p p p", {});

	//	f_thread->datas[0].p() = (wchar_t*)filepath;
	//	f_thread->datas[1].p() = w;
	//	f_thread->datas[2].p() = callback;

	//	f_thread->exec_in_new_thread();

	//	return w;
	//}

	//void remove_file_watcher(FileWatcher *w)
	//{
	//	SetEvent(w->hEventExpired);
	//}

	//void read_process_memory(void *process, void *address, int size, void *dst)
	//{
	//	SIZE_T ret_byte;
	//	assert(ReadProcessMemory(process, address, dst, size, &ret_byte));
	//}

	//static HHOOK global_key_hook = 0;
	//static std::map<int, std::vector<Function*>> global_key_listeners;

	//LRESULT CALLBACK global_key_callback(int nCode, WPARAM wParam, LPARAM lParam)
	//{
	//	auto kbhook = (KBDLLHOOKSTRUCT*)lParam;

	//	auto it = global_key_listeners.find(kbhook->vkCode);
	//	if (it != global_key_listeners.end())
	//	{
	//		for (auto &f : it->second)
	//			f->exec();
	//	}

	//	return CallNextHookEx(global_key_hook, nCode, wParam, lParam);
	//}

	//Function *add_global_key_listener(int key, PF pf, const std::vector<CommonData> &capt)
	//{
	//	auto f = Function::create(pf, "", capt);

	//	auto it = global_key_listeners.find(key);
	//	if (it == global_key_listeners.end())
	//		it = global_key_listeners.emplace(key, std::vector<Function*>()).first;
	//	it->second.push_back(f);

	//	if (global_key_hook == 0)
	//		global_key_hook = SetWindowsHookEx(WH_KEYBOARD_LL, global_key_callback, (HINSTANCE)get_hinst(), 0);

	//	return f;
	//}

	//void remove_global_key_listener(int key, Function *f)
	//{
	//	auto it = global_key_listeners.find(key);
	//	if (it == global_key_listeners.end())
	//		return;

	//	for (auto _it = it->second.begin(); _it != it->second.end(); _it++)
	//	{
	//		if ((*_it) == f)
	//		{
	//			Function::destroy(f);
	//			it->second.erase(_it);
	//			break;
	//		}
	//	}

	//	if (it->second.empty())
	//		global_key_listeners.erase(it);

	//	if (global_key_listeners.empty())
	//	{
	//		if (global_key_hook)
	//		{
	//			UnhookWindowsHookEx(global_key_hook);
	//			global_key_hook = 0;
	//		}
	//	}
	//}

	void get_thumbnai(int width, const wchar_t *_filename, int *out_width, int *out_height, char **out_data)
	{
		filesystem::path path(_filename);
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
}
