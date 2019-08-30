#include <flame/foundation/window.h>
#include <flame/foundation/serialize.h>
#include <flame/graphics/device.h>
#include <flame/graphics/synchronize.h>
#include <flame/graphics/swapchain.h>
#include <flame/graphics/commandbuffer.h>
#include <flame/graphics/image.h>
#include <flame/graphics/font.h>
#include <flame/graphics/canvas.h>
#include <flame/universe/components/element.h>
#include <flame/universe/components/text.h>
#include <flame/universe/components/event_dispatcher.h>
#include <flame/universe/components/event_receiver.h>
#include <flame/universe/components/aligner.h>
#include <flame/universe/components/layout.h>
#include <flame/universe/components/image.h>

using namespace flame;
using namespace graphics;

uint styles[][4][16] = {
	{
		{
			0, 1, 0, 0,
			0, 1, 0, 0,
			0, 1, 0, 0,
			0, 1, 0, 0
		},
		{
			0, 0, 0, 0,
			1, 1, 1, 1,
			0, 0, 0, 0,
			0, 0, 0, 0
		},
		{
			0, 1, 0, 0,
			0, 1, 0, 0,
			0, 1, 0, 0,
			0, 1, 0, 0
		},
		{
			0, 0, 0, 0,
			1, 1, 1, 1,
			0, 0, 0, 0,
			0, 0, 0, 0
		}
	},
	{
		{
			0, 1, 0, 0,
			1, 1, 1, 0,
			0, 0, 0, 0,
			0, 0, 0, 0
		},
		{
			0, 1, 0, 0,
			0, 1, 1, 0,
			0, 1, 0, 0,
			0, 0, 0, 0
		},
		{
			0, 0, 0, 0,
			1, 1, 1, 0,
			0, 1, 0, 0,
			0, 0, 0, 0
		},
		{
			0, 1, 0, 0,
			1, 1, 0, 0,
			0, 1, 0, 0,
			0, 0, 0, 0
		}
	},
	{
		{
			1, 0, 0, 0,
			1, 1, 1, 0,
			0, 0, 0, 0,
			0, 0, 0, 0
		},
		{
			0, 1, 1, 0,
			0, 1, 0, 0,
			0, 1, 0, 0,
			0, 0, 0, 0
		},
		{
			0, 0, 0, 0,
			1, 1, 1, 0,
			0, 0, 1, 0,
			0, 0, 0, 0
		},
		{
			0, 1, 0, 0,
			0, 1, 0, 0,
			1, 1, 0, 0,
			0, 0, 0, 0
		}
	},
	{
		{
			0, 0, 1, 0,
			1, 1, 1, 0,
			0, 0, 0, 0,
			0, 0, 0, 0
		},
		{
			0, 1, 0, 0,
			0, 1, 0, 0,
			0, 1, 1, 0,
			0, 0, 0, 0
		},
		{
			0, 0, 0, 0,
			1, 1, 1, 0,
			1, 0, 0, 0,
			0, 0, 0, 0
		},
		{
			1, 1, 0, 0,
			0, 1, 0, 0,
			0, 1, 0, 0,
			0, 0, 0, 0
		}
	},
	{
		{
			0, 1, 0, 0,
			0, 1, 1, 0,
			0, 0, 1, 0,
			0, 0, 0, 0
		},
		{
			0, 0, 0, 0,
			0, 1, 1, 0,
			1, 1, 0, 0,
			0, 0, 0, 0
		},
		{
			1, 0, 0, 0,
			1, 1, 0, 0,
			0, 1, 0, 0,
			0, 0, 0, 0
		},
		{
			0, 1, 1, 0,
			1, 1, 0, 0,
			0, 0, 0, 0,
			0, 0, 0, 0
		}
	},
	{
		{
			0, 1, 0, 0,
			1, 1, 0, 0,
			1, 0, 0, 0,
			0, 0, 0, 0
		},
		{
			1, 1, 0, 0,
			0, 1, 1, 0,
			0, 0, 0, 0,
			0, 0, 0, 0
		},
		{
			0, 0, 1, 0,
			0, 1, 1, 0,
			0, 1, 0, 0,
			0, 0, 0, 0
		},
		{
			0, 0, 0, 0,
			1, 1, 0, 0,
			0, 1, 1, 0,
			0, 0, 0, 0
		}
	},
	{
		{
			1, 1, 0, 0,
			1, 1, 0, 0,
			0, 0, 0, 0,
			0, 0, 0, 0
		},
		{
			1, 1, 0, 0,
			1, 1, 0, 0,
			0, 0, 0, 0,
			0, 0, 0, 0
		},
		{
			1, 1, 0, 0,
			1, 1, 0, 0,
			0, 0, 0, 0,
			0, 0, 0, 0
		},
		{
			1, 1, 0, 0,
			1, 1, 0, 0,
			0, 0, 0, 0,
			0, 0, 0, 0
		}
	}
};

