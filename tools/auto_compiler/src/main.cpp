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

using namespace flame;

std::string arch;
std::vector<std::string> include_dirs;
std::vector<std::string> link_libraries;
std::string output_dir;

bool recompile = false;

void compile(const std::string &cpp_filename)
{
	std::filesystem::path cpp_path(cpp_filename);
	auto dll_filename = output_dir + cpp_path.stem().string() + ".dll";

	if (!recompile)
	{
		if (std::filesystem::exists(dll_filename) && std::filesystem::last_write_time(cpp_path) <
			std::filesystem::last_write_time(dll_filename))
		{
			printf("%s (up to date)\n\n\n", cpp_filename.c_str());
			return;
		}
	}

	printf("compiling: %s\n", cpp_filename.c_str());

	std::string cl;
	if (arch == "x86")
		cl = "\"C:\\Program Files (x86)\\Microsoft Visual Studio\\2017\\Community\\VC\\Auxiliary\\Build\\vcvars32.bat\"";
	else if (arch == "x64")
		cl = "\"C:\\Program Files (x86)\\Microsoft Visual Studio\\2017\\Community\\VC\\Auxiliary\\Build\\vcvars64.bat\"";

	cl += " & cl ";
	cl += cpp_filename;
	cl += " -LD -MD -EHsc -Zi";
	for (auto &d : include_dirs)
		cl += " -I " + d;
	std::string libraries(" ");
	for (auto &l : link_libraries)
		libraries += l + " ";
	cl += " -link -DEBUG"
		+ libraries;

	std::string out_parent_path;
	if (!output_dir.empty())
	{
		out_parent_path = output_dir;
		if (!is_slash_chr(output_dir.back()))
			out_parent_path += '/';
	}

	cl += " -out:" + dll_filename;

	LongString output;
	exec("", cl.c_str(), &output);

	printf("%s\n\n\n", output.data);

	return;
}

int main(int argc, char **args)
{
	printf("config:");
	char config[100];
	scanf("%s", config);
	if (std::string(config) == "[recompile]")
		recompile = true;

	std::ifstream option_file("compile_option.txt");
	if (option_file.good())
	{
		enum State
		{
			StateNone,
			StateArch,
			StateInclude,
			StateLink,
			StateOutput
		}state = StateNone;

		while (!option_file.eof())
		{
			std::string line;
			std::getline(option_file, line);

			if (line == "arch:")
				state = StateArch;
			else if (line == "include:")
				state = StateInclude;
			else if (line == "link:")
				state = StateLink;
			else if (line == "output:")
				state = StateOutput;
			else
			{
				switch (state)
				{
				case StateArch:
					arch = line;
					break;
				case StateInclude:
					include_dirs.push_back(line);
					break;
				case StateLink:
					link_libraries.push_back(line);
					break;
				case StateOutput:
					output_dir = line;
					if (output_dir.size() > 0 && 
						!is_slash_chr(output_dir[output_dir.size() - 1]))
						output_dir += "/";
					break;
				}
			}
		}

		option_file.close();
	}

	if (arch == "")
		arch = "x64";

	MediumString curr_path;
	get_curr_path(&curr_path);

	for (std::filesystem::directory_iterator end, it(curr_path.data); it != end; it++)
	{
		if (!std::filesystem::is_directory(it->status()))
		{
			if (it->path().extension() == ".cpp")
				compile(it->path().string());
		}
	}

	add_file_watcher(FileWatcherModeContent, curr_path.data, [&](FileChangeType type, const char *filename) {
		if (type == FileModified)
		{
			std::filesystem::path path(filename);
			if (path.extension() == ".cpp")
				compile(filename);
		}
	});

	system("pause");

	return 0;
}
