#include <flame/universe/app.h>
#include <flame/foundation/bitmap.h>

#include <random>
#include <algorithm>
#include <functional>

using namespace flame;
using namespace graphics;

App app;

int main(int argc, char** args)
{
	{
		auto ext_xz = 400.f;
		auto ext_y = 200.f;
		auto n_blocks = 16;
		auto n_regions = n_blocks / 2;
		auto base_height = 30.f;
		auto block_height = 4.f;

		std::vector<std::vector<int>> blocks;
		blocks.resize(n_blocks);
		for (auto& col : blocks)
			col.resize(n_blocks);

		auto number = n_blocks * n_blocks / 2;
		for (auto i = 0; i < number; i++)
		{
			auto& b = blocks[linearRand(0, n_blocks - 1)][linearRand(0, n_blocks - 1)];
			if (b == 0)
				b = 1;
			else
				i--;
		}

		//struct Region
		//{
		//	int visited = 0;
		//	int edges[4] = { 0, 0, 0, 0 }; // L T R B
		//};

		//std::vector<std::vector<Region>> regions;
		//regions.resize(n_regions);
		//for (auto& col : regions)
		//	col.resize(n_regions);

		//std::random_device rd;
		//std::mt19937 g(rd());

		//std::function<bool(int x, int y)> visit;
		//visit = [&](int x, int y) {
		//	auto& r = regions[x][y];
		//	if (r.visited)
		//		return false;
		//	r.visited = 1;

		//	std::vector<int> order = { 0, 1, 2, 3 };
		//	std::shuffle(order.begin(), order.end(), g);
		//	for (auto& i : order)
		//	{
		//		switch (i)
		//		{
		//		case 0:
		//			if (x > 0 && visit(x - 1, y))
		//				r.edges[0] = 1;
		//			break;
		//		case 1:
		//			if (y > 0 && visit(x, y - 1))
		//				r.edges[1] = 1;
		//			break;
		//		case 2:
		//			if (x < n_regions - 1 && visit(x + 1, y))
		//				r.edges[2] = 1;
		//			break;
		//		case 3:
		//			if (y < n_regions - 1 && visit(x, y + 1))
		//				r.edges[3] = 1;
		//			break;
		//		}
		//	}
		//	return true;
		//};
		//visit(0, 0);

		//for (auto& col : blocks)
		//{
		//	for (auto& b : col)
		//		b = 1;
		//}
		//for (auto y = 0; y < n_regions; y++)
		//{
		//	for (auto x = 0; x < n_regions; x++)
		//	{
		//		auto& r = regions[x][y];
		//		blocks[x * 2][y * 2] = 0;
		//		for (auto i = 0; i < 4; i++)
		//		{
		//			if (r.edges[i])
		//			{
		//				switch (i)
		//				{
		//				case 0:
		//					blocks[x * 2 - 1][y * 2] = 0;
		//					break;
		//				case 1:
		//					blocks[x * 2][y * 2 - 1] = 0;
		//					break;
		//				case 2:
		//					blocks[x * 2 + 1][y * 2] = 0;
		//					break;
		//				case 3:
		//					blocks[x * 2][y * 2 + 1] = 0;
		//					break;
		//				}
		//			}
		//		}
		//	}
		//}

		std::ofstream file_blocks(L"blocks.txt");
		file_blocks << n_blocks;
		file_blocks << "\n";
		for (auto x = 0; x < n_blocks; x++)
		{
			for (auto y = 0; y < n_blocks; y++)
			{
				file_blocks << blocks[x][y];
				file_blocks << " ";
			}
			file_blocks << "\n";
		}
		file_blocks.close();

		auto bmp_fbm = Bitmap::create(L"D:\\assets\\terrain\\fbm.png");
		auto fbm_pixels = bmp_fbm->get_data();
		auto fbm_scale = 0.2f;

		auto img_ext = ivec2(bmp_fbm->get_width(), bmp_fbm->get_height());
		auto block_pixels = 256 / n_blocks;
		auto bmp = Bitmap::create(img_ext.x, img_ext.y, 1);
		auto pixels = bmp->get_data();

		auto each_pixel = [&](int xa, int xb, int ya, int yb, const std::function<void(int, int)>& f) {
			for (auto y = ya; y < yb; y++)
			{
				for (auto x = xa; x < xb; x++)
					f(x, y);
			}
		};

		auto h1 = int(base_height / ext_y * 255.f);
		auto h2 = int((block_height + base_height) / ext_y * 255.f);

		each_pixel(0, img_ext.x, 0, img_ext.y, [&](int x, int y) {
			auto ibx = x / block_pixels;
			auto iby = y / block_pixels;
			auto fbx = fract((float)x / block_pixels);
			auto fby = fract((float)y / block_pixels);
			auto pid = img_ext.x * y + x;
			float h;
			if (blocks[ibx][iby])
			{
				h = h2;
				if (fbx < 0.25f && ibx > 0 && blocks[ibx - 1][iby] == 0)
				{
					if (perlin(vec2(ibx, (float)y / img_ext.y * 47.149f)) * 0.25f > fbx)
						h = h1;
				}
				if (fbx > 0.75f && ibx < n_blocks - 1 && blocks[ibx + 1][iby] == 0)
				{
					if (fbx > 1.f + perlin(vec2(ibx + 1, (float)y / img_ext.y * 47.149f)) * 0.25f)
						h = h1;
				}
				if (fby < 0.25f && iby > 0 && blocks[ibx][iby - 1] == 0)
				{
					if (perlin(vec2((float)x / img_ext.x * 47.149f, iby)) * 0.25f > fby)
						h = h1;
				}
				if (fby > 0.75f && iby < n_blocks - 1 && blocks[ibx][iby + 1] == 0)
				{
					if (fbx > 1.f + perlin(vec2((float)x / img_ext.x * 47.149f, iby + 1)) * 0.25f)
						h = h1;
				}
			}
			else
			{
				h = h1;
				if (fbx < 0.25f && ibx > 0 && blocks[ibx - 1][iby] == 1)
				{
					if (perlin(vec2(ibx, (float)y / img_ext.y * 47.149f)) * 0.25f - 0.25f > fbx)
						h = h2;
				}
				if (fbx > 0.75f && ibx < n_blocks - 1 && blocks[ibx + 1][iby] == 1)
				{
					if (fbx > 1.f + perlin(vec2(ibx + 1, (float)y / img_ext.y * 47.149f)) * 0.25f)
						h = h2;
				}
				if (fby < 0.25f && iby > 0 && blocks[ibx][iby - 1] == 1)
				{
					if (perlin(vec2((float)x / img_ext.x * 47.149f, iby)) * 0.25f > fby)
						h = h2;
				}
				if (fby > 0.75f && iby < n_blocks - 1 && blocks[ibx][iby + 1] == 1)
				{
					if (fbx > 1.f + perlin(vec2((float)x / img_ext.x * 47.149f, iby + 1)) * 0.25f)
						h = h2;
				}
			}
			pixels[pid] = h + fbm_pixels[pid] * fbm_scale;
		});

		// slopes
		for (auto y = 0; y < n_blocks; y++)
		{
			for (auto x = 1; x < n_blocks; x++)
			{
				if (linearRand(0.f, 1.f) > 0.8f)
				{
					if (blocks[x - 1][y] == 0 && blocks[x][y] == 1)
					{
						auto xa = int((x - 0.3f) * block_pixels);
						auto xb = int((x + 0.3f) * block_pixels);
						each_pixel(xa, xb, (y + 0.25) * block_pixels, (y + 0.75f) * block_pixels, [&](int x, int y) {
							auto idx = img_ext.x * y + x;
							pixels[idx] = int(mix((float)h1, (float)h2, float(x - xa) / float(xb - xa)) + fbm_pixels[idx] * fbm_scale);
						});
					}
					else if (blocks[x - 1][y] == 1 && blocks[x][y] == 0)
					{
						auto xa = int((x - 0.3f) * block_pixels);
						auto xb = int((x + 0.3f) * block_pixels);
						each_pixel(xa, xb, (y + 0.25) * block_pixels, (y + 0.75f) * block_pixels, [&](int x, int y) {
							auto idx = img_ext.x * y + x;
							pixels[idx] = int(mix((float)h2, (float)h1, float(x - xa) / float(xb - xa)) + fbm_pixels[idx] * 0.2f);
						});
					}
					else
						continue;
				}
			}
		}
		for (auto x = 0; x < n_blocks; x++)
		{
			for (auto y = 1; y < n_blocks; y++)
			{
				if (linearRand(0.f, 1.f) > 0.8f)
				{
					if (blocks[x][y - 1] == 0 && blocks[x][y] == 1)
					{
						auto ya = int((y - 0.3f) * block_pixels);
						auto yb = int((y + 0.3f) * block_pixels);
						each_pixel((x + 0.25) * block_pixels, (x + 0.75f) * block_pixels, ya, yb, [&](int x, int y) {
							auto idx = img_ext.x * y + x;
							pixels[idx] = int(mix((float)h1, (float)h2, float(y - ya) / float(yb - ya)) + fbm_pixels[idx] * fbm_scale);
						});
					}
					else if (blocks[x][y - 1] == 1 && blocks[x][y] == 0)
					{
						auto ya = int((y - 0.3f) * block_pixels);
						auto yb = int((y + 0.3f) * block_pixels);
						each_pixel((x + 0.25)* block_pixels, (x + 0.75f)* block_pixels, ya, yb, [&](int x, int y) {
							auto idx = img_ext.x * y + x;
							pixels[idx] = int(mix((float)h2, (float)h1, float(y - ya) / float(yb - ya)) + fbm_pixels[idx] * fbm_scale);
						});
					}
					else
						continue;
				}
			}
		}

		bmp->save(L"D:\\assets\\terrain\\height.png");
		//return 0;
	}

#if _DEBUG
	app.create(true);
#else
	app.create(false);
#endif

	auto w = Window::create(nullptr, NativeWindow::create(L"Dota Pubg", uvec2(800, 600), WindowFrame | WindowResizable));
	w->get_native()->set_cursor(CursorNone);
	app.set_main_window(w);

	auto renderer = app.s_renderer;
	renderer->set_shadow_props(3, 50.f, 20.f);
	renderer->set_ssao_props(1.64f, 0.04f);

	auto script_ins = script::Instance::get_default();
	script_ins->excute_file(L"terrain_scatter.lua");
	script_ins->excute_file(L"scripts/head.lua");
	script_ins->excute_file(L"scripts/attribute.lua");
	script_ins->excute_file(L"scripts/character.lua");
	script_ins->excute_file(L"scripts/player.lua");
	script_ins->excute_file(L"scripts/npc.lua");
	script_ins->excute_file(L"scripts/skill.lua");
	script_ins->excute_file(L"scripts/item.lua");
	script_ins->excute_file(L"scripts/buff.lua");
	script_ins->excute_file(L"scripts/particle.lua");
	script_ins->excute_file(L"scripts/projectile.lua");
	{
		auto e = Entity::create();
		e->load(L"prefabs/main");
		app.root->add_child(e);
	}
	script_ins->excute_file(L"scripts/main.lua");

	//w->s_physics->set_visualization(true);

	run([](Capture& c, float) {
		app.update();
		printf("fps: %d\n", get_fps());
	}, Capture());

	return 0;
}