/*
struct Piece
{
	Vec2i pos;
	TetrisType* t;
	int transform_id;
	int x_move, y_move;
	bool transform;
	bool down;

	void reset();
	void spawn();
	void take_away();
	bool try_pos(const flame::Ivec2& off);
	void print();
};

const auto block_cx = 10;
const auto block_cy = 20;
uint blocks[block_cx * block_cy];

const auto tick = 1.f / 16.f;
const auto down_speed_sample = 16;
float down_speed;
float timer;
auto down_timer = 0;

void Tetris::reset()
{
	pos = Ivec2(0);
	t = nullptr;
	transform_id = 0;
	x_move = 0;
	y_move = false;
	transform = false;
	down = false;
}

static int next_id;

void Tetris::spawn()
{
	reset();

	pos = Ivec2((grid_hori_size - hori_size) / 2, 0);
	t = &tetris_types[next_id];
	next_id = rand() % FLAME_ARRAYSIZE(tetris_types);

	down_speed = down_speed_sample;
}

void Tetris::take_away()
{
	auto vs = t->v[transform_id];
	for (auto y = 0; y < vert_size; y++)
	{
		for (auto x = 0; x < hori_size; x++)
		{
			if (vs[y * hori_size + x])
			{
				auto xx = pos.x() + x;
				auto yy = pos.y() + y;
				if (xx >= 0 && xx < grid_hori_size &&
					yy >= 0 && yy < grid_vert_size)
					grids[yy * grid_hori_size + xx].b = false;
			}
		}
	}
}

bool Tetris::try_pos(const Ivec2& off)
{
	auto vs = t->v[transform_id];
	for (auto y = 0; y < vert_size; y++)
	{
		for (auto x = 0; x < hori_size; x++)
		{
			if (vs[y * hori_size + x])
			{
				auto xx = pos.x() + off.x() + x;
				auto yy = pos.y() + off.y() + y;
				if (xx < 0 || xx > grid_hori_size - 1 ||
					yy < 0 || yy > grid_vert_size - 1 ||
					grids[yy * grid_hori_size + xx].b)
					return false;
			}
		}
	}
	return true;
}

void Tetris::print()
{
	auto vs = t->v[transform_id];
	for (auto y = 0; y < vert_size; y++)
	{
		for (auto x = 0; x < hori_size; x++)
		{
			if (vs[y * Tetris::hori_size + x])
			{
				auto xx = pos.x() + x;
				auto yy = pos.y() + y;
				if (xx >= 0 && xx < grid_hori_size &&
					yy >= 0 && yy < grid_vert_size)
				{
					auto& g = grids[yy * grid_hori_size + xx];
					g.b = true;
					g.h = t->h;
				}
			}
		}
	}
}

Tetris tetris;

bool running;
bool gameover;
bool clean_lines[grid_vert_size];
int clean_timer;

void WidgetGameScene::on_draw(UI::Canvas* c, const Vec2f& off, float scl)
{
	if (running)
	{
		if (gameover)
		{
			running = false;
			gameover = false;
		}
		else if (clean_timer > 0)
		{
			clean_timer--;
			if (clean_timer == 0)
			{
				auto score = 0;
				for (auto i = grid_vert_size - 1; i >= 0; i--)
				{
					if (clean_lines[i])
					{
						for (auto k = i; k > 0; k--)
						{
							for (auto j = 0; j < grid_hori_size; j++)
								grids[k * grid_hori_size + j] = grids[(k - 1) * grid_hori_size + j];
							clean_lines[k] = clean_lines[k - 1];
						}
						for (auto j = 0; j < grid_hori_size; j++)
							grids[j].b = false;
						clean_lines[0] = false;

						score++;
						i++;
					}
				}
			}
		}
		else if (tetris.t == nullptr)
		{
			for (auto i = 0; i < grid_vert_size; i++)
			{
				auto full = true;
				for (auto j = 0; j < grid_hori_size; j++)
				{
					if (!grids[i * grid_hori_size + j].b)
					{
						full = false;
						break;
					}
				}
				clean_lines[i] = full;
				if (full)
				{
					snd_src_success->stop();
					snd_src_success->play();

					clean_timer = clean_time_total;
				}
			}

			if (clean_timer == 0)
			{
				tetris.spawn();
				if (!tetris.try_pos(Ivec2(0)))
					gameover = true;
			}
		}
		else
		{
			tetris.take_away();

			if (tetris.transform)
			{
				snd_src_select->stop();
				snd_src_select->play();

				auto prev_id = tetris.transform_id;
				tetris.transform_id = (tetris.transform_id + 1) % 4;
				if (!tetris.try_pos(Ivec2(0)))
					tetris.transform_id = prev_id;
				tetris.transform = false;
			}
			if (tetris.down)
			{
				if (tetris.gear == 0)
				{
					snd_src_select->stop();
					snd_src_select->play();

					down_speed = 1;
					tetris.gear = 1;
				}
				else if (tetris.gear == 1)
				{
					snd_src_ok->stop();
					snd_src_ok->play();

					tetris.gear = 2;
				}
				tetris.down = false;
			}

			auto bottom_hit = false;

			if (tetris.x_move != 0)
			{
				snd_src_select->stop();
				snd_src_select->play();

				if (tetris.try_pos(Ivec2(tetris.x_move, 0)))
					tetris.pos.x() += tetris.x_move;
				tetris.x_move = 0;
			}
			if (tetris.y_move != 0 || tetris.gear == 2)
			{
				while (true)
				{
					if (tetris.try_pos(Ivec2(0, 1)))
						tetris.pos.y() += 1;
					else
					{
						bottom_hit = true;
						break;
					}
					if (tetris.gear != 2)
						break;
				}
				tetris.y_move = 0;
			}

			tetris.print();

			if (bottom_hit)
			{
				tetris.t = nullptr;
			}
		}

		down_timer++;
		if (down_timer >= down_speed)
		{
			tetris.y_move++;
			down_timer = 0;
		}
	}

	const auto cube_width = 20.f;
	const auto cube_gap = 8.f;

	auto draw_cube = [&](const Vec2f& pen, float h, float w, float A) {
		auto col0 = HSV(h, 0.3f, 1.f, A);
		auto col1 = HSV(h, 0.9f, 0.8f, A);
		auto col2 = HSV(h, 0.75f, 0.9f, A);
		auto col3 = HSV(h, 0.6f, 0.9f, A);
		auto col4 = HSV(h, 0.54f, 0.96f, A);
		auto col5 = HSV(h, 0.85f, 0.68f, A);

		c->add_rect_filled(pen + Vec2f(w), Vec2f(cube_width - w), col4);

		c->path_line_to(pen + Vec2f(w - 4.f));
		c->path_line_to(pen + Vec2f(w));
		c->path_line_to(pen + Vec2f(cube_width - w, w));
		c->path_line_to(pen + Vec2f(cube_width - w + 4.f, w - 4.f));
		c->fill(col0);
		c->stroke(col5, 1.f, true);
		c->clear_path();

		c->path_line_to(pen + Vec2f(cube_width - w, w));
		c->path_line_to(pen + Vec2f(cube_width - w));
		c->path_line_to(pen + Vec2f(cube_width - w + 4.f));
		c->path_line_to(pen + Vec2f(cube_width - w + 4.f, w - 4.f));
		c->fill(col1);
		c->stroke(col5, 1.f, true);
		c->clear_path();

		c->path_line_to(pen + Vec2f(w, cube_width - w));
		c->path_line_to(pen + Vec2f(w - 4.f, cube_width - w + 4.f));
		c->path_line_to(pen + Vec2f(cube_width - w + 4.f));
		c->path_line_to(pen + Vec2f(cube_width - w));
		c->fill(col2);
		c->stroke(col5, 1.f, true);
		c->clear_path();

		c->path_line_to(pen + Vec2f(w - 4.f));
		c->path_line_to(pen + Vec2f(w - 4.f, cube_width - w + 4.f));
		c->path_line_to(pen + Vec2f(w, cube_width - w));
		c->path_line_to(pen + Vec2f(w));
		c->fill(col3);
		c->stroke(col5, 1.f, true);
		c->clear_path();
	};

	auto pos = off + Vec2f(20.f, 30.f);
	auto pen = pos;
	for (auto y = 0; y < grid_vert_size; y++)
	{
		for (auto x = 0; x < grid_hori_size; x++)
		{
			auto& g = grids[y * grid_hori_size + x];
			if (g.b)
				draw_cube(pen, g.h, cube_width * 0.5f, 1.f);
			pen.x() += cube_width + cube_gap;
		}
		pen.x() = pos.x();
		pen.y() += cube_width + cube_gap;
	}

	{
		auto _pos = pos + Vec2f((cube_width + cube_gap) * grid_hori_size + 50.f, 0.f);
		c->add_text_sdf(_pos, Vec4c(0, 0, 0, 255), L"NEXT", 3.f);

		_pos.y() += 60.f;
		auto pen = _pos;

		auto t = &tetris_types[next_id];
		auto vs = t->v[0];
		for (auto y = 0; y < Tetris::vert_size; y++)
		{
			for (auto x = 0; x < Tetris::hori_size; x++)
			{
				if (vs[y * Tetris::hori_size + x])
					draw_cube(pen, t->h, 0.f, 1.f);
				pen.x() += cube_width + cube_gap;
			}
			pen.x() = _pos.x();
			pen.y() += cube_width + cube_gap;
		}

		c->add_rect(_pos - Vec2f(16.f), Vec2f((cube_width + cube_gap) * Tetris::hori_size - cube_gap,
			(cube_width + cube_gap) * Tetris::vert_size - cube_gap) + Vec2f(16.f * 2.f), Vec4c(0, 0, 0, 255), 4.f);
	}

	c->add_rect(pos - Vec2f(6.f), Vec2f((cube_width + cube_gap) * grid_hori_size - cube_gap,
		(cube_width + cube_gap) * grid_vert_size - cube_gap) + Vec2f(6.f * 2.f), Vec4c(0, 0, 0, 255), 4.f);
}

void start_game()
{
	memset(grids, 0, sizeof(grids));

	down_timer = 0;
	next_id = rand() % FLAME_ARRAYSIZE(tetris_types);
	tetris.reset();

	running = true;
	gameover = false;
	for (auto i = 0; i < grid_vert_size; i++)
		clean_lines[i] = false;
	clean_timer = 0;
}

void create_game_frame()
{
	ui->root()->add_widget(-1, w_game_scene, true);

	key_listener = ui->root()->add_keydown_listener([](int key) {
		switch (key)
		{
		case Key_Left:
			tetris.x_move = -1;
			break;
		case Key_Right:
			tetris.x_move = 1;
			break;
		case Key_Up:
			tetris.transform = true;
			break;
		case Key_Down:
			tetris.down = true;
			break;
		break;
		}
	});

	start_game();
}
*/

