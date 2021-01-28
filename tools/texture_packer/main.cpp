#include <flame/serialize.h>
#include <flame/foundation/bitmap.h>

using namespace flame;

int main(int argc, char **args)
{
	std::string output;
	auto ap = pack_args(argc, args);
	auto border = false;
	if (!ap.get_item("-o", output))
		return 0;
	if (ap.has("-b"))
		border = true;
	auto output_path = std::filesystem::path(output);
	std::vector<std::filesystem::path> inputs;
	for (std::filesystem::directory_iterator end, it(std::filesystem::current_path()); it != end; it++)
	{
		if (!std::filesystem::is_directory(it->status()) && is_image_file(it->path().extension()))
			inputs.push_back(it->path());
	}

	auto image_path = output_path;
	image_path.replace_extension(L".png");
	auto ext = output_path.extension();
	if (auto p = output_path.parent_path(); !std::filesystem::exists(p))
		std::filesystem::create_directories(p);
	if (ext == L".atlas")
	{
		bin_pack(uvec2(1024, 4096), inputs, image_path, border, [&](const std::vector<BinPackTile>& tiles) {
			std::ofstream file(output_path);
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
