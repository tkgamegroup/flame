#include "system_private.h"

#include <process.h>
#include <shellapi.h>
#include <ShlObj.h>
#include <Shlwapi.h>
#include <CommCtrl.h>
#include <thumbcache.h>
#include <ImageHlp.h>

namespace flame
{

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

	void get_logical_drives(uint* count, wchar_t** names)
	{
		wchar_t buf[256];
		GetLogicalDriveStringsW(countof(buf), buf);

		auto n = 1;
		names[0] = buf;
		auto pstr = buf;
		while (true)
		{
			if (pstr[0] == 0 && pstr[1] == 0)
				break;
			if (pstr[0] == 0)
			{
				names[n] = pstr;
				n++;
			}
			pstr++;
		}

		*count = n;
	}

	void* get_hinst()
	{
		return GetModuleHandle(nullptr);
	}

	uvec2 get_screen_size()
	{
		uvec2 ret;
		ret.x = GetSystemMetrics(SM_CXSCREEN);
		ret.y = GetSystemMetrics(SM_CYSCREEN);
		return ret;
	}

	void* create_native_event(bool signaled, bool manual)
	{
		return CreateEvent(NULL, manual, signaled, NULL);
	}

	void set_native_event(void* ev)
	{
		SetEvent(ev);
	}

	void reset_native_event(void* ev)
	{
		ResetEvent(ev);
	}

	bool wait_native_event(void* ev, int timeout)
	{
		return WaitForSingleObject(ev, timeout < 0 ? INFINITE : timeout) == WAIT_OBJECT_0;
	}

	void destroy_native_event(void* ev)
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

	void get_module_dependencies(const wchar_t* filename, void (*callback)(Capture& c, const wchar_t* filename), const Capture& capture)
	{
		auto path = std::filesystem::path(filename);
		auto parent_path = path.parent_path();
		std::deque<std::filesystem::path> remains;
		std::vector<std::string> list;
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
					auto found = false;
					for (auto& s : list)
					{
						if (s == pstr)
						{
							found = true;
							break;
						}
					}
					if (!found)
					{
						remains.push_back(pstr);
						list.push_back(pstr);
					}

					importDesc++;
				}
			}
			ImageUnload(image);
		}
		for (auto i = (int)list.size() - 1; i >= 0; i--)
			callback((Capture&)capture, (parent_path / list[i]).c_str());

		f_free(capture._data);
	}

	void get_clipboard(void* str, wchar_t* (*str_allocator)(void* str, uint size))
	{
		OpenClipboard(NULL);
		auto hMemory = GetClipboardData(CF_UNICODETEXT);
		auto size = GlobalSize(hMemory) / sizeof(wchar_t) - 1;
		wcscpy(str_allocator(str, size), (wchar_t*)GlobalLock(hMemory));
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

	void get_thumbnail(uint width, const wchar_t* _filename, uint* out_width, uint* out_height, uchar** out_data)
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
			*out_data = (uchar*)f_malloc(bmp.bmWidth * bmp.bmHeight * 4);
			GetBitmapBits(hbmp, bmp.bmWidthBytes * bmp.bmHeight, *out_data);
			DeleteObject(hbmp);
		}

		thumbnail_provider->Release();

		desktop_folder->Release();
		shell_folder->Release();
	}

	void get_icon(const wchar_t* filename, int* out_id, uint* out_width, uint* out_height, uchar** out_data)
	{
		SHFILEINFOW file_info = {};
		auto path = std::filesystem::path(filename);
		path.make_preferred();
		SHGetFileInfoW(path.c_str(), 0, &file_info, sizeof(SHFILEINFOW), SHGFI_ICON | SHGFI_LARGEICON);

		if (file_info.hIcon == nullptr)
			return;

		if (out_id)
		{
			*out_id = file_info.iIcon;
			DestroyIcon(file_info.hIcon);
			return;
		}

		ICONINFO icon_info = { 0 };
		GetIconInfo(file_info.hIcon, &icon_info);

		if (icon_info.hbmColor)
		{
			DIBSECTION ds;
			GetObject(icon_info.hbmColor, sizeof(ds), &ds);
			int byte_size = ds.dsBm.bmWidth * ds.dsBm.bmHeight * (ds.dsBm.bmBitsPixel / 8);

			if (byte_size)
			{
				*out_width = ds.dsBm.bmWidth;
				*out_height = ds.dsBm.bmHeight;
				*out_data = (uchar*)f_malloc(byte_size);
				GetBitmapBits(icon_info.hbmColor, byte_size, *out_data);
			}
		}

		DestroyIcon(file_info.hIcon);
	}

	bool is_keyboard_key_pressing(KeyboardKey key)
	{
		return GetAsyncKeyState(key_to_vk_code(key)) & 0x8000;
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

	void set_mouse_pos(const ivec2& pos)
	{
		SetCursorPos(pos.x, pos.y);
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

	void exec(const wchar_t* filename, wchar_t* parameters, char* output)
	{
		bool ok;
		HANDLE hChildStd_OUT_Rd = NULL;
		HANDLE hChildStd_OUT_Wr = NULL;
		if (output)
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

		if (output)
		{
			DWORD size;
			PeekNamedPipe(hChildStd_OUT_Rd, NULL, NULL, NULL, &size, NULL);
			PeekNamedPipe(hChildStd_OUT_Rd, output, size, NULL, NULL, NULL);
			output[size] = 0;
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
		auto n = CaptureStackBackTrace(0, _countof(buf), buf, nullptr);
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

	void do_file_watch(void* event_end, bool all_changes, const std::wstring& path, void (*callback)(Capture& c, FileChangeType type, const wchar_t* filename), Capture& capture)
	{
		auto dir_handle = CreateFileW(path.c_str(), GENERIC_READ | GENERIC_WRITE | FILE_LIST_DIRECTORY, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED | FILE_FLAG_BACKUP_SEMANTICS, NULL);
		fassert(dir_handle != INVALID_HANDLE_VALUE);

		BYTE notify_buf[1024];

		OVERLAPPED overlapped;
		auto event_changed = create_native_event(false);

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

		destroy_native_event(event_changed);
		destroy_native_event(dir_handle);

		if (event_end)
			destroy_native_event(event_end);
	}

	void* add_file_watcher(const wchar_t* _path, void (*callback)(Capture& c, FileChangeType type, const wchar_t* filename), const Capture& capture, bool all_changes, bool sync)
	{
		std::filesystem::path path = _path;

		if (!sync)
		{
			auto ev = create_native_event(false);

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
}