struct App
{
	Window* w;
	Device* d;
	Semaphore* render_finished;
	SwapchainResizable* scr;
	Fence* fence;
	std::vector<Commandbuffer*> cbs;

	FontAtlas* font_atlas_1;
	FontAtlas* font_atlas_2;
	Canvas* canvas;
	int rt_frame;

	Entity* root;
	cElement* c_element_root;
	cText* c_text_fps;

	void run()
	{
		auto sc = scr->sc();
		auto sc_frame = scr->sc_frame();

		if (sc_frame > rt_frame)
		{
			canvas->set_render_target(TargetImages, sc ? &sc->images() : nullptr);
			rt_frame = sc_frame;
		}

		if (sc)
		{
			sc->acquire_image();
			fence->wait();

			c_element_root->width = w->size.x();
			c_element_root->height = w->size.y();
			c_text_fps->set_text(std::to_wstring(app_fps()));
			root->update();

			auto img_idx = sc->image_index();
			auto cb = cbs[img_idx];
			canvas->record(cb, img_idx);

			d->gq->submit(cb, sc->image_avalible(), render_finished, fence);
			d->gq->present(sc, render_finished);
		}
	}
}app;

int main(int argc, char **args)
{
	app.w = Window::create("Tetris", Vec2u(1280, 20 * 32), WindowFrame);
	app.d = Device::create(true);
	app.render_finished = Semaphore::create(app.d);
	app.scr = SwapchainResizable::create(app.d, app.w);
	app.fence = Fence::create(app.d);
	auto sc = app.scr->sc();
	app.canvas = Canvas::create(app.d, TargetImages, &sc->images());
	app.cbs.resize(sc->images().size());
	for (auto i = 0; i < app.cbs.size(); i++)
		app.cbs[i] = Commandbuffer::create(app.d->gcp);

	auto font1 = Font::create(L"c:/windows/fonts/msyh.ttc", 14);
	auto font2 = Font::create(L"../game/tetris/joystix monospace.ttf", 32);
	app.font_atlas_1 = FontAtlas::create(app.d, FontDrawPixel, { font1 });
	app.font_atlas_1->index = 1;
	app.font_atlas_2 = FontAtlas::create(app.d, FontDrawPixel, { font2 });
	app.font_atlas_2->index = 2;
	app.canvas->set_image(app.font_atlas_1->index, Imageview::create(app.font_atlas_1->image(), Imageview2D, 0, 1, 0, 1, SwizzleOne, SwizzleOne, SwizzleOne, SwizzleR));
	app.canvas->set_image(app.font_atlas_2->index, Imageview::create(app.font_atlas_2->image(), Imageview2D, 0, 1, 0, 1, SwizzleOne, SwizzleOne, SwizzleOne, SwizzleR));

	const auto atlas_id = 3U;
	auto atlas_image = Image::create_from_file(app.d, L"../game/tetris/images/atlas.png");
	app.canvas->set_image(atlas_id, Imageview::create(atlas_image), FilterNearest);

	auto texture_pack = load_texture_pack(L"../game/tetris/images/atlas.png.pack");

	app.root = Entity::create();
	{
		app.c_element_root = cElement::create(app.canvas);
		app.root->add_component(app.c_element_root);

		app.root->add_component(cEventDispatcher::create(app.w));

		app.root->add_component(cLayout::create());
	}

	auto e_fps = Entity::create();
	app.root->add_child(e_fps);
	{
		e_fps->add_component(cElement::create());

		auto c_text = cText::create(app.font_atlas_1);
		app.c_text_fps = c_text;
		e_fps->add_component(c_text);

		auto c_aligner = cAligner::create();
		c_aligner->x_align = AlignxRight;
		c_aligner->y_align = AlignyBottom;
		e_fps->add_component(c_aligner);
	}

	{
		Vec2f uv0 = Vec2f(0.f);
		Vec2f uv1 = Vec2f(0.f);
		for (auto& t : texture_pack)
		{
			if (t.first == "brick.png")
			{
				uv0.x() = (float)t.second.x() / atlas_image->size.x();
				uv0.y() = (float)t.second.y() / atlas_image->size.y();
				uv1.x() = (float)(t.second.x() + t.second.z()) / atlas_image->size.x();
				uv1.y() = (float)(t.second.y() + t.second.w()) / atlas_image->size.y();
				break;
			}
		}

		for (auto i = 0; i < 20; i++)
		{
			auto e_image = Entity::create();
			app.root->add_child(e_image);
			{
				auto c_element = cElement::create();
				c_element->x = 0;
				c_element->y = i * 32;
				c_element->width = 32;
				c_element->height = 32;
				e_image->add_component(c_element);

				auto c_image = cImage::create();
				c_image->id = atlas_id;
				c_image->uv0 = uv0;
				c_image->uv1 = uv1;
				e_image->add_component(c_image);
			}
		}

		for (auto i = 0; i < 20; i++)
		{
			auto e_image = Entity::create();
			app.root->add_child(e_image);
			{
				auto c_element = cElement::create();
				c_element->x = 11 * 32;
				c_element->y = i * 32;
				c_element->width = 32;
				c_element->height = 32;
				e_image->add_component(c_element);

				auto c_image = cImage::create();
				c_image->id = atlas_id;
				c_image->uv0 = uv0;
				c_image->uv1 = uv1;
				e_image->add_component(c_image);
			}
		}
	}

	{
		Vec2f uv0 = Vec2f(0.f);
		Vec2f uv1 = Vec2f(0.f);
		for (auto& t : texture_pack)
		{
			if (t.first == "block.png")
			{
				uv0.x() = (float)t.second.x() / atlas_image->size.x();
				uv0.y() = (float)t.second.y() / atlas_image->size.y();
				uv1.x() = (float)(t.second.x() + t.second.z()) / atlas_image->size.x();
				uv1.y() = (float)(t.second.y() + t.second.w()) / atlas_image->size.y();
				break;
			}
		}

		for (auto i = 0; i < 20; i++)
		{
			for (auto j = 0; j < 10; j++)
			{
				auto e_image = Entity::create();
				app.root->add_child(e_image);
				{
					auto c_element = cElement::create();
					c_element->x = 32 + j * 32;
					c_element->y = i * 32;
					c_element->width = 32;
					c_element->height = 32;
					e_image->add_component(c_element);

					auto c_image = cImage::create();
					c_image->id = atlas_id;
					c_image->uv0 = uv0;
					c_image->uv1 = uv1;
					e_image->add_component(c_image);
				}
			}
		}
	}

	auto e_score_title = Entity::create();
	app.root->add_child(e_score_title);
	{
		auto c_element = cElement::create();
		c_element->x = 13 * 32;
		c_element->y = 1 * 32;
		e_score_title->add_component(c_element);

		auto c_text = cText::create(app.font_atlas_2);
		c_text->set_text(L"SCORE");
		e_score_title->add_component(c_text);
	}

	auto e_level_title = Entity::create();
	app.root->add_child(e_level_title);
	{
		auto c_element = cElement::create();
		c_element->x = 13 * 32;
		c_element->y = 5 * 32;
		e_level_title->add_component(c_element);

		auto c_text = cText::create(app.font_atlas_2);
		c_text->set_text(L"LEVEL");
		e_level_title->add_component(c_text);
	}

	auto e_lines_title = Entity::create();
	app.root->add_child(e_lines_title);
	{
		auto c_element = cElement::create();
		c_element->x = 13 * 32;
		c_element->y = 7 * 32;
		e_lines_title->add_component(c_element);

		auto c_text = cText::create(app.font_atlas_2);
		c_text->set_text(L"LINES");
		e_lines_title->add_component(c_text);
	}

	app_run([](void* c) {
		auto app = (*(App**)c);
		app->run();
	}, new_mail_p(&app));

	return 0;
}
