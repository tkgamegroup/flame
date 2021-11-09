#pragma once

#include "foundation.h"

namespace flame
{
	struct StackFrameInfo
	{
		char file[260];
		uint line;
		char function[260];
	};

	enum FileChangeType
	{
		FileAdded,
		FileRemoved,
		FileModified,
		FileRenamed
	};

	FLAME_FOUNDATION_EXPORTS Guid generate_guid();
	FLAME_FOUNDATION_EXPORTS void get_app_path(wchar_t* dst, bool has_name = false);
	FLAME_FOUNDATION_EXPORTS void get_logical_drives(uint* count, wchar_t** names);
	FLAME_FOUNDATION_EXPORTS void* get_hinst();
	FLAME_FOUNDATION_EXPORTS uvec2 get_screen_size();
	FLAME_FOUNDATION_EXPORTS void* create_native_event(bool signaled, bool manual = false);
	FLAME_FOUNDATION_EXPORTS void set_native_event(void* ev);
	FLAME_FOUNDATION_EXPORTS void reset_native_event(void* ev);
	FLAME_FOUNDATION_EXPORTS bool wait_native_event(void* ev, int timeout);
	FLAME_FOUNDATION_EXPORTS void destroy_native_event(void* ev);
	FLAME_FOUNDATION_EXPORTS void get_module_dependencies(const wchar_t* filename, void (*callback)(Capture& c, const wchar_t* filename), const Capture& capture);
	FLAME_FOUNDATION_EXPORTS void get_clipboard(void* str, wchar_t* (*str_allocator)(void* str, uint size));
	FLAME_FOUNDATION_EXPORTS void set_clipboard(const wchar_t* s);
	// you are responsible for freeing *out_data using f_free
	FLAME_FOUNDATION_EXPORTS void get_thumbnail(uint width, const wchar_t* filename, uint* out_width, uint* out_height, uchar** out_data);
	// you are responsible for freeing *out_data using f_free
	FLAME_FOUNDATION_EXPORTS void get_icon(const wchar_t* filename, int* out_id, uint* out_width, uint* out_height, uchar** out_data);
	FLAME_FOUNDATION_EXPORTS bool is_keyboard_key_pressing(KeyboardKey key);
	FLAME_FOUNDATION_EXPORTS void send_global_keyboard_event(KeyboardKey key, bool down = true);
	FLAME_FOUNDATION_EXPORTS void send_global_mouse_event(MouseKey key, bool down = true);
	FLAME_FOUNDATION_EXPORTS void set_mouse_pos(const ivec2& pos);
	FLAME_FOUNDATION_EXPORTS void shell_exec(const wchar_t* filename, wchar_t* parameters, bool wait, bool show = false);
	// if str is null then the output will be redirect to std output
	FLAME_FOUNDATION_EXPORTS void exec(const wchar_t* filename, wchar_t* parameters, char* output = nullptr);
	FLAME_FOUNDATION_EXPORTS void debug_break();

	// max depth: 64
	FLAME_FOUNDATION_EXPORTS void get_call_frames(void** (*array_allocator)(Capture& c, uint size), const Capture& capture);
	FLAME_FOUNDATION_EXPORTS void get_call_frames_infos(uint frames_count, void** frames, StackFrameInfo* dst);

	FLAME_FOUNDATION_EXPORTS void* add_global_key_listener(KeyboardKey key, void (*callback)(Capture& c), const Capture& capture, bool down = true, bool ctrl = false, bool shift = false, bool alt = false);
	FLAME_FOUNDATION_EXPORTS void remove_global_key_listener(void* ret);

	// set_event to the returned ev to end the file watching
	FLAME_FOUNDATION_EXPORTS void* /* event */ add_file_watcher(const wchar_t* path, void (*callback)(Capture& c, FileChangeType type, const wchar_t* filename), const Capture& capture, bool all_changes = true, bool sync = true);
}
