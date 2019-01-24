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

using namespace flame;

FLAME_PACKAGE_BEGIN(FileWatcherC)
	FLAME_PACKAGE_ITEM(charptr, filename, p)
	FLAME_PACKAGE_ITEM(charptr, command, p)
FLAME_PACKAGE_END

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

	printf("run [ %s ] when [ %s ] changed\n", args[2], args[1]);

	add_file_watcher(std::filesystem::path(args[1]).parent_path().wstring().c_str(), Function<FileWatcherParm>([](FileWatcherParm &p) {
		auto c = p.get_capture<FileWatcherC>();

		auto now_time = get_now_ns();
		if (s2w(c.filename()) == p.filename() && now_time - last_change_time > 1'000'000'000)
		{
			printf("file changed: %s\n", c.filename());
			printf("run: %s\n", c.command());

			system(c.command());

			printf("run [ %s ] when [ %s ] changed\n", c.command(), c.filename());
		}
		last_change_time = now_time;

	}, { (voidptr)args[1], (voidptr)args[2] }), FileWatcherMonitorOnlyContentChanged | FileWatcherSynchronous);
}
