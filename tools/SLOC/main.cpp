#include <flame/serialize.h>
#include <flame/foundation/foundation.h>

#include <iterator>

using namespace flame;

static std::vector<std::wstring> extensions;
static std::vector<std::wstring> general_excludes;
static std::vector<std::wstring> special_excludes;

bool is_slash_chr(wchar_t ch)
{
	return ch == L'/' || ch == L'\\';
}

void add_extension(const std::wstring &i)
{
	extensions.push_back(i);
}

void add_exclude(const std::wstring &i)
{
	std::filesystem::path p(i);
	auto i_fmt = p.wstring();

	if (i_fmt.size() > 2 && i_fmt[0] == L'*' && is_slash_chr(i_fmt[1]))
		general_excludes.push_back(std::wstring(i_fmt.c_str() + 2));
	else
		special_excludes.push_back(i_fmt);
}

void add_default_extensions()
{
	extensions.push_back(L".h");
	extensions.push_back(L".inl");
	extensions.push_back(L".hpp");
	extensions.push_back(L".c");
	extensions.push_back(L".cpp");
	extensions.push_back(L".cxx");
	extensions.push_back(L".lua");
	extensions.push_back(L".js");
}

void add_default_excludes()
{
	add_exclude(L"*/.git");
}

long long total_lines = 0;

void iter(const std::wstring &p)
{
	for (std::filesystem::directory_iterator end, it(p); it != end; it++)
	{
		auto ffn = it->path().wstring();
		auto fn = it->path().filename().wstring();

		auto ignore = false;
		for (auto &i : general_excludes)
		{
			if (i == fn)
			{
				ignore = true;
				break;
			}
		}
		if (ignore)
			continue;
		for (auto &i : special_excludes)
		{
			if (i == ffn)
			{
				ignore = true;
				break;
			}
		}
		if (ignore)
			continue;

		if (std::filesystem::is_directory(it->status()))
		{
			auto s = it->path().stem().wstring();
			if (s.size() > 1 && s[0] != L'.')
				iter(it->path().wstring());
		}
		else
		{
			auto ext = it->path().extension();
			auto accept = false;
			for (auto &e : extensions)
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
				std::stringstream buffer;
				buffer << file.rdbuf();
				auto str = buffer.str();
				total_lines += std::count(str.begin(), str.end(), '\n');
				file.close();
			}
		}
	}
}

int main(int argc, char **args)
{
	auto ini = parse_ini_file("sloc_options.ini");
	for (auto& e : ini.get_section_entries("extensions"))
	{
		if (e.value == "%default%")
			add_default_extensions();
		else
			add_extension(s2w(e.value));
	}
	for (auto& e : ini.get_section_entries("excludes"))
	{
		if (e.value == "%default%")
			add_default_excludes();
		else
			add_exclude(s2w(e.value));
	}

	if (general_excludes.empty() && special_excludes.empty())
		add_default_extensions();
	if (extensions.empty())
		add_default_extensions();

	auto curr_path = std::filesystem::current_path();

	for (auto& i : special_excludes)
	{
		if (i.size() > 0 && !is_slash_chr(i[0]))
			i = L"\\" + i;
		i = curr_path.wstring() + i;
	}

	iter(curr_path.wstring());

	printf("lines:%d\n", total_lines);

	system("pause");

	return 0;
}
