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
	for (std::filesystem::directory_iterator end, it(std::filesystem::current_path()); it != end; it++)
	{
		if (!std::filesystem::is_directory(it->status()) && is_image_file(it->path().extension()))
			inputs.push_back(it->path());
	}

	auto image_path = output;
	image_path.replace_extension(L".png");
	auto ext = output.extension();
	if (auto p = output.parent_path(); !std::filesystem::exists(p))
		std::filesystem::create_directories(p);
	if (ext == L".atlas")
	{
		bin_pack(uvec2(1024, 4096), inputs, image_path, border, [&](const std::vector<BinPackTile>& tiles) {
			std::ofstream file(output);
			file << "image = \"" << image_path.filename().string() << "\"\n";
			file << "border = " << (border ? "1" : "0") << "\n";
			file << "\n[tiles]\n";
			for (auto& t : tiles)
				file << t.id + " " + to_string(uvec4(uvec2(t.pos) + (border ? 1U : 0U), uvec2(t.b->get_width(), t.b->get_height()))) + "\n";
			file.close();
		});
	}

	return 0;
}
