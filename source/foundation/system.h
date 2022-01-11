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

	enum FileChangeType
	{
		FileAdded,
		FileRemoved,
		FileModified,
		FileRenamed
	};

	FLAME_FOUNDATION_EXPORTS void sleep(int time);
	FLAME_FOUNDATION_EXPORTS Guid generate_guid();
	FLAME_FOUNDATION_EXPORTS uint64 performance_counter();
	FLAME_FOUNDATION_EXPORTS uint64 performance_frequency();
	FLAME_FOUNDATION_EXPORTS std::filesystem::path get_app_path(bool has_name = false);
	FLAME_FOUNDATION_EXPORTS void* get_hinst();
	FLAME_FOUNDATION_EXPORTS uvec2 get_screen_size();
	FLAME_FOUNDATION_EXPORTS void* create_native_event(bool signaled, bool manual = false);
	FLAME_FOUNDATION_EXPORTS void set_native_event(void* ev);
	FLAME_FOUNDATION_EXPORTS void reset_native_event(void* ev);
	FLAME_FOUNDATION_EXPORTS bool wait_native_event(void* ev, int timeout);
	FLAME_FOUNDATION_EXPORTS void destroy_native_event(void* ev);
	FLAME_FOUNDATION_EXPORTS std::vector<std::filesystem::path> get_module_dependencies(const std::filesystem::path& path);
	FLAME_FOUNDATION_EXPORTS std::wstring get_clipboard();
	FLAME_FOUNDATION_EXPORTS void set_clipboard(std::wstring_view str);
	FLAME_FOUNDATION_EXPORTS std::pair<uvec2, std::unique_ptr<uchar>> get_thumbnail(uint width, const std::filesystem::path& path);
	// if out_id!=nullptr icon id will be assigned, and no icon data will be returned
	FLAME_FOUNDATION_EXPORTS std::pair<uvec2, std::unique_ptr<uchar>> get_icon(const std::filesystem::path& path, int* out_id);
	FLAME_FOUNDATION_EXPORTS bool is_keyboard_pressing(KeyboardKey key);
	FLAME_FOUNDATION_EXPORTS void send_keyboard_event(KeyboardKey key, bool down = true);
	FLAME_FOUNDATION_EXPORTS void send_mouse_event(MouseButton key, bool down = true);
	FLAME_FOUNDATION_EXPORTS void set_mouse_pos(const ivec2& pos);
	FLAME_FOUNDATION_EXPORTS void shell_exec(const std::filesystem::path& filename, const std::wstring& parameters, bool wait, bool show = false);
	// if output==nullptr then the output will be redirect to std output
	FLAME_FOUNDATION_EXPORTS void exec(const std::filesystem::path& filename, const std::wstring& parameters, std::string* output = nullptr);
	FLAME_FOUNDATION_EXPORTS void debug_break();

	FLAME_FOUNDATION_EXPORTS std::vector<void*> get_call_frames();
	FLAME_FOUNDATION_EXPORTS std::vector<StackFrameInfo> get_call_frames_infos(std::span<void*> frames);

	FLAME_FOUNDATION_EXPORTS void* add_global_key_listener(KeyboardKey key, const std::function<void(bool down)>& callback);
	FLAME_FOUNDATION_EXPORTS void remove_global_key_listener(void* lis);

	// set_event to the returned ev to end the file watching
	// return native event
	FLAME_FOUNDATION_EXPORTS void* add_file_watcher(const std::filesystem::path& path, const std::function<void(FileChangeType type, const std::filesystem::path& path)>& callback, bool all_changes = true, bool sync = true);
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
