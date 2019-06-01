// MIT License
// 
// Copyright (c) 2019 wjs
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "share.h"
#include "game_frame.h"
#include "title_frame.h"

#include <flame/window.h>
#include <flame/UI/canvas.h>
#include <flame/UI/widgets/button.h>
#include <flame/UI/dialogs/yesno_dialog.h>
#include <flame/UI/styles/button_style_color.h>

using namespace flame;

const auto grid_hori_size = 10;
const auto grid_vert_size = 20;
struct Grid
{
	bool b;
	float h;
};
Grid grids[grid_hori_size * grid_vert_size];

const auto tick = 1.f / 16.f;
const auto down_speed_sample = 16;
float down_speed;
float timer;
auto down_timer = 0;

TetrisType tetris_types[] = {
	{
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
		0.f
	},
	{
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
		120.f
	},
	{
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
		30.f
	},
	{
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
		300.f
	},
	{
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
		60.f
	},
	{
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
		320.f
	},
	{
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
		},
		200.f
	}
};

auto show_prober = false;
Ivec2 prober_pos;

void Tetris::reset()
{
	pos = Ivec2(0); 
	t = nullptr;
	transform_id = 0;
	x_move = 0;
	y_move = false;
	transform = false;
	down = false;
	gear = 0;
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

bool Tetris::try_pos(const Ivec2 &off)
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
					auto &g = grids[yy * grid_hori_size + xx];
					g.b = true;
					g.h = t->h;
				}
			}
		}
	}
}

Tetris tetris;

struct WidgetGameScene : UI::Widget
{
	WidgetGameScene(UI::Instance *ui);
	~WidgetGameScene();

	virtual void on_draw(UI::Canvas *c, const Vec2f &off, float scl) override;
};

WidgetGameScene::WidgetGameScene(UI::Instance *ui) :
	UI::Widget(ui)
{
	timer = ui->total_time();
	srand(time(0));
}

WidgetGameScene::~WidgetGameScene()
{
}

bool running;
bool gameover;
bool clean_lines[grid_vert_size];
const auto clean_time_total = 4;
int clean_timer;

void WidgetGameScene::on_draw(UI::Canvas *c, const Vec2f &off, float scl)
{
	if (running)
	{
		auto curr_time = ui->total_time();
		if (curr_time - timer >= tick)
		{
			auto set_prober = []() {
				auto i = 1;
				while (true)
				{
					if (!tetris.try_pos(Ivec2(0, i)))
						break;
					i++;
				}

				prober_pos = tetris.pos;
				prober_pos.y() += i - 1;
			};

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
					else
					{
						show_prober = true;
						set_prober();
					}
				}
			}
			else
			{
				tetris.take_away();

				auto need_update_prober = false;

				if (tetris.transform)
				{
					snd_src_select->stop();
					snd_src_select->play();

					auto prev_id = tetris.transform_id;
					tetris.transform_id = (tetris.transform_id + 1) % 4;
					if (!tetris.try_pos(Ivec2(0)))
						tetris.transform_id = prev_id;
					else
						need_update_prober = true;
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
					{
						tetris.pos.x() += tetris.x_move;
						need_update_prober = true;
					}
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
				if (!bottom_hit && need_update_prober)
					set_prober();

				tetris.print();

				if (bottom_hit)
				{
					tetris.t = nullptr;
					show_prober = false;
				}
			}

			down_timer++;
			if (down_timer >= down_speed)
			{
				tetris.y_move++;
				down_timer = 0;
			}

			timer = curr_time;
		}
	}

	const auto cube_width = 20.f;
	const auto cube_gap = 8.f;

	auto draw_cube = [&](const Vec2f &pen, float h, float w, float A) {
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
			auto &g = grids[y * grid_hori_size + x];
			if (g.b)
			{
				auto w = 0.f, A = 1.f;
				if (clean_lines[y])
				{
					A = clean_timer / (float)clean_time_total;
					w = (1.f - A) * cube_width * 0.5f;
				}

				draw_cube(pen, g.h, w, A);
			}
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

	if (show_prober)
	{
		auto _pos = pos + Vec2f(prober_pos) * Vec2f(cube_width + cube_gap);
		auto pen = _pos;
		auto col = HSV(tetris.t->h, 0.85f, 0.68f, 1.f);

		auto vs = tetris.t->v[tetris.transform_id];
		for (auto y = 0; y < Tetris::vert_size; y++)
		{
			for (auto x = 0; x < Tetris::hori_size; x++)
			{
				if (vs[y * Tetris::hori_size + x])
					c->add_rect(pen - Vec2f(3.f), Vec2f(cube_width + 6.f), col, 1.f);
				pen.x() += cube_width + cube_gap;
			}
			pen.x() = _pos.x();
			pen.y() += cube_width + cube_gap;
		}
	}

	c->add_rect(pos - Vec2f(6.f), Vec2f((cube_width + cube_gap) * grid_hori_size - cube_gap,
		(cube_width + cube_gap) * grid_vert_size - cube_gap) + Vec2f(6.f * 2.f), Vec4c(0, 0, 0, 255), 4.f);
}

void start_game()
{
	memset(grids, 0, sizeof(grids));

	down_timer = 0;
	show_prober = false;
	next_id = rand() % FLAME_ARRAYSIZE(tetris_types);
	tetris.reset();

	running = true;
	gameover = false;
	for (auto i = 0; i < grid_vert_size; i++)
		clean_lines[i] = false;
	clean_timer = 0;
}

WidgetGameScene *w_game_scene;
void *key_listener;

void create_game_frame()
{
	w_game_scene = new WidgetGameScene(ui);

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
		case Key_Esc:
		{
			snd_src_back->stop();
			snd_src_back->play();

			running = false;

			auto d = new UI::YesNoDialog(ui, L"", 2.f, L"Paused", L"Main Menu", L"Resume", [](bool ok) {
				snd_src_ok->stop();
				snd_src_ok->play();

				if (!ok)
					running = true;
				else
				{
					destroy_game_frame();
					create_title_frame();
				}
			});
			d->w_yes->add_mouseenter_listener([]() {
				snd_src_select->stop();
				snd_src_select->play();
			});
			d->w_no->add_mouseenter_listener([]() {
				snd_src_select->stop();
				snd_src_select->play();
			});
		}
			break;
		}
	});

	start_game();
}

void destroy_game_frame()
{
	ui->root()->remove_widget(w_game_scene, true);
	ui->root()->remove_keydown_listener(key_listener);

	w_game_scene = nullptr;
}
