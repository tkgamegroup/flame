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
#include <flame/foundation/serialize.h>

using namespace flame;

int main(int argc, char **args)
{
	if (argc == 1)
		return 0;

	// typeinfo collect must do by order, because it only record the first entry
	std::vector<std::wstring> pdbs = {
		L"flame_foundation.dll",
		L"flame_graphics.dll",
		L"flame_network.dll",
		L"flame_sound.dll",
		L"flame_universe.dll",
	};

	auto id = 0;
	for (auto& fn : pdbs)
	{
		auto w_dst = ext_replace(fn, L".typeinfo");
		auto dst = w2s(w_dst);
		if (!std::filesystem::exists(w_dst) || std::filesystem::last_write_time(w_dst) < std::filesystem::last_write_time(fn))
		{
			printf("generating: %s\n", dst.c_str());

			typeinfo_collect(fn, id);
			typeinfo_save(w_dst, id);

			printf("ok\n");
		}
		else
			printf("up-to-data: %s\n", dst.c_str());
		id++;
	}

	return 0;
}
