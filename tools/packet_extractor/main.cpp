#include <flame/json.h>
#include <flame/foundation/foundation.h>

using namespace flame;

int main(int argc, char **args)
{
	int src_port = 0;
	int dst_port = 0;
	auto ap = parse_args(argc, args);
	auto input = ap.get_item("-input");
	if (auto str = ap.get_item("-src_port"); !str.empty())
		src_port = s2t<int>(str);
	if (auto str = ap.get_item("-dst_port"); !str.empty())
		dst_port = s2t<int>(str);
	auto printable_mode = ap.has("-printable");
	std::string res;
	if (!input.empty())
	{
		auto json = nlohmann::json::parse(get_file_content(input));
		auto n = json.size();
		for (auto i = 0; i < n; i++)
		{
			auto& p = json[i];
			auto& layers = p["_source"]["layers"];
			auto& tcp = layers["tcp"];
			if (src_port != 0)
			{
				auto port = tcp["tcp.srcport"].get<std::string>();
				if (src_port != s2t<int>(port))
					continue;
			}
			if (dst_port != 0)
			{
				auto port = tcp["tcp.dstport"].get<std::string>();
				if (dst_port != s2t<int>(port))
					continue;
			}
			auto& data = tcp["tcp.payload"];
			if (data.is_string())
			{
				auto str = data.get<std::string>();
				if (printable_mode)
				{
					auto sp = SUS::split(str, ':');
					str.clear();
					for (auto& t : sp)
					{
						auto ch = s2u_hex<uint>(t);
						if (isprint(ch))
							str += char(ch);
						else
							str += '.';
					}
				}
				res += str;
			}
		}
	}
	std::ofstream file(input + ".data");
	file << res;
	file.close();
	return 0;
}
