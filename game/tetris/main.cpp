#include <flame/foundation/bitmap.h>
#include <flame/foundation/serialize.h>
#include <flame/foundation/blueprint.h>
#include <flame/graphics/device.h>
#include <flame/graphics/synchronize.h>
#include <flame/graphics/swapchain.h>
#include <flame/graphics/commandbuffer.h>
#include <flame/graphics/image.h>
#include <flame/graphics/font.h>
#include <flame/universe/world.h>
#include <flame/universe/systems/layout_management.h>
#include <flame/universe/systems/2d_renderer.h>
#include <flame/universe/systems/event_dispatcher.h>
#include <flame/universe/components/element.h>
#include <flame/universe/components/text.h>
#include <flame/universe/components/aligner.h>
#include <flame/universe/components/layout.h>
#include <flame/universe/components/image.h>

#include "../renderpath/canvas/canvas.h"

using namespace flame;
using namespace graphics;

uchar styles[][4][16] = {
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

const auto block_cx = 10;
const auto block_cy = 20;
const auto block_size = 32.f;

struct Timer
{
	uint max;
	uint curr;

	bool step()
	{
		if (curr++ >= max)
		{
			curr = 0;
			return true;
		}
		return false;
	}
};

struct Piece
{
	uint x;
	uint y;
	uint style_index;
	uint transform_index;

	cElement* root;
	Entity* blocks[16];

	Timer down_timer;

	void reset()
	{
		x = (block_cx - 4) / 2;
		y = 0;
		style_index = rand() % FLAME_ARRAYSIZE(styles);

		auto& d = styles[style_index][transform_index];
		for (auto i = 0; i < 16; i++)
			blocks[i]->visibility_ = (d[i] == 1);

		down_timer.max = 50;
		down_timer.curr = 0;
	}
};

struct App
{
	Window* w;
	Device* d;
	SwapchainResizable* scr;
	Fence* fence;
	std::vector<Commandbuffer*> cbs;
	Semaphore* render_finished;
	BP* canvas_bp;
	Canvas* canvas;
	std::vector<TypeinfoDatabase*> dbs;

	Universe* u;
	sEventDispatcher* event_dispatcher;
	cElement* c_element_root;

	bool gameover;
	uint score;
	uint level;
	uint lines;
	cText* text_score;
	cText* text_level;
	cText* text_lines;
	Entity* blocks[block_cy * block_cx];
	Piece piece;

	bool check(int x_off, int y_off)
	{
		for (auto i = 0; i < 4; i++)
		{
			for (auto j = 0; j < 4; j++)
			{
				if (piece.blocks[i * 4 + j]->visibility_)
				{
					auto x = piece.x + j + x_off;
					auto y = piece.y + i + y_off;
					if (x < 0 || x >= block_cx ||
						y < 0 || y >= block_cy)
						return false;
					if (blocks[y * block_cx + x]->visibility_)
						return false;
				}
			}
		}

		return true;
	}

	bool check(uchar* d)
	{
		for (auto i = 0; i < 4; i++)
		{
			for (auto j = 0; j < 4; j++)
			{
				if (d[i * 4 + j])
				{
					auto x = piece.x + j;
					auto y = piece.y + i;
					if (x < 0 || x >= block_cx ||
						y < 0 || y >= block_cy)
						return false;
					if (blocks[y * block_cx + x]->visibility_)
						return false;
				}
			}
		}

		return true;
	}

	void run()
	{
		if (!gameover)
		{
			auto& key_states = event_dispatcher->key_states;
			if (key_states[Key_Up] == (KeyStateDown | KeyStateJust))
			{
				auto index = (piece.transform_index + 1) % 4;
				if (check(styles[piece.style_index][index]))
				{
					piece.transform_index = index;
					auto& d = styles[piece.style_index][index];
					for (auto i = 0; i < 16; i++)
						piece.blocks[i]->visibility_ = (d[i] == 1);
				}
			}
			auto x_off = 0;
			if (key_states[Key_Left] == (KeyStateDown | KeyStateJust))
				x_off -= 1;
			if (key_states[Key_Right] == (KeyStateDown | KeyStateJust))
				x_off += 1;
			if (x_off != 0)
			{
				if (check(x_off, 0))
					piece.x += x_off;
			}
			auto touch_bottom = false;
			if (key_states[Key_Down] == (KeyStateDown | KeyStateJust))
			{
				while (check(0, 1))
					piece.y++;
				touch_bottom = true;
			}
			else
			{
				if (piece.down_timer.step())
				{
					if (check(0, 1))
						piece.y++;
					else
						touch_bottom = true;
				}
			}
			if (touch_bottom)
			{
				for (auto i = 0; i < 4; i++)
				{
					for (auto j = 0; j < 4; j++)
					{
						if (piece.blocks[i * 4 + j]->visibility_)
						{
							auto x = piece.x + j;
							auto y = piece.y + i;
							blocks[y * block_cx + x]->visibility_ = true;
						}
					}
				}

				auto l = 0;
				for (auto i = block_cy - 1; i >= 0; i--)
				{
					auto full = true;
					for (auto j = 0; j < block_cx; j++)
					{
						if (!blocks[i * block_cx + j]->visibility_)
						{
							full = false;
							break;
						}
					}
					if (!full)
						continue;
					for (auto ii = i; ii > 0; ii--)
					{
						for (auto j = 0; j < block_cx; j++)
							blocks[ii * block_cx + j]->visibility_ = blocks[(ii - 1) * block_cx + j]->visibility_;
					}
					for (auto i = 0; i < block_cx; i++)
						blocks[i]->visibility_ = false;
					l++;

					i++;
				}
				lines += l;
				score += l * l;

				piece.reset();
				if (!check(0, 0))
					gameover = true;
			}

			text_score->set_text(std::to_wstring(score));
			text_level->set_text(std::to_wstring(level));
			text_lines->set_text(std::to_wstring(lines));

			piece.root->pos_.x() = (piece.x + 1) * 32;
			piece.root->pos_.y() = piece.y * 32;
		}

		auto sc = scr->sc();

		if (sc)
			sc->acquire_image();

		fence->wait();
		looper().process_events();

		if (sc)
		{
			c_element_root->set_size(Vec2f(w->size));
			u->update();
		}
		canvas_bp->update();

		if (sc)
		{
			d->gq->submit({ cbs[sc->image_index()] }, sc->image_avalible(), render_finished, fence);
			d->gq->present(sc, render_finished);
		}
	}
}app;

int main(int argc, char **args)
{
	app.w = Window::create("Tetris", Vec2u(18 * block_size, block_cy * block_size), WindowFrame);
	app.d = Device::create(true);
	app.scr = SwapchainResizable::create(app.d, app.w);
	app.fence = Fence::create(app.d);
	app.cbs.resize(app.scr->sc()->images().size());
	for (auto i = 0; i < app.cbs.size(); i++)
		app.cbs[i] = Commandbuffer::create(app.d->gcp);
	app.render_finished = Semaphore::create(app.d);
	app.dbs.push_back(TypeinfoDatabase::load(app.dbs, L"flame_foundation.typeinfo"));
	app.dbs.push_back(TypeinfoDatabase::load(app.dbs, L"flame_graphics.typeinfo"));
	app.dbs.push_back(TypeinfoDatabase::load(app.dbs, L"flame_universe.typeinfo"));

	app.canvas_bp = BP::create_from_file(L"../renderpath/canvas/bp", true);
	app.scr->link_bp(app.canvas_bp, app.cbs);
	app.canvas_bp->update();
	app.canvas = (Canvas*)app.canvas_bp->find_output("*.make_cmd.canvas")->data_p();

	auto font_atlas_standard = FontAtlas::create(app.d, FontDrawPixel, { L"c:/windows/fonts/msyh.ttc" });
	app.canvas->add_font(font_atlas_standard);

	app.u = Universe::create();
	app.u->add_object(app.w);
	app.u->add_object(app.canvas);

	auto w = World::create_from_file(app.u, app.dbs, L"../game/tetris/world");
	app.event_dispatcher = w->get_system(sEventDispatcher);

	auto font_atlas_joystix = (FontAtlas*)w->find_object(cH("FontAtlas"), 0);

	auto atlas_main = (Atlas*)w->find_object(cH("Atlas"), cH("release/main.png"));

	auto brick_idx = atlas_main->find_region(cH("../asset/brick.png"));
	auto block_idx = atlas_main->find_region(cH("../asset/block.png"));

	auto main_scene = Entity::create_from_file(w, app.dbs, L"../game/tetris/main.prefab");

	auto root = w->root();
	{
		app.c_element_root = cElement::create();
		root->add_component(app.c_element_root);

		root->add_component(cLayout::create(LayoutFree));
	}

	root->add_child(main_scene);

	auto e_fps = Entity::create();
	root->add_child(e_fps);
	{
		e_fps->add_component(cElement::create());

		auto c_text = cText::create(font_atlas_standard);
		e_fps->add_component(c_text);

		auto c_aligner = cAligner::create();
		c_aligner->x_align_ = AlignxRight;
		c_aligner->y_align_ = AlignyBottom;
		e_fps->add_component(c_aligner);

		add_fps_listener([](void* c, uint fps) {
			(*(cText**)c)->set_text(std::to_wstring(fps));
		}, new_mail_p(c_text));
	}

	app.gameover = false;
	app.score = 0;
	app.level = 0;
	app.lines = 0;

	for (auto i = 0; i < block_cy; i++)
	{
		for (auto j = 0; j < block_cx; j++)
		{
			auto e_image = Entity::create();
			e_image->visibility_ = false;
			app.blocks[i * block_cx + j] = e_image;
			root->add_child(e_image);
			{
				auto c_element = cElement::create();
				c_element->pos_.x() = block_size + j * block_size;
				c_element->pos_.y() = i * block_size;
				c_element->size_ = block_size;
				e_image->add_component(c_element);

				auto c_image = cImage::create();
				c_image->id = (atlas_main->canvas_slot_ << 16) + block_idx;
				e_image->add_component(c_image);
			}
		}
	}

	auto e_root = Entity::create();
	root->add_child(e_root);
	app.piece.root = cElement::create();
	e_root->add_component(app.piece.root);
	for (auto i = 0; i < 4; i++)
	{
		for (auto j = 0; j < 4; j++)
		{
			auto e_image = Entity::create();
			app.piece.blocks[i * 4 + j] = e_image;
			e_root->add_child(e_image);
			{
				auto c_element = cElement::create();
				c_element->pos_.x() = j * block_size;
				c_element->pos_.y() = i * block_size;
				c_element->size_.x() = block_size;
				c_element->size_.y() = block_size;
				c_element->alpha_ = 0.8f;
				e_image->add_component(c_element);

				auto c_image = cImage::create();
				c_image->id = (atlas_main->canvas_slot_ << 16) + block_idx;
				e_image->add_component(c_image);
			}
		}
	}
	app.piece.reset();

	auto e_score = Entity::create();
	root->add_child(e_score);
	{
		auto c_element = cElement::create();
		c_element->pos_.x() = 16 * block_size;
		c_element->pos_.y() = 2 * block_size;
		e_score->add_component(c_element);

		app.text_score = cText::create(font_atlas_joystix);
		e_score->add_component(app.text_score);
	}

	auto e_level = Entity::create();
	root->add_child(e_level);
	{
		auto c_element = cElement::create();
		c_element->pos_.x() = 16 * block_size;
		c_element->pos_.y() = 6 * block_size;
		e_level->add_component(c_element);

		app.text_level = cText::create(font_atlas_joystix);
		e_level->add_component(app.text_level);
	}

	auto e_lines = Entity::create();
	root->add_child(e_lines);
	{
		auto c_element = cElement::create();
		c_element->pos_.x() = 16 * block_size;
		c_element->pos_.y() = 10 * block_size;
		e_lines->add_component(c_element);

		app.text_lines = cText::create(font_atlas_joystix);
		e_lines->add_component(app.text_lines);
	}

	looper().loop([](void* c) {
		(*(App**)c)->run();
	}, new_mail_p(&app));

	return 0;
}
