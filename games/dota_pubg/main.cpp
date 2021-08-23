#include <flame/universe/app.h>
#include <flame/foundation/bitmap.h>

#include <random>
#include <algorithm>
#include <functional>

using namespace flame;
using namespace graphics;

App g_app;

int main(int argc, char** args)
{
	{
		auto ext_xz = 400.f;
		auto ext_y = 200.f;
		auto n_blocks = 16;
		auto n_regions = n_blocks / 2;
		auto base_height = 30.f;
		auto block_height = 4.f;

		std::random_device rd;
		std::mt19937 g(rd());

		std::vector<std::vector<int>> blocks;
		blocks.resize(n_blocks);
		for (auto& col : blocks)
			col.resize(n_blocks);

		auto number = n_blocks * n_blocks / 2;
		for (auto i = 0; i < number; i++)
		{
			auto x = g() % n_blocks;
			auto y = g() % n_blocks;
			if (blocks[x][y] == 0)
				blocks[x][y] = 1;
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
		for (auto x = 0; x < n_blocks; x++)
		{
			for (auto y = 0; y < n_blocks; y++)
			{
				auto h = blocks[x][y] ? h2 : h1;
				each_pixel(x* block_pixels, (x + 1) * block_pixels, 
				y * block_pixels, (y + 1) * block_pixels, [&](int x, int y) {
					auto idx = img_ext.x * y + x;
					pixels[idx] = h + fbm_pixels[idx] * fbm_scale;
				});
			}
		}

		for (auto y = 0; y < n_blocks; y++)
		{
			for (auto x = 1; x < n_blocks; x++)
			{
				if ((float)g() / (float)g.max() > 0.8f)
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
				if ((float)g() / (float)g.max() > 0.8f)
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
		auto coner = int(0.5f * block_pixels);
		for (auto x = 1; x < n_blocks; x++)
		{
			for (auto y = 1; y < n_blocks; y++)
			{
				if (blocks[x - 1][y - 1] == 1 && blocks[x][y - 1] == 0 && blocks[x - 1][y] == 0)
				{
					auto xb = x * block_pixels;
					auto yb = y * block_pixels;
					auto xa = xb - coner;
					auto ya = yb - coner;
					each_pixel(xa, xb, ya, yb, [&](int x, int y) {
						if (distance(vec2(x, y), vec2(xa, ya)) > coner)
						{
							auto idx = img_ext.x * y + x;
							pixels[idx] = h1 + fbm_pixels[idx] * fbm_scale;
						}
					});
				}
				if (blocks[x][y - 1] == 1 && blocks[x - 1][y - 1] == 0 && blocks[x][y] == 0)
				{
					auto xa = x * block_pixels;
					auto yb = y * block_pixels;
					auto xb = xa + coner;
					auto ya = yb - coner;
					each_pixel(xa, xb, ya, yb, [&](int x, int y) {
						if (distance(vec2(x, y), vec2(xb, ya)) > coner)
						{
							auto idx = img_ext.x * y + x;
							pixels[idx] = h1 + fbm_pixels[idx] * fbm_scale;
						}
					});
				}
				if (blocks[x][y] == 1 && blocks[x][y - 1] == 0 && blocks[x - 1][y] == 0)
				{
					auto xa = x * block_pixels;
					auto ya = y * block_pixels;
					auto xb = xa + coner;
					auto yb = ya + coner;
					each_pixel(xa, xb, ya, yb, [&](int x, int y) {
						if (distance(vec2(x, y), vec2(xb, yb)) > coner)
						{
							auto idx = img_ext.x * y + x;
							pixels[idx] = h1 + fbm_pixels[idx] * fbm_scale;
						}
					});
				}
				if (blocks[x - 1][y] == 1 && blocks[x - 1][y - 1] == 0 && blocks[x][y] == 0)
				{
					auto xb = x * block_pixels;
					auto ya = y * block_pixels;
					auto xa = xb - coner;
					auto yb = ya + coner;
					each_pixel(xa, xb, ya, yb, [&](int x, int y) {
						if (distance(vec2(x, y), vec2(xa, yb)) > coner)
						{
							auto idx = img_ext.x * y + x;
							pixels[idx] = h1 + fbm_pixels[idx] * fbm_scale;
						}
					});
				}
			}
		}

		bmp->save(L"D:\\assets\\terrain\\height.png");
		//return 0;
	}

#if _DEBUG
	g_app.create(true);
#else
	g_app.create(false);
#endif

	auto w = new GraphicsWindow(&g_app, L"Dota Pubg", uvec2(800, 600), WindowFrame | WindowResizable, true);
	w->window->set_cursor(CursorNone);
	w->s_renderer->set_shadow_props(3, 50.f, 20.f);

	auto script_ins = script::Instance::get_default();
	script_ins->excute_file(L"terrain_scatter.lua");
	script_ins->excute_file(L"scripts/head.lua");
	script_ins->excute_file(L"scripts/attribute.lua");
	script_ins->excute_file(L"scripts/character.lua");
	script_ins->excute_file(L"scripts/player.lua");
	script_ins->excute_file(L"scripts/npc.lua");
	script_ins->excute_file(L"scripts/skill.lua");
	script_ins->excute_file(L"scripts/item.lua");
	script_ins->excute_file(L"scripts/particle.lua");
	script_ins->excute_file(L"scripts/projectile.lua");
	{
		auto e = Entity::create();
		e->load(L"prefabs/main");
		w->root->add_child(e);
	}
	script_ins->excute_file(L"scripts/main.lua");

	//w->s_physics->set_visualization(true);

	looper().add_event([](Capture& c) {
		printf("fps: %d\n", looper().get_fps());
		c._current = nullptr;
	}, Capture(), 1.f);

	g_app.run();

	return 0;
}
