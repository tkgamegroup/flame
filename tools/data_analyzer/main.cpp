#include <flame/foundation/foundation.h>

using namespace flame;

int main(int argc, char **args)
{
	auto ap = parse_args(argc, args);
	auto input = ap.get_item("-input");
	auto output = ap.get_item("-output");
	auto action = ap.get_item("-action");
	if (output.empty())
		output = input + "." + action + ".res";
	if (action == "find")
	{
		auto items = ap.get_items("-items");
		auto content = get_file_content(input);
		if (!content.empty())
		{
			std::ofstream file(output);
			for (auto& n : items)
			{
				file << n << std::endl;
				size_t pos = 0;
				while (true)
				{
					pos = content.find(n, pos);
					if (pos == std::string::npos)
						break;
					pos += n.size();
					file << "  " << (int)pos << std::endl;
				}
			}
			file.close();
		}
	}
	else if (action == "common_prefix")
	{
		auto offset_items = ap.get_items("-offsets");
		auto length = s2t<int>(ap.get_item("-length"));
		auto step = s2t<int>(ap.get_item("-step"));
		if (offset_items.size() >= 2 && length >= 1 && step >= 1)
		{
			auto content = get_file_content(input);
			if (!content.empty())
			{
				std::vector<int> offsets;
				offsets.resize(offset_items.size());
				for (auto i = 0; i < offsets.size(); i++)
					offsets[i] = s2t<int>(offset_items[i]);
				std::sort(offsets.begin(), offsets.end());
				auto min_dist = 0xffffffff;
				for (auto i = (int)offsets.size() - 1; i > 0; i--)
				{
					auto dist = offsets[i] - offsets[i - 1];
					if (dist < min_dist)
						min_dist = dist;
				}
				std::string str;
				for (auto i = 0; i < min_dist; i += step)
				{
					str.clear();
					auto ok = true;
					for (auto offset : offsets)
					{
						auto s = content.substr(offset - i - length, length);
						if (str.empty())
							str = s;
						else
						{
							if (str != s)
							{
								ok = false;
								break;
							}
						}
					}
					if (ok)
						int cut = 1;
				}
			}
		}
	}
	return 0;
}
