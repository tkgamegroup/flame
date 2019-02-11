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

std::vector<std::string> sources;
std::vector<std::string> include_dirs;
std::vector<std::string> link_libraries;
std::string output;

bool force = false;

void compile()
{
	std::string cl("\"");
	cl += VS_LOCATION;
	cl += "/VC/Auxiliary/Build/vcvars64.bat\"";

	cl += " & cl ";
	for (auto& s : sources)
		cl += s + " ";
	cl += "-LD -MD -EHsc -Zi";
	for (auto &d : include_dirs)
		cl += " -I " + d;
	cl += " -link -DEBUG ";
	for (auto &l : link_libraries)
		cl += l + " ";

	cl += " -out:" + output;

	auto output = exec_and_get_output(L"", cl.c_str());

	printf("%s\n", output.v);
}

int main(int argc, char **args)
{
	if (argc <= 1)
		return 0;

	for (auto i = 1; i < argc; i++)
	{
		auto param = std::string(args[i]);
		if (param[0] == '-' && param.size() > 1)
		{
			auto what = param[1];
			switch (what)
			{
			case 'i':
				if (param.size() > 2)
					include_dirs.push_back(param.c_str() + 2);
				break;
			case 'l':
				if (param.size() > 2)
					link_libraries.push_back(param.c_str() + 2);
				break;
			case 'o':
				if (param.size() > 2)
					output = param.c_str() + 2;
				break;
			case 'f':
				force = true;
				break;
			}
		}
		else
			sources.push_back(param);
	}
	compile();

	system("pause");

	return 0;
}
