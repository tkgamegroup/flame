// MIT License
// 
// Copyright (c) 2019 wjs
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

using namespace flame;

static ulonglong last_change_time = 0;

int main(int argc, char **args)
{
	if (argc != 3)
	{
		printf(
			   "usage:\n"
			   "  filename \"command\"\n"
			   "note:\n"
			   "  command must be wraped in \"\"\n"
		);
		system("pause");
		return 0;
	}

	printf("watch=\"%s\" cmd=\"%s\"\n", args[1], args[2]);

	struct Capture
	{
		const char* filename;
		const char* command;
	};

	Capture capture;
	capture.filename = args[1];
	capture.command = args[2];

	add_file_watcher(std::fs::path(args[1]).parent_path().wstring(), Function<void(void* c, FileChangeType type, const std::wstring& filename)>(
	[](void* _c, FileChangeType type, const std::wstring& filename) {
		auto c = (Capture*)_c;

		auto now_time = get_now_ns();
		if (s2w(c->filename) == filename && now_time - last_change_time > 1'000'000'000)
		{
			printf("file changed: %s\n", c->filename);
			printf("run: %s\n", c->command);

			system(c->command);

			printf("watch=[ %s ] cmd=[ %s ]\n", c->filename, c->command);
		}
		last_change_time = now_time;

	}, sizeof(Capture), &capture), FileWatcherMonitorOnlyContentChanged | FileWatcherSynchronous);
}
