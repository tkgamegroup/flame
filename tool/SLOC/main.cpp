#include <vector>
#include <iterator>

#include <flame/foundation/foundation.h>

using namespace flame;

static std::vector<std::wstring> accept_exts;
static std::vector<std::wstring> general_ignores;
static std::vector<std::wstring> special_ignores;

void add_accept(const std::wstring &i)
{
	accept_exts.push_back(i);
}

void add_igonore(const std::wstring &i)
{
	std::fs::path p(i);
	auto i_fmt = p.wstring();

	if (i_fmt.size() > 2 && i_fmt[0] == L'*' &&  is_slash_chr(i_fmt[1]))
	{
		general_ignores.push_back(std::wstring(i_fmt.c_str() + 2));
		return;
	}
	special_ignores.push_back(i_fmt);
}

void add_default_accept()
{
	accept_exts.push_back(L".h");
	accept_exts.push_back(L".inl");
	accept_exts.push_back(L".hpp");
	accept_exts.push_back(L".c");
	accept_exts.push_back(L".cpp");
	accept_exts.push_back(L".cxx");
	accept_exts.push_back(L".js");
}

void add_default_ignore()
{
	add_igonore(L"*/.git");
}

long long total_lines = 0;
long long empty_lines = 0;

void iter(const std::wstring &p)
{
	for (std::fs::directory_iterator end, it(p); it != end; it++)
	{
		auto ffn = it->path().wstring();
		auto fn = it->path().filename().wstring();

		auto ignore = false;
		for (auto &i : general_ignores)
		{
			if (i == fn)
			{
				ignore = true;
				break;
			}
		}
		if (ignore)
			continue;
		for (auto &i : special_ignores)
		{
			if (i == ffn)
			{
				ignore = true;
				break;
			}
		}
		if (ignore)
			continue;

		if (std::fs::is_directory(it->status()))
		{
			auto s = it->path().stem().wstring();
			if (s.size() > 1 && s[0] != L'.')
				iter(it->path().wstring());
		}
		else
		{
			auto accept = false;
			auto ext = it->path().extension();
			for (auto &e : accept_exts)
			{
				if (e == ext)
				{
					accept = true;
					break;
				}
			}
			if (accept)
			{
				std::ifstream file(ffn);
				if (file.good())
				{
					while (!file.eof())
					{
						std::string line;
						std::getline(file, line);

						total_lines++;
						empty_lines++;
						for (auto chr : line)
						{
							if (!is_space_chr(chr))
							{
								empty_lines--;
								break;
							}
						}

					}
					file.close();
				}
			}
		}
	}
}

int main(int argc, char **args)
{
	std::ifstream policy_file("SLOC_Policy.txt");
	if (policy_file.good())
	{
		while (!policy_file.eof())
		{
			std::string line;
			std::getline(policy_file, line);

			auto sp = string_split(line);
			if (sp.size() > 0)
			{
				if (sp[0] == "accept:")
				{
					for (auto i = 1; i < sp.size(); i++)
					{
						if (sp[i] == "[default]")
							add_default_accept();
						else
							add_accept(s2w(sp[i]));
					}
				}
				else if (sp[0] == "ignore:")
				{
					for (auto i = 1; i < sp.size(); i++)
					{
						if (sp[i] == "[default]")
							add_default_ignore();
						else
							add_igonore(s2w(sp[i]));
					}
				}
			}
		}
		policy_file.close();
	}

	if (general_ignores.empty() && special_ignores.empty())
		add_default_ignore();
	if (accept_exts.empty())
		add_default_accept();

	auto curr_path = get_curr_path();

	for (auto& i : special_ignores)
	{
		if (i.size() > 0 && !is_slash_chr(i[0]))
			i = L"\\" + i;
		i = *curr_path.p + i;
	}

	iter(*curr_path.p);

	printf("total:%d\n", total_lines);
	printf("empty:%d\n", empty_lines);

	system("pause");

	return 0;
}
