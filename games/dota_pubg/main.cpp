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
		auto n_blocks = 32;
		auto base_height = 35.f;
		auto block_height = 4.f;

		struct Region
		{
			int visited = 0;
			int edges[4] = { 0, 0, 0, 0 }; // L T R B
		};

		std::vector<std::vector<Region>> regions;
		regions.resize(n_blocks / 2);
		for (auto& col : regions)
			col.resize(n_blocks / 2);

		std::function<bool(int x, int y)> visit;
		std::random_device rd;
		std::mt19937 g(rd());
		visit = [&](int x, int y) {
			auto& r = regions[x][y];
			if (r.visited)
				return false;
			r.visited = 1;

			std::vector<int> order = { 0, 1, 2, 3 };
			std::shuffle(order.begin(), order.end(), g);
			for (auto& i : order)
			{
				switch (i)
				{
				case 0:
					if (x > 0 && visit(x - 1, y))
						r.edges[0] = 1;
					break;
				case 1:
					if (y > 0 && visit(x, y - 1))
						r.edges[1] = 1;
					break;
				case 2:
					if (x < regions.size() - 1 && visit(x + 1, y))
						r.edges[2] = 1;
					break;
				case 3:
					if (y < regions[0].size() - 1 && visit(x, y + 1))
						r.edges[3] = 1;
					break;
				}
			}
			return true;
		};
		visit(0, 0);

		std::vector<std::vector<int>> blocks;
		blocks.resize(n_blocks);
		for (auto& col : blocks)
			col.resize(n_blocks);
		for (auto& col : blocks)
		{
			for (auto& b : col)
				b = 1;
		}
		for (auto y = 0; y < regions[0].size(); y++)
		{
			for (auto x = 0; x < regions.size(); x++)
			{
				auto& r = regions[x][y];
				blocks[x * 2][y * 2] = 0;
				for (auto i = 0; i < 4; i++)
				{
					if (r.edges[i])
					{
						switch (i)
						{
						case 0:
							blocks[x * 2 - 1][y * 2] = 0;
							break;
						case 1:
							blocks[x * 2][y * 2 - 1] = 0;
							break;
						case 2:
							blocks[x * 2 + 1][y * 2] = 0;
							break;
						case 3:
							blocks[x * 2][y * 2 + 1] = 0;
							break;
						}
					}
				}
			}
		}

		std::ofstream file_blocks(L"blocks.txt");
		file_blocks << n_blocks;
		file_blocks << "\n";
		for (auto x = 0; x < blocks.size(); x++)
		{
			for (auto y = 0; y < blocks[0].size(); y++)
			{
				file_blocks << blocks[x][y];
				file_blocks << " ";
			}
			file_blocks << "\n";
		}
		file_blocks.close();

		auto bmp_fbm = Bitmap::create(L"D:\\assets\\terrain\\fbm.png");
		auto fbm_pixels = bmp_fbm->get_data();

		auto img_ext = ivec2(bmp_fbm->get_width(), bmp_fbm->get_height());
		auto block_pixels = 256 / n_blocks;
		auto bmp = Bitmap::create(img_ext.x, img_ext.y, 1);
		auto pixels = bmp->get_data();

		auto h1 = int(base_height / ext_y * 255.f);
		auto h2 = int((block_height + base_height) / ext_y * 255.f);
		for (auto x = 0; x < blocks.size(); x++)
		{
			for (auto y = 0; y < blocks[0].size(); y++)
			{
				auto h = blocks[x][y] ? h2 : h1;
				for (auto yy = y * block_pixels; yy < (y + 1) * block_pixels; yy++)
				{
					for (auto xx = x * block_pixels; xx < (x + 1) * block_pixels; xx++)
					{
						auto idx = img_ext.x * yy + xx;
						if (blocks[x][y])
							pixels[idx] = h2 + fbm_pixels[idx] * 0.2f;
						else
							pixels[idx] = h1 + fbm_pixels[idx] * 0.2f;
					}
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
