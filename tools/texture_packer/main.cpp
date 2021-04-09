#include <flame/serialize.h>
#include <flame/foundation/bitmap.h>

using namespace flame;

struct BinPackTile
{
	std::string id;
	UniPtr<Bitmap> bmp;
	ivec2 pos;
};

inline void bin_pack(const uvec2& size, const std::vector<std::filesystem::path>& inputs, const std::filesystem::path& output, bool border, const std::function<void(const std::vector<BinPackTile>& tiles)>& callback)
{
	auto b1 = border ? 1U : 0U;
	auto b2 = b1 << 1;

	std::vector<BinPackTile> tiles;
	for (auto& i : inputs)
	{
		BinPackTile t;
		t.id = i.filename().stem().string();
		t.bmp.reset(Bitmap::create(i.c_str()));
		t.pos = ivec2(-1);
		tiles.push_back(std::move(t));
	}
	std::sort(tiles.begin(), tiles.end(), [](const auto& a, const auto& b) {
		return max(a.bmp->get_width(), a.bmp->get_height()) > max(b.bmp->get_width(), b.bmp->get_height());
	});

	auto tree = std::make_unique<BinPackNode>(size);

	for (auto& t : tiles)
	{
		auto n = tree->find(uvec2(t.bmp->get_width(), t.bmp->get_height()) + b2);
		if (n)
			t.pos = n->pos;
	}

	auto _size = uvec2(0);
	for (auto& t : tiles)
	{
		_size.x = max(t.pos.x + t.bmp->get_width() + b1, _size.x);
		_size.y = max(t.pos.y + t.bmp->get_height() + b1, _size.y);
	}

	auto b = Bitmap::create(_size.x, _size.y, 4);
	for (auto& t : tiles)
	{
		if (t.pos.x >= 0 && t.pos.y >= 0)
			t.bmp->copy_to((BitmapPtr)b, t.bmp->get_width(), t.bmp->get_height(), 0, 0, t.pos.x, t.pos.y, border);
	}
	b->save(output.c_str());

	callback(tiles);
}

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
