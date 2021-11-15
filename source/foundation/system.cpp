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

	std::filesystem::path get_app_path(bool has_name)
	{
		wchar_t buf[256];
		GetModuleFileNameW(nullptr, buf, 256);
		auto path = std::filesystem::path(buf);
		if (!has_name)
			path = path.parent_path();
		return path;
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

	std::vector<std::filesystem::path> get_module_dependencies(const std::filesystem::path& path)
	{
		auto parent_path = path.parent_path();
		std::deque<std::filesystem::path> remains;
		std::vector<std::filesystem::path> ret;
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
					for (auto& s : ret)
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
						ret.push_back(pstr);
					}

					importDesc++;
				}
			}
			ImageUnload(image);
		}
		for (auto& p : ret)
			p = parent_path / p;
		std::reverse(ret.begin(), ret.end());
		return ret;
	}

	std::wstring get_clipboard()
	{
		OpenClipboard(NULL);
		auto hMemory = GetClipboardData(CF_UNICODETEXT);
		auto size = GlobalSize(hMemory) / sizeof(wchar_t) - 1;
		std::wstring ret;
		ret.resize(size);
		wcsncpy(ret.data(), (wchar_t*)GlobalLock(hMemory), size);
		GlobalUnlock(hMemory);
		CloseClipboard();
		return ret;
	}

	void set_clipboard(std::wstring_view str)
	{
		auto hGlobalMemory = GlobalAlloc(GHND, str.size());
		memcpy(GlobalLock(hGlobalMemory), str.data(), str.size());
		GlobalUnlock(hGlobalMemory);
		OpenClipboard(NULL);
		EmptyClipboard();
		SetClipboardData(CF_UNICODETEXT, hGlobalMemory);
		CloseClipboard();
	}

	std::pair<uvec2, std::unique_ptr<uchar>> get_thumbnail(uint width, const std::filesystem::path& path)
	{
		HRESULT hr;

		IShellFolder* desktop_folder, * shell_folder;
		SHGetDesktopFolder(&desktop_folder);

		LPITEMIDLIST pidl;
		hr = desktop_folder->ParseDisplayName(NULL, NULL, (wchar_t*)path.c_str(), NULL, &pidl, NULL);
		SHBindToParent(pidl, IID_PPV_ARGS(&shell_folder), NULL);
		auto pidl_child = ILFindLastID(pidl);

		IThumbnailProvider* thumbnail_provider;
		hr = shell_folder->GetUIObjectOf(NULL, 1, (LPCITEMIDLIST*)&pidl_child, IID_IThumbnailProvider, NULL, (void**)&thumbnail_provider);
		HBITMAP hbmp;
		WTS_ALPHATYPE alpha_type;
		uint w = 0, h = 0;
		std::unique_ptr<uchar> data;
		if (SUCCEEDED(thumbnail_provider->GetThumbnail(width, &hbmp, &alpha_type)))
		{
			BITMAP bmp;
			GetObject(hbmp, sizeof(bmp), &bmp);
			w = bmp.bmWidth;
			h = bmp.bmHeight;
			data.reset((uchar*)f_malloc(bmp.bmWidth * bmp.bmHeight * 4));
			GetBitmapBits(hbmp, bmp.bmWidthBytes * bmp.bmHeight, data.get());
			DeleteObject(hbmp);
		}

		thumbnail_provider->Release();

		desktop_folder->Release();
		shell_folder->Release();

		return std::make_pair(uvec2(w, h), std::move(data));
	}

	std::pair<uvec2, std::unique_ptr<uchar>> get_icon(const std::filesystem::path& path, int* out_id)
	{
		SHFILEINFOW file_info = {};
		SHGetFileInfoW(path.c_str(), 0, &file_info, sizeof(SHFILEINFOW), SHGFI_ICON | SHGFI_LARGEICON);

		if (file_info.hIcon == nullptr)
			return std::make_pair(uvec2(0), std::unique_ptr<uchar>());

		if (out_id)
		{
			*out_id = file_info.iIcon;
			DestroyIcon(file_info.hIcon);
			return std::make_pair(uvec2(0), std::unique_ptr<uchar>());
		}

		ICONINFO icon_info = { 0 };
		GetIconInfo(file_info.hIcon, &icon_info);

		uint w = 0, h = 0;
		std::unique_ptr<uchar> data;
		if (icon_info.hbmColor)
		{
			DIBSECTION ds;
			GetObject(icon_info.hbmColor, sizeof(ds), &ds);
			int byte_size = ds.dsBm.bmWidth * ds.dsBm.bmHeight * (ds.dsBm.bmBitsPixel / 8);

			if (byte_size)
			{
				w = ds.dsBm.bmWidth;
				h = ds.dsBm.bmHeight;
				data.reset((uchar*)f_malloc(byte_size));
				GetBitmapBits(icon_info.hbmColor, byte_size, data.get());
			}
		}

		DestroyIcon(file_info.hIcon);

		return std::make_pair(uvec2(w, h), std::move(data));
	}

	bool is_keyboard_pressing(KeyboardKey key)
	{
		return GetAsyncKeyState(key_to_vk_code(key)) & 0x8000;
	}

	void send_keyboard_event(KeyboardKey key, bool down)
	{
		keybd_event(key_to_vk_code(key), 0, down ? 0 : KEYEVENTF_KEYUP, 0);
	}

	void send_mouse_event(MouseKey key, bool down)
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

	void shell_exec(const std::filesystem::path& filename, const std::wstring& parameters, bool wait, bool show)
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

	void exec(const std::filesystem::path& filename, const std::wstring& parameters, std::string* output)
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
		if (!CreateProcessW(filename.c_str(), (wchar_t*)parameters.c_str(), NULL, NULL, TRUE, 0, NULL, NULL, &start_info, &proc_info))
		{
			auto e = GetLastError();
			assert(0);
		}

		WaitForSingleObject(proc_info.hProcess, INFINITE);

		CloseHandle(proc_info.hProcess);
		CloseHandle(proc_info.hThread);

		if (output)
		{
			DWORD size;
			PeekNamedPipe(hChildStd_OUT_Rd, NULL, NULL, NULL, &size, NULL);
			output->resize(size);
			PeekNamedPipe(hChildStd_OUT_Rd, output->data(), size, NULL, NULL, NULL);
		}
	}

	void debug_break()
	{
#ifdef _DEBUG
		DebugBreak();
#endif
	}

	std::vector<void*> get_call_frames()
	{
		void* buf[64];
		auto n = CaptureStackBackTrace(0, _countof(buf), buf, nullptr);
		std::vector<void*> ret;
		ret.resize(n);
		memcpy(ret.data(), buf, sizeof(void*) * n);
		return ret;
	}

	std::vector<StackFrameInfo> get_call_frames_infos(const std::vector<void*>& frames)
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
		std::vector<StackFrameInfo> ret;
		ret.resize(frames.size());
		for (auto i = 0; i < frames.size(); i++)
		{
			auto frame = (DWORD64)frames[i];
			SymFromAddr(process, frame, nullptr, symbol);
			if (SymGetLineFromAddr64(process, (DWORD64)frame, &displacement, line))
			{
				auto& info = ret[i];
				info.file = line->FileName;
				info.lineno = line->LineNumber;
				info.function = symbol->Name;
			}
		}
		delete[] line;
		delete[] symbol;
		return ret;
	}

	struct GlobalKeyListener
	{
		KeyboardKey key;
		std::function<void(bool down)> callback;
	};

	static HHOOK global_key_hook = 0;
	static std::list<GlobalKeyListener> global_key_listeners;

	LRESULT CALLBACK global_key_callback(int nCode, WPARAM wParam, LPARAM lParam)
	{
		auto kbhook = (KBDLLHOOKSTRUCT*)lParam;

		auto key = vk_code_to_key(kbhook->vkCode);

		for (auto& l : global_key_listeners)
		{
			if (l.key != key)
				continue;
			l.callback(wParam != WM_KEYUP);
		}

		return CallNextHookEx(global_key_hook, nCode, wParam, lParam);
	}

	void* add_global_key_listener(KeyboardKey key, const std::function<void(bool down)>& callback)
	{
		auto& l = global_key_listeners.emplace_back();
		l.key = key;
		l.callback = callback;

		if (!global_key_hook)
			global_key_hook = SetWindowsHookEx(WH_KEYBOARD_LL, global_key_callback, (HINSTANCE)get_hinst(), 0);

		return &l;
	}

	void remove_global_key_listener(void* lis)
	{
		for (auto it = global_key_listeners.begin(); it != global_key_listeners.end(); it++)
		{
			if (&(*it) == lis)
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

	void do_file_watch(void* event_end, bool all_changes, const std::filesystem::path& path, const std::function<void(FileChangeType type, const std::filesystem::path& path)>& callback)
	{
		auto dir_handle = CreateFileW(path.c_str(), GENERIC_READ | GENERIC_WRITE | FILE_LIST_DIRECTORY, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED | FILE_FLAG_BACKUP_SEMANTICS, NULL);
		assert(dir_handle != INVALID_HANDLE_VALUE);

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
					callback(type, path / p->FileName);

				if (p->NextEntryOffset <= 0)
					break;
				base += p->NextEntryOffset;
				p = (FILE_NOTIFY_INFORMATION*)(notify_buf + base);
			}
		}

		destroy_native_event(event_changed);
		destroy_native_event(dir_handle);

		if (event_end)
			destroy_native_event(event_end);
	}

	void* add_file_watcher(const std::filesystem::path& path, const std::function<void(FileChangeType type, const std::filesystem::path& path)>& callback, bool all_changes, bool sync)
	{
		if (!sync)
		{
			auto ev = create_native_event(false);

			std::thread([=]() {
				do_file_watch(ev, all_changes, path, callback);
			}).detach();

			return ev;
		}
		else
		{
			do_file_watch(nullptr, all_changes, path, callback);

			return nullptr;
		}

		return nullptr;
	}
}
