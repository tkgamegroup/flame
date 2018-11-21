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

#include <flame/file.h>
#include <flame/system.h>
#include <flame/graphics/device.h>
#include <flame/UI/instance.h>
#include <flame/3d/scene.h>

#include <Windows.h>

#include <iostream>

using namespace flame;

std::wstring root_dir;
std::wstring bin_dir;

int main(int argc, char **args)
{
	root_dir = s2w(getenv("flame_path"));

	{
		std::ifstream file(root_dir + L"/dir.txt");
		if (file.good())
		{
			while (!file.eof())
			{
				std::string line;
				std::getline(file, line);

				auto sp = string_split(line);
				if (sp.size() > 0)
				{
					if (sp[0] == "bin:")
					{
						if (sp.size() == 2)
							bin_dir = s2w(sp[1]);
					}
				}
			}
		}
	}

	{
		auto d = graphics::Device::create(true);
		d->set_shared();
		ui::init(d, graphics::SampleCount_8);
		_3d::init(d);
	}

	while (true)
	{
		std::string cmd;
		std::cin >> cmd;

		if (cmd == "host")
		{
			std::string fn;
			std::cin >> fn;

			auto filename = root_dir + L"/" + bin_dir + L"/" + s2w(fn);
			auto lib = LoadLibraryW(filename.c_str());
			if (lib)
			{
				typedef int(*main_enterpoint)();
				auto main_ep = (main_enterpoint)GetProcAddress(lib, "main");
				if (main_ep)
					main_ep();
				FreeLibrary(lib);
			}
		}
	}

	return 0;
}
