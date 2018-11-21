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

#pragma once

#ifdef FLAME_WINDOWS
#ifdef FLAME_SYSTEM_MODULE
#define FLAME_SYSTEM_EXPORTS __declspec(dllexport)
#else
#define FLAME_SYSTEM_EXPORTS __declspec(dllimport)
#endif
#else
#define FLAME_SYSTEM_EXPORTS
#endif

#include "math.h"
#include "string.h"
#include "function.h"

namespace flame
{
	struct Function;

	FLAME_SYSTEM_EXPORTS void *get_hinst();
	FLAME_SYSTEM_EXPORTS Ivec2 get_screen_size();
	FLAME_SYSTEM_EXPORTS const wchar_t *get_curr_path();
	FLAME_SYSTEM_EXPORTS const wchar_t *get_app_path();
	FLAME_SYSTEM_EXPORTS void exec(const wchar_t *filename, const wchar_t *parameters, bool wait);
	FLAME_SYSTEM_EXPORTS String exec_and_get_output(const wchar_t *filename, const wchar_t *parameters);

	FLAME_SYSTEM_EXPORTS StringW get_clipboard();
	FLAME_SYSTEM_EXPORTS void set_clipboard(const StringW &s);

	FLAME_SYSTEM_EXPORTS void open_explorer_and_select(const wchar_t *filename);
	FLAME_SYSTEM_EXPORTS void move_to_trashbin(const wchar_t *filename);

	struct FileWatcher;

	enum FileWatcherMode
	{
		FileWatcherModeAll,
		FileWatcherModeContent
	};

	enum FileChangeType
	{
		FileAdded,
		FileRemoved,
		FileModified,
		FileRenamed
	};

	FLAME_SYSTEM_EXPORTS FileWatcher *add_file_watcher(FileWatcherMode mode, const wchar_t *filepath, PF pf, char *capture_fmt, ...); // (FileChangeType type, const wchar_t *filename)
	FLAME_SYSTEM_EXPORTS void remove_file_watcher(FileWatcher *w);

	FLAME_SYSTEM_EXPORTS void read_process_memory(void *process, void *address, int size, void *dst);

	FLAME_SYSTEM_EXPORTS Function *add_global_key_listener(int key, PF pf, char *capture_fmt, ...); // 0 parms
	FLAME_SYSTEM_EXPORTS void remove_global_key_listener(int key, Function *f);

	FLAME_SYSTEM_EXPORTS void get_thumbnai(int width, const wchar_t *filename, int *out_width, int *out_height, char **out_data);
}
