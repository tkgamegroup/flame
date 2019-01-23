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

int main(int argc, char **args)
{
	if (argc < 3)
	{
		printf("usage: watch_file run_script_file(bat)\n");
		system("pause");
		return 0;
	}

	std::filesystem::path filename(args[1]);
	auto path = filename.parent_path();

	add_file_watcher(FileWatcherModeContent, path.wstring().c_str(), Function<>());
}

//add_file_watcher(FileWatcherModeContent, curr_path.data, [&](FileChangeType type, const char *filename) {
//	if (type == FileModified)
//	{
//		std::filesystem::path path(filename);
//		if (path.extension() == ".cpp")
//			compile(filename);
//	}
//});
