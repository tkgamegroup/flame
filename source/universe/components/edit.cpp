#include <flame/graphics/font.h>
#include <flame/universe/systems/event_dispatcher.h>
#include <flame/universe/systems/2d_renderer.h>
#include <flame/universe/components/timer.h>
#include <flame/universe/components/element.h>
#include "text_private.h"
#include <flame/universe/components/event_receiver.h>
#include <flame/universe/components/aligner.h>
#include <flame/universe/components/edit.h>
#include <flame/universe/utils/style.h>
#include <flame/universe/utils/event.h>

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

			select_start = 0;
			select_end = 0;

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
					auto& select_start = thiz->select_start;
					auto& select_end = thiz->select_end;
					auto c_text = (cTextPrivate*)thiz->text;
					auto str = c_text->text;
					auto changed = false;

					if (action == KeyStateNull)
					{
						switch (value)
						{
						case L'\b':
							if (select_start > 0)
							{
								select_start--;
								select_end = select_start;
								str.erase(str.begin() + select_start);
								changed = true;
							}
							break;
						case 22:
						{
							auto cb = get_clipboard();
							str = str.substr(0, select_start) + cb.str() + str.substr(select_start);
							select_start += cb.s;
							select_end = select_start;
							changed = true;
						}
							break;
						case 27:
							break;
						case 13:
							value = '\n';
						default:
							str.insert(str.begin() + select_start, value);
							select_start++;
							select_end = select_start;
							changed = true;
						}
					}
					else if (action == KeyStateDown)
					{
						switch (value)
						{
						case Key_Left:
							if (select_start > 0)
							{
								select_start--;
								select_end = select_start;
							}
							break;
						case Key_Right:
							if (select_start < str.size())
							{
								select_start++;
								select_end = select_start;
							}
							break;
						case Key_Home:
							select_start = 0;
							select_end = select_start;
							break;
						case Key_End:
							select_start = str.size();
							select_end = select_start;
							break;
						case Key_Del:
							if (select_start < str.size())
							{
								str.erase(str.begin() + select_start);
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
					auto thiz = *(cEditPrivate**)c;
					if (action == (KeyStateDown | KeyStateJust) && key == Mouse_Left)
					{
						thiz->select_start = thiz->select_end = thiz->locate_cursor(pos);
						thiz->flash_cursor(2);
					}
					else if (is_mouse_move(action, key) && utils::is_active(thiz->event_receiver))
					{
						thiz->select_end = thiz->locate_cursor(thiz->event_receiver->dispatcher->mouse_pos);
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
			if (!element->clipped)
			{
				auto c_text = (cTextPrivate*)text;
				auto& str = c_text->text;
				auto font_atlas = c_text->font_atlas;
				auto font_size = c_text->font_size_ * element->global_scale;
				auto p = element->content_min();

				if (select_start != select_end)
				{
					auto i = min(select_start, select_end);
					auto end = max(select_start, select_end);
					while (i < end)
					{
						auto left = i;
						auto right = left + 1;
						auto is_last_new_line = false;
						while (right < end)
						{
							if (str[right] == '\n')
								break;
							right++;
						}
						i = right + 1;
						
						auto sb = str.c_str() + left, se = str.c_str() + right;
						std::vector<Vec2f> points;
						auto pos1 = Vec2f(font_atlas->text_offset(font_size, str.c_str(), sb));
						auto pos2 = Vec2f(font_atlas->text_offset(font_size, str.c_str(), se));
						if (right < end && str[right] == '\n')
							pos2.x() += 4.f;
						path_rect(points, pos1 + p, Vec2f(pos2.x() - pos1.x(), font_size));
						canvas->fill(points.size(), points.data(), Vec4c(128, 128, 255, 255));
						canvas->add_text(font_atlas, sb, se, font_size, pos1 + p, Vec4c(255));
					}
				}

				if (show_cursor)
				{
					std::vector<Vec2f> points;
					points.push_back(p + Vec2f(font_atlas->text_offset(font_size, str.c_str(), str.c_str() + select_end)));
					points[0].x() += 1.f;
					points.push_back(points[0] + Vec2f(0.f, font_size));
					canvas->stroke(points.size(), points.data(), c_text->color_.new_proply<3>(element->alpha_), 2.f);
				}
			}
		}
	};

	cEdit* cEdit::create()
	{
		return new cEditPrivate();
	}
}
