#pragma once

#include "foundation.h"

namespace flame
{
	struct StackFrameInfo
	{
		std::string file;
		uint lineno;
		std::string function;
	};

	enum FileChangeFlags
	{
		FileAdded = 1 << 0,
		FileRemoved = 1 << 1,
		FileModified = 1 << 2,
		FileRenamed = 1 << 3
	};

	FLAME_FOUNDATION_API void sleep(int time);
	FLAME_FOUNDATION_API GUID generate_guid();
	FLAME_FOUNDATION_API uint64 performance_counter();
	FLAME_FOUNDATION_API uint64 performance_frequency();
	// "Desktop"
	// "My Document" - typical: C:\Documents and Settings\User\My Documents
	// "Visual Studio Installation Location"
	FLAME_FOUNDATION_API std::filesystem::path get_special_path(std::string_view type);
	FLAME_FOUNDATION_API std::vector<std::filesystem::path> get_drives();
	FLAME_FOUNDATION_API void directory_lock(const std::filesystem::path& path, bool lock);
	FLAME_FOUNDATION_API void move_to_recycle_bin(const std::filesystem::path& path);
	FLAME_FOUNDATION_API std::filesystem::path get_app_path(bool has_name = false);
	FLAME_FOUNDATION_API void* get_hinst();
	FLAME_FOUNDATION_API uvec2 get_screen_size();
	FLAME_FOUNDATION_API void* load_library(const std::filesystem::path& path);
	FLAME_FOUNDATION_API void free_library(void* library);
	FLAME_FOUNDATION_API void* get_library_function(void* library, const std::string& name);
	FLAME_FOUNDATION_API void* create_native_event(bool signaled, bool manual = false);
	FLAME_FOUNDATION_API void set_native_event(void* ev);
	FLAME_FOUNDATION_API void reset_native_event(void* ev);
	FLAME_FOUNDATION_API bool wait_native_event(void* ev, int timeout);
	FLAME_FOUNDATION_API void destroy_native_event(void* ev);
	FLAME_FOUNDATION_API std::vector<std::filesystem::path> get_module_dependencies(const std::filesystem::path& path);
	FLAME_FOUNDATION_API std::wstring get_clipboard(bool peek = false); // if peek, return L"1" when has data
	FLAME_FOUNDATION_API void set_clipboard(std::wstring_view str);
	FLAME_FOUNDATION_API std::vector<std::filesystem::path> get_clipboard_files(bool peek = false); // if peek, return one L"1" when has data
	FLAME_FOUNDATION_API void set_clipboard_files(const std::vector<std::filesystem::path>& paths);
	FLAME_FOUNDATION_API std::pair<uvec2, std::unique_ptr<uchar>> get_thumbnail(uint width, const std::filesystem::path& path);
	// if out_id!=nullptr icon id will be assigned, and no icon data will be returned
	FLAME_FOUNDATION_API std::pair<uvec2, std::unique_ptr<uchar>> get_sys_icon(const std::filesystem::path& ext, int* out_id);
	FLAME_FOUNDATION_API void* get_console_window();
	FLAME_FOUNDATION_API void focus_window(void* hwnd);
	FLAME_FOUNDATION_API bool is_keyboard_pressing(KeyboardKey key);
	FLAME_FOUNDATION_API void send_keyboard_event(KeyboardKey key, bool down = true);
	FLAME_FOUNDATION_API void send_mouse_event(MouseButton key, bool down = true);
	FLAME_FOUNDATION_API void set_mouse_pos(const ivec2& pos);
	FLAME_FOUNDATION_API void shell_exec(const std::filesystem::path& filename, const std::wstring& parameters, bool wait, bool show = false);
	// if output==nullptr then the output will be redirect to std output
	FLAME_FOUNDATION_API void exec(const std::filesystem::path& filename, const std::wstring& parameters, std::string* output = nullptr);
	FLAME_FOUNDATION_API void debug_break();

	FLAME_FOUNDATION_API std::vector<void*> get_call_frames();
	FLAME_FOUNDATION_API std::vector<StackFrameInfo> get_call_frames_infos(std::span<void*> frames);

	FLAME_FOUNDATION_API void* add_global_key_listener(KeyboardKey key, const std::function<void(bool down)>& callback);
	FLAME_FOUNDATION_API void remove_global_key_listener(void* lis);

	// set_event to the returned ev to end the file watching
	// return native event
	FLAME_FOUNDATION_API void* add_file_watcher(const std::filesystem::path& path, const std::function<void(FileChangeFlags flags, const std::filesystem::path& path)>& callback, bool all_changes = true, bool sync = true);
}

#define FLAME_EXE_MAIN(entry) void* __crt_ev = nullptr; \
	extern "C" void mainCRTStartup(); \
	extern "C" __declspec(dllexport) void __stdcall __init_crt(void* ev) \
	{ \
		__crt_ev = ev; \
		mainCRTStartup(); \
	} \
	int main(int argc, char** args) \
	{ \
		if (__crt_ev) \
		{ \
			flame::set_native_event(__crt_ev); \
			while (true) \
				flame::sleep(60000); \
		} \
		return entry(argc, args); \
	}
