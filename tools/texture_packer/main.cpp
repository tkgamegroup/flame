#include <flame/serialize.h>
#include <flame/foundation/bitmap.h>

using namespace flame;

struct BinPackTile
{
	std::string id;
	UniPtr<Bitmap> bmp;
	ivec2 pos;
	uvec2 size;
};

int main(int argc, char **args)
{
	auto current_path = std::filesystem::current_path();
	auto output_path = current_path.filename();

	std::vector<std::filesystem::path> inputs;
	for (auto& it : std::filesystem::directory_iterator(current_path))
	{
		if (!std::filesystem::is_directory(it.status()))
		{
			auto& path = it.path();
			if (is_image_file(path.extension()) && path.filename().stem() != output_path)
				inputs.push_back(it.path());
		}
	}

	std::vector<BinPackTile> tiles;
	for (auto& i : inputs)
	{
		BinPackTile t;
		t.id = i.filename().stem().string();
		t.bmp.reset(Bitmap::create(i.c_str()));
		t.pos = ivec2(-1);
		t.size = uvec2(t.bmp->get_width(), t.bmp->get_height());
		tiles.push_back(std::move(t));
	}
	std::sort(tiles.begin(), tiles.end(), [](const auto& a, const auto& b) {
		return max(a.size.x, a.size.y) > max(b.size.x, b.size.y);
	});

	auto size = uvec2(1024, 4096);

	auto tree = std::make_unique<BinPackNode>(size);

	for (auto& t : tiles)
	{
		auto n = tree->find(t.size + 2U);
		if (n)
			t.pos = n->pos;
	}

	auto shrink_size = uvec2(0);
	for (auto& t : tiles)
		shrink_size = max(uvec2(t.pos) + t.size + 2U, shrink_size);

	auto b = Bitmap::create(shrink_size.x, shrink_size.y, 4);
	for (auto& t : tiles)
	{
		if (t.pos.x >= 0 && t.pos.y >= 0)
			t.bmp->copy_to((BitmapPtr)b, t.size.x, t.size.y, 0, 0, t.pos.x, t.pos.y, true);
	}

	output_path.replace_extension(L".png");
	b->save(output_path.c_str());

	output_path.replace_extension(L".atlas");
	std::ofstream file(output_path);
	for (auto& t : tiles)
		file << t.id + " " + to_string(uvec4(uvec2(t.pos) + 1U, t.size)) + "\n";
	file.close();

	return 0;
}
