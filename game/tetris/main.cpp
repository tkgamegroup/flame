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
#include <flame/universe/components/aligner.h>
#include <flame/universe/components/layout.h>
#include <flame/universe/components/image.h>

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
			blocks[i]->visible = d[i] == 1;

		down_timer.max = 50;
		down_timer.curr = 0;
	}
};

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
	cEventDispatcher* c_event_dispatcher_root;
	cText* c_text_fps;

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
				if (piece.blocks[i * 4 + j]->visible)
				{
					auto x = piece.x + j + x_off;
					auto y = piece.y + i + y_off;
					if (x < 0 || x >= block_cx ||
						y < 0 || y >= block_cy)
						return false;
					if (blocks[y * block_cx + x]->visible)
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
					if (blocks[y * block_cx + x]->visible)
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
			auto& key_states = c_event_dispatcher_root->key_states;
			if (key_states[Key_Up] == (KeyStateDown | KeyStateJust))
			{
				auto index = (piece.transform_index + 1) % 4;
				if (check(styles[piece.style_index][index]))
				{
					piece.transform_index = index;
					auto& d = styles[piece.style_index][index];
					for (auto i = 0; i < 16; i++)
						piece.blocks[i]->visible = d[i] == 1;
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
						if (piece.blocks[i * 4 + j]->visible)
						{
							auto x = piece.x + j;
							auto y = piece.y + i;
							blocks[y * block_cx + x]->visible = true;
						}
					}
				}

				auto l = 0;
				for (auto i = block_cy - 1; i >= 0; i--)
				{
					auto full = true;
					for (auto j = 0; j < block_cx; j++)
					{
						if (!blocks[i * block_cx + j]->visible)
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
							blocks[ii * block_cx + j]->visible = blocks[(ii - 1) * block_cx + j]->visible;
					}
					for (auto i = 0; i < block_cx; i++)
						blocks[i]->visible = false;
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

			piece.root->pos.x() = (piece.x + 1) * 32;
			piece.root->pos.y() = piece.y * 32;
		}

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
			looper().process_delay_events();

			c_element_root->size = w->size;
			c_text_fps->set_text(std::to_wstring(looper().fps));
			root->update();

			auto img_idx = sc->image_index();
			auto cb = cbs[img_idx];
			canvas->record(cb, img_idx);

			d->gq->submit({ cb }, sc->image_avalible(), render_finished, fence);
			d->gq->present(sc, render_finished);
		}
	}
}app;

