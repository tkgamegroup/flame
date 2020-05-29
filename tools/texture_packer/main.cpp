#include <flame/serialize.h>
#include <flame/foundation/bitmap.h>

using namespace flame;

int main(int argc, char **args)
{
	std::vector<std::filesystem::path> inputs;
	std::filesystem::path output;
	auto border = false;
	for (auto i = 1; i < argc; i++)
	{
		auto arg = std::string(args[i]);
		if (arg[0] == '-' && arg.size() > 1)
		{
			if (arg[1] == 'b')
				border = true;
			else if (arg[1] == 'o' && arg.size() > 2)
				output = s2w(std::string(arg.begin() + 2, arg.end()));
		}
	}
	for (std::filesystem::directory_iterator end, it(get_curr_path().v); it != end; it++)
	{
		if (!std::filesystem::is_directory(it->status()) && is_image_file(it->path().extension()))
			inputs.push_back(it->path());
	}

	auto image_path = output;
	image_path.replace_extension(L".png");
	auto ext = output.extension();
	if (ext == L".atlas")
	{
		bin_pack(inputs, image_path, border, [&](const std::vector<BinPackTile>& tiles) {
			std::ofstream file(output);
			file << "image = \"" << output.filename().string() << "\"\n";
			file << "border = " << (border ? "1" : "0") << "\n";
			file << "\n[tiles]\n";
			for (auto& t : tiles)
				file << t.id + " " + to_string(Vec4u(Vec2u(t.pos) + (border ? 1U : 0U), t.b->size)) + "\n";
			file.close();
		});
	}
	else if (ext == L".json")
	{
		bin_pack(inputs, image_path, border, [&](const std::vector<BinPackTile>& tiles) {
			nlohmann::json json;
			{
				auto& mc = json["mc"];
				auto& clip1 = mc["clip1"];
				clip1["frameRate"] = 24;
				clip1["events"] = nlohmann::json::array();
				auto& frames = clip1["frames"];
				for (auto i = 0; i < tiles.size(); i++)
				{
					frames[i] = nlohmann::json{
						{"res", std::to_string(10000 + i + 1) + ".png"},
						{"x", 0},
						{"y", 0}
					};
				}
				auto& res = json["res"];
				for (auto& t : tiles)
				{
					res[t.id] = nlohmann::json{
						{"x", t.pos.x()},
						{"y", t.pos.y()},
						{"w", t.b->size.x()},
						{"h", t.b->size.y()}
					};
				}
			}
			std::ofstream file(output);
			file << json.dump();
			file.close();
		});
	}

	return 0;
}
