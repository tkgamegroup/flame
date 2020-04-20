#include <flame/graphics/font.h>
#include <flame/graphics/canvas.h>
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

namespace flame
{
	struct cEditPrivate : cEdit
	{
		void* key_listener;
		void* mouse_listener;
		void* focus_listener;
		void* state_listener;
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

			select_all_on_dbclicked = true;
			select_all_on_focus = true;

			key_listener = nullptr;
			mouse_listener = nullptr;
			focus_listener = nullptr;
			state_listener = nullptr;
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
				event_receiver->focus_listeners.remove(focus_listener);
				event_receiver->state_listeners.remove(state_listener);
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
			const auto str = c_text->text.v;
			auto len = c_text->text.s;
			auto font_atlas = c_text->font_atlas;
			auto font_size = c_text->font_size * element->global_scale;
			auto p = element->content_min();

			auto y = p.y();
			auto i = 0;
			for (; i < len; )
			{
				auto next = i;
				while (true)
				{
					if (str[next] == '\n')
					{
						next++;
						break;
					}
					next++;
					if (next == len)
					{
						next = -1;
						break;
					}
				}

				if (pos.y() < y + font_size || next == -1)
				{
					auto x = p.x();
					while (true)
					{
						auto ch = str[i];
						if (ch == '\n')
							break;
						if (ch == '\t')
							ch = ' ';
						auto w = font_atlas->get_glyph(ch, font_size)->advance;
						if (pos.x() < x + w / 2)
							break;
						x += w;
						i++;
						if (i == len)
							break;
					}
					break;
				}

				i = next;
				y += font_size;
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
					auto c_text = (cTextPrivate*)thiz->text;
					auto& text = c_text->text;
					const auto str = text.v;
					auto len = text.s;
					auto& select_start = thiz->select_start;
					auto& select_end = thiz->select_end;
					auto low = min(select_start, select_end);
					auto high = max(select_start, select_end);
					auto ed = cEventReceiver::current()->dispatcher;

					auto line_start = [&](int p) {
						p--;
						do
						{
							if (p < 0)
								return 0;
							if (str[p] == '\n')
								return p + 1;
							p--;
						} while (p >= 0);
						return 0;
					};
					auto line_end = [&](int p) {
						while (p < len)
						{
							if (str[p] == '\n')
								return p;
							p++;
						}
						return (int)len;
					};

					if (action == KeyStateNull)
					{
						switch (value)
						{
						case L'\b':
							if (low == high)
							{
								if (low > 0)
								{
									low--;
									auto temp = text.str();
									temp.erase(temp.begin() + low);
									text = temp;
									c_text->set_text(nullptr, -1, thiz);
									select_start = select_end = low;
								}
							}
							else
							{
								auto temp = text.str();
								temp = temp.substr(0, low) + temp.substr(high);
								text = temp;
								c_text->set_text(nullptr, -1, thiz);
								select_start = select_end = low;
							}
							break;
						case 3: // Ctrl+C
							if (low != high)
								set_clipboard(text.str().substr(low, high).c_str());
							break;
						case 22: // Ctrl+V
						{
							auto cb = get_clipboard().str();
							cb.erase(std::remove(cb.begin(), cb.end(), '\r'), cb.end());
							if (!cb.empty())
							{
								auto temp = text.str();
								temp = temp.substr(0, low) + cb + temp.substr(high);
								text = temp;
								c_text->set_text(nullptr, -1, thiz);
								select_start = select_end = high + cb.size() - (high - low);
							}
						}
							break;
						case 27:
							break;
						case 13:
							value = '\n';
						default:
							auto temp = text.str();
							temp = temp.substr(0, low) + std::wstring(1, value) + temp.substr(high);
							text = temp;
							c_text->set_text(nullptr, -1, thiz);
							select_start = select_end = high + 1 - (high - low);
						}
					}
					else if (action == KeyStateDown)
					{
						switch (value)
						{
						case Key_Left:
							if (ed->key_states[Key_Shift] & KeyStateDown)
							{
								if (select_end > 0)
									select_end--;
							}
							else
							{
								if (low > 0)
									low--;
								select_start = select_end = low;
							}
							break;
						case Key_Right:
							if (ed->key_states[Key_Shift] & KeyStateDown)
							{
								if (select_end < len)
									select_end++;
							}
							else
							{
								if (high < len)
									high++;
								select_start = select_end = high;
							}
							break;
						case Key_Up:
						{
							if (ed->key_states[Key_Shift] & KeyStateDown)
							{
								auto end_s = line_start(select_end);
								auto up_s = line_start(end_s - 1);
								if (end_s != up_s)
									select_end = up_s + min((int)select_end - end_s, max(0, end_s - up_s - 1));
							}
							else
							{
								auto end_s = line_start(select_end);
								auto low_s = low == select_end ? end_s : line_start(low);
								auto up_s = line_start(low_s - 1);
								if (low_s != up_s)
									select_end = select_start = up_s + min((int)select_end - end_s, max(0, low_s - up_s - 1));
								else
									select_start = select_end;
							}
						}
							break;
						case Key_Down:
						{
							if (ed->key_states[Key_Shift] & KeyStateDown)
							{
								auto end_s = line_start(select_end);
								auto end_e = line_end(select_end);
								auto down_l = line_end(end_e + 1) - end_e - 1;
								if (down_l >= 0)
									select_end = end_e + 1 + min(down_l, (int)select_end - end_s);
							}
							else
							{
								auto end_s = line_start(select_end);
								auto high_e = line_end(high);
								auto down_l = line_end(high_e + 1) - high_e - 1;
								if (down_l >= 0)
									select_end = select_start = high_e + 1 + min(down_l, (int)select_end - end_s);
								else
									select_start = select_end;
							}
						}
							break;
						case Key_Home:
							select_end = (ed->key_states[Key_Ctrl] & KeyStateDown) ? 0 : line_start(select_end);
							if (!(ed->key_states[Key_Shift] & KeyStateDown))
								select_start = select_end;
							break;
						case Key_End:
							select_end = (ed->key_states[Key_Ctrl] & KeyStateDown) ? len : line_end(select_end);
							if (!(ed->key_states[Key_Shift] & KeyStateDown))
								select_start = select_end;
							break;
						case Key_Del:
							if (low == high)
							{
								if (low < len)
								{
									auto temp = text.str();
									temp.erase(temp.begin() + low);
									text = temp;
									c_text->set_text(nullptr, -1, thiz);
								}
							}
							else
							{
								auto temp = text.str();
								temp = temp.substr(0, low) + temp.substr(high);
								text = temp;
								c_text->set_text(nullptr, -1, thiz);
								select_start = select_end = low;
							}
							break;
						}
					}

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
						thiz->select_end = thiz->locate_cursor(cEventReceiver::current()->dispatcher->mouse_pos);
						thiz->flash_cursor(2);
					}
					else if (is_mouse_clicked(action, key) && (action & KeyStateDouble) && thiz->select_all_on_dbclicked)
					{
						thiz->set_select(0, thiz->text->text.s);
						thiz->element->mark_dirty();
					}
					return true;
				}, Mail::from_p(this));

				focus_listener = event_receiver->focus_listeners.add([](void* c, bool focusing) {
					auto thiz = *(cEditPrivate**)c;
					if (focusing)
					{
						thiz->timer->start();
						if (thiz->select_all_on_focus)
							thiz->set_select(0, thiz->text->text.s);
					}
					else
					{
						thiz->timer->stop();
						thiz->set_select(0);
						thiz->flash_cursor(1);
					}
					return true;
				}, Mail::from_p(this));

				state_listener = event_receiver->state_listeners.add([](void*, EventReceiverState s) {
					cEventReceiver::current()->dispatcher->window->set_cursor(s ? CursorIBeam : CursorArrow);
					return true;
				}, Mail());
			}
		}

		void draw(graphics::Canvas* canvas)
		{
			if (!element->clipped)
			{
				auto c_text = (cTextPrivate*)text;
				const auto str = c_text->text.v;
				auto font_atlas = c_text->font_atlas;
				auto font_size = c_text->font_size * element->global_scale;
				auto p = element->content_min();

				if (select_start != select_end)
				{
					auto low = min(select_start, select_end);
					auto high = max(select_start, select_end);
					while (low < high)
					{
						auto left = low;
						auto right = left;
						while (right < high)
						{
							if (str[right] == '\n')
								break;
							right++;
						}
						low = right + 1;
						
						auto sb = str + left, se = str + right;
						std::vector<Vec2f> points;
						auto pos1 = Vec2f(font_atlas->text_offset(font_size, str, sb));
						auto pos2 = Vec2f(font_atlas->text_offset(font_size, str, se));
						if (right < high && str[right] == '\n')
							pos2.x() += 4.f;
						path_rect(points, pos1 + p, Vec2f(pos2.x() - pos1.x(), font_size));
						canvas->fill(points.size(), points.data(), Vec4c(128, 128, 255, 255));
						canvas->add_text(font_atlas, sb, se, font_size, pos1 + p, Vec4c(255));
					}
				}

				if (show_cursor)
				{
					std::vector<Vec2f> points;
					points.push_back(p + Vec2f(font_atlas->text_offset(font_size, str, str + select_end)));
					points[0].x() += 1.f;
					points.push_back(points[0] + Vec2f(0.f, font_size));
					canvas->stroke(points.size(), points.data(), c_text->color.copy().factor_w(element->alpha), 2.f);
				}
			}
		}
	};

	cEdit* cEdit::create()
	{
		return new cEditPrivate();
	}
}