int main(int argc, char **args)
{
	app.w = Window::create("Tetris", Vec2u(18 * block_size, block_cy * block_size), WindowFrame);
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
	auto font2 = Font::create(L"../game/tetris/joystix monospace.ttf", block_size);
	app.font_atlas_1 = FontAtlas::create(app.d, FontDrawPixel, { font1 });
	app.font_atlas_1->index = app.canvas->set_image(-1, app.font_atlas_1->imageview());
	app.font_atlas_2 = FontAtlas::create(app.d, FontDrawPixel, { font2 });
	app.font_atlas_2->index = app.canvas->set_image(-1, app.font_atlas_2->imageview());

	auto atlas_image = Image::create_from_file(app.d, L"../game/tetris/images/atlas.png");
	const auto atlas_id = app.canvas->set_image(-1, Imageview::create(atlas_image), FilterNearest);

	auto texture_pack = load_texture_pack(L"../game/tetris/images/atlas.png.pack");

	app.root = Entity::create();
	{
		app.c_element_root = cElement::create(app.canvas);
		app.root->add_component(app.c_element_root);

		app.c_event_dispatcher_root = cEventDispatcher::create(app.w);
		app.root->add_component(app.c_event_dispatcher_root);

		app.root->add_component(cLayout::create(LayoutFree));
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

	app.gameover = false;
	app.score = 0;
	app.level = 0;
	app.lines = 0;

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

		for (auto i = 0; i < block_cy; i++)
		{
			auto e_image = Entity::create();
			app.root->add_child(e_image);
			{
				auto c_element = cElement::create();
				c_element->pos.x() = 0;
				c_element->pos.y() = i * block_size;
				c_element->size = block_size;
				e_image->add_component(c_element);

				auto c_image = cImage::create();
				c_image->id = atlas_id;
				c_image->uv0 = uv0;
				c_image->uv1 = uv1;
				e_image->add_component(c_image);
			}
		}

		for (auto i = 0; i < block_cy; i++)
		{
			auto e_image = Entity::create();
			app.root->add_child(e_image);
			{
				auto c_element = cElement::create();
				c_element->pos.x() = (block_cx + 1) * block_size;
				c_element->pos.y() = i * block_size;
				c_element->size = block_size;
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

		for (auto i = 0; i < block_cy; i++)
		{
			for (auto j = 0; j < block_cx; j++)
			{
				auto e_image = Entity::create();
				e_image->visible = false;
				app.blocks[i * block_cx + j] = e_image;
				app.root->add_child(e_image);
				{
					auto c_element = cElement::create();
					c_element->pos.x() = block_size + j * block_size;
					c_element->pos.y() = i * block_size;
					c_element->size = block_size;
					e_image->add_component(c_element);

					auto c_image = cImage::create();
					c_image->id = atlas_id;
					c_image->uv0 = uv0;
					c_image->uv1 = uv1;
					e_image->add_component(c_image);
				}
			}
		}

		auto e_root = Entity::create();
		app.root->add_child(e_root);
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
					c_element->x = j * block_size;
					c_element->y = i * block_size;
					c_element->width = block_size;
					c_element->height = block_size;
					c_element->alpha = 0.8f;
					e_image->add_component(c_element);

					auto c_image = cImage::create();
					c_image->id = atlas_id;
					c_image->uv0 = uv0;
					c_image->uv1 = uv1;
					e_image->add_component(c_image);
				}
			}
		}
		app.piece.reset();
	}

	auto e_score_title = Entity::create();
	app.root->add_child(e_score_title);
	{
		auto c_element = cElement::create();
		c_element->x = 13 * block_size;
		c_element->y = 1 * block_size;
		e_score_title->add_component(c_element);

		auto c_text = cText::create(app.font_atlas_2);
		c_text->set_text(L"SCORE");
		e_score_title->add_component(c_text);
	}

	auto e_score = Entity::create();
	app.root->add_child(e_score);
	{
		auto c_element = cElement::create();
		c_element->x = 16 * block_size;
		c_element->y = 2 * block_size;
		e_score->add_component(c_element);

		app.text_score = cText::create(app.font_atlas_2);
		app.text_score->right_align = true;
		e_score->add_component(app.text_score);
	}

	auto e_level_title = Entity::create();
	app.root->add_child(e_level_title);
	{
		auto c_element = cElement::create();
		c_element->x = 13 * block_size;
		c_element->y = 5 * block_size;
		e_level_title->add_component(c_element);

		auto c_text = cText::create(app.font_atlas_2);
		c_text->set_text(L"LEVEL");
		e_level_title->add_component(c_text);
	}

	auto e_level = Entity::create();
	app.root->add_child(e_level);
	{
		auto c_element = cElement::create();
		c_element->x = 16 * block_size;
		c_element->y = 6 * block_size;
		e_level->add_component(c_element);

		app.text_level = cText::create(app.font_atlas_2);
		app.text_score->right_align = true;
		e_level->add_component(app.text_level);
	}

	auto e_lines_title = Entity::create();
	app.root->add_child(e_lines_title);
	{
		auto c_element = cElement::create();
		c_element->x = 13 * block_size;
		c_element->y = 9 * block_size;
		e_lines_title->add_component(c_element);

		auto c_text = cText::create(app.font_atlas_2);
		c_text->set_text(L"LINES");
		e_lines_title->add_component(c_text);
	}

	auto e_lines = Entity::create();
	app.root->add_child(e_lines);
	{
		auto c_element = cElement::create();
		c_element->x = 16 * block_size;
		c_element->y = 10 * block_size;
		e_lines->add_component(c_element);

		app.text_lines = cText::create(app.font_atlas_2);
		e_lines->add_component(app.text_lines);
	}

	looper().loop([](void* c) {
		(*(App**)c)->run();
	}, new_mail_p(&app));

	return 0;
}
