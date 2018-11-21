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

#include <flame/string.h>
#include <flame/file.h>

#include <vector>
#include <regex>

using namespace flame;

std::wstring root_dir;
std::vector<std::wstring> src_dirs;

std::vector<unsigned int> gids;

bool has_gid(unsigned int gid) 
{
	for (auto i : gids)
	{
		if (i == gid)
			return true;
	}
	return false;
}

unsigned int get_gid() 
{
	while (true)
	{
		unsigned int gid = rand();
		if (!has_gid(gid))
		{
			gids.push_back(gid);
			return gid;
		}
	}
	return 0;
}

int main(int argc, char **args)
{
	srand(time(0));

	root_dir = s2w(getenv("flame_path"));

	std::ifstream dir_file(root_dir + L"/dir.txt");
	if (dir_file.good())
	{
		while (!dir_file.eof())
		{
			std::string line;
			std::getline(dir_file, line);

			auto sp = string_split(line);
			if (sp.size() > 0)
			{
				if (sp[0] == "src:")
				{
					for (auto i = 1; i < sp.size(); i++)
						src_dirs.push_back(s2w(sp[i]));
				}
			}
		}
		dir_file.close();
	}

	for (auto &d : src_dirs)
	{
		for (std::filesystem::recursive_directory_iterator end, it(root_dir + L"/" + d); it != end; it++)
		{
			if (!std::filesystem::is_directory(it->status()) && it->path().extension() == L".cpp")
			{
				auto ffn = it->path().wstring();

				auto src = get_file_string(ffn);

				{
					auto pos = src.cbegin();
					std::smatch match;
					std::regex reg(R"(FLAME_GID\(([0-9]+)\))");
					while (std::regex_search(pos, src.cend(), match, reg))
					{
						gids.push_back(std::stoi(match[1]));
						pos += match.position() + match.length();
					}
				}

				{
					auto num_gid_provided = 0;

					std::string res;
					auto pos = src.cbegin();
					std::smatch match;
					std::regex reg("FLAME_""GID_UNDEFINED");
					while (std::regex_search(pos, src.cend(), match, reg))
					{
						res += match.prefix();
						res += "FLAME_GID(" + std::to_string(get_gid()) + ")";
						pos += match.position() + match.length();

						num_gid_provided++;
					}
					if (num_gid_provided > 0)
					{
						if (pos != src.cend())
							res += &(*pos);
						std::ofstream out(ffn, std::ios::binary);
						out.write(res.c_str(), res.size());
						out.close();

						printf("%d new GID(s) in: %s\n", num_gid_provided, w2s(ffn).c_str());
					}
				}
			}
		}
	}

	printf("total count of gid:%d\n", gids.size());

	return 0;
}
