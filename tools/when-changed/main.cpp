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
	FLAME_PACKAGE_ITEM(wcharptr, filename, p)
	FLAME_PACKAGE_ITEM(wcharptr, scriptfilename, p)
	FLAME_PACKAGE_ITEM(charptr, params, p)
FLAME_PACKAGE_END

static ulonglong last_change_time = 0;

int main(int argc, char **args)
{
	if (argc < 3)
	{
		printf("usage: filename exe(or bat) [\"params\"]\nnote: must be wraped in \"\", %%s can be use in param, means the filename\n");
		system("pause");
		return 0;
	}

	auto filename = s2w(args[1]);
	auto scriptfilename = s2w(args[2]);
	auto path = std::filesystem::path(filename).parent_path().wstring();

	printf("watching file: %s\n", args[1]);

	add_file_watcher(path.c_str(), Function<FileWatcherParm>([](FileWatcherParm &p) {
		auto c = p.get_capture<FileWatcherC>();

		auto target_file = std::wstring(c.filename());
		auto now_time = get_now_ns();
		if (target_file == c.filename() && now_time - last_change_time > 1'000'000'000)
		{
			auto scriptfilename = std::wstring(c.scriptfilename());
			auto s_target_file = w2s(target_file);
			auto s_cmd = "\"" + w2s(scriptfilename) + "\"";
			if (c.params())
			{
				s_cmd += " ";
				s_cmd += c.params();
			}

			printf("file changed: %s\n", s_target_file.c_str());
			printf("run: %s\n", s_cmd.c_str());

			system(s_cmd.c_str());

			printf("watching file: %s\n", s_target_file.c_str());
		}
		last_change_time = now_time;

	}, { (voidptr)filename.c_str(), (voidptr)scriptfilename.c_str(), (voidptr)(argc == 4 ? args[3] : nullptr) }), FileWatcherMonitorOnlyContentChanged | FileWatcherSynchronous);
}
