#include <flame/graphics/font.h>
#include <flame/universe/systems/2d_renderer.h>
#include <flame/universe/components/timer.h>
#include <flame/universe/components/element.h>
#include "text_private.h"
#include <flame/universe/components/event_receiver.h>
#include <flame/universe/components/aligner.h>
#include <flame/universe/components/edit.h>
#include <flame/universe/utils/style.h>

#include "../renderpath/canvas/canvas.h"

namespace flame
{
	struct cEditPrivate : cEdit
	{
		void* key_listener;
		void* mouse_listener;
		void* focus_listener;
		void* draw_cmd;

		bool show_cursor;

		cEditPrivate()
		{
			timer = nullptr;
			element = nullptr;
			text = nullptr;
			event_receiver = nullptr;

			cursor = 0;

			key_listener = nullptr;
			mouse_listener = nullptr;
			focus_listener = nullptr;
			draw_cmd = nullptr;

			show_cursor = false;
		}

		~cEditPrivate()
		{
			if (!entity->dying_)
			{
				timer->set_callback(nullptr, Mail());
				element->cmds.remove(draw_cmd);
				event_receiver->key_listeners.remove(key_listener);
				event_receiver->mouse_listeners.remove(mouse_listener);
				event_receiver->mouse_listeners.remove(focus_listener);
			}
		}

		void flash_cursor(int mode)
		{
			auto render = element->renderer;
			if (render)
				render->pending_update = true;
			if (mode == 1)
				show_cursor = false;
			else if (mode == 2)
			{
				timer->reset();
				show_cursor = true;
			}
			else
				show_cursor = !show_cursor;
		}

		int locate_cursor(const Vec2i& pos)
		{
			auto c_text = ((cTextPrivate*)text);
			auto& str = c_text->text;
			auto font_atlas = c_text->font_atlas;
			auto font_size = c_text->font_size_ * element->global_scale;
			auto p = element->content_min();

			auto y = p.y();
			auto i = 0;
			for (; i < str.size(); )
			{
				if (pos.y() < y + font_size)
				{
					auto x = p.x();
					while (true)
					{
						auto ch = str[i];
						if (ch == '\n')
							break;
						auto w = font_atlas->get_glyph(ch, font_size)->advance;
						if (pos.x() < x + w / 2)
							break;
						x += w;
						i++;
						if (i == str.size())
							break;
					}
					break;
				}

				while (i < str.size())
				{
					if (str[i++] == '\n')
					{
						y += font_size;
						break;
					}
				}
			}
			return i;
		}

		void on_component_added(Component* c) override
		{
			if (c->name_hash == FLAME_CHASH("cTimer") && c->id == FLAME_CHASH("edit"))
			{
				timer = (cTimer*)c;
				timer->set_callback([](void* c) {
					(*(cEditPrivate**)c)->flash_cursor(0);
				}, Mail::from_p(this), false);
			}
			else if (c->name_hash == FLAME_CHASH("cElement"))
			{
				element = (cElement*)c;
				draw_cmd = element->cmds.add([](void* c, graphics::Canvas* canvas) {
					(*(cEditPrivate**)c)->draw(canvas);
					return true;
				}, Mail::from_p(this));
			}
			else if (c->name_hash == FLAME_CHASH("cText"))
				text = (cText*)c;
			else if (c->name_hash == FLAME_CHASH("cEventReceiver"))
			{
				event_receiver = (cEventReceiver*)c;
				key_listener = event_receiver->key_listeners.add([](void* c, KeyStateFlags action, int value) {
					auto thiz = *(cEditPrivate**)c;
					auto& cursor = thiz->cursor;
					auto c_text = (cTextPrivate*)thiz->text;
					auto str = c_text->text;
					auto changed = false;

					if (action == KeyStateNull)
					{
						switch (value)
						{
						case L'\b':
							if (cursor > 0)
							{
								cursor--;
								str.erase(str.begin() + cursor);
								changed = true;
							}
							break;
						case 22:
						{
							auto cb = get_clipboard();
							str = str.substr(0, cursor) + cb.str() + str.substr(cursor);
							cursor += cb.s;
							changed = true;
						}
							break;
						case 27:
							break;
						case 13:
							value = '\n';
						default:
							str.insert(str.begin() + cursor, value);
							cursor++;
							changed = true;
						}
					}
					else if (action == KeyStateDown)
					{
						switch (value)
						{
						case Key_Left:
							if (cursor > 0)
								cursor--;
							break;
						case Key_Right:
							if (cursor < str.size())
								cursor++;
							break;
						case Key_Home:
							cursor = 0;
							break;
						case Key_End:
							cursor = str.size();
							break;
						case Key_Del:
							if (cursor < str.size())
							{
								str.erase(str.begin() + cursor);
								changed = true;
							}
							break;
						}
					}

					if (changed)
						c_text->set_text(str.c_str());
					thiz->flash_cursor(2);

					return true;
				}, Mail::from_p(this));

				mouse_listener = event_receiver->mouse_listeners.add([](void* c, KeyStateFlags action, MouseKey key, const Vec2i& pos) {
					if (is_mouse_down(action, key, true) && key == Mouse_Left)
					{
						auto thiz = *(cEditPrivate**)c;
						thiz->cursor = thiz->locate_cursor(pos);
						thiz->flash_cursor(2);
					}
					return true;
				}, Mail::from_p(this));

				focus_listener = event_receiver->focus_listeners.add([](void* c, bool focusing) {
					auto thiz = *(cEditPrivate**)c;
					if (focusing)
						thiz->timer->start();
					else
					{
						thiz->timer->stop();
						thiz->flash_cursor(1);
					}
					return true;
				}, Mail::from_p(this));
			}
		}

		void draw(graphics::Canvas* canvas)
		{
			if (show_cursor && !element->clipped)
			{
				auto c_text = (cTextPrivate*)text;
				auto& str = c_text->text;
				auto font_atlas = c_text->font_atlas;
				auto font_size = c_text->font_size_ * element->global_scale;

				std::vector<Vec2f> points;
				points.push_back(element->content_min() + Vec2f(font_atlas->text_offset(font_size, str.c_str(), str.c_str() + cursor)));
				points[0].x()++;
				points.push_back(points[0] + Vec2f(0.f, font_size));
				canvas->stroke(points.size(), points.data(), c_text->color_.new_proply<3>(element->alpha_), 2.f);
			}
		}
	};

	cEdit* cEdit::create()
	{
		return new cEditPrivate();
	}
}
