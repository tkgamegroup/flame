#include <flame/graphics/font.h>
#include <flame/graphics/canvas.h>
#include "../entity_private.h"
#include "text_private.h"
#include "event_receiver_private.h"
#include "edit_private.h"
#include "../systems/event_dispatcher_private.h"

namespace flame
{
	void cEditPrivate::mark_changed()
	{
		if (trigger_changed_on_lost_focus)
			changed = true;
		else
			text->mark_text_changed();
	}

	void cEditPrivate::flash_cursor(int mode)
	{
		if (element)
			element->mark_drawing_dirty();
		if (mode == 1)
			show_cursor = false;
		else if (mode == 2)
		{
			if (flash_event)
				looper().reset_event(flash_event);
			show_cursor = true;
		}
		else
			show_cursor = !show_cursor;
	}

	int cEditPrivate::locate_cursor(const Vec2f& mpos)
	{
		const auto& str = text->text;
		auto atlas = (graphics::FontAtlas*)text->canvas->get_resource(graphics::ResourceFontAtlas, text->res_id);
		auto font_size = text->size;

		element->update_transform();
		auto pos = element->points[4];
		auto p = Vec2f(0.f);
		auto axes = element->axes;

		auto i = 0;
		for (; i < str.size(); i++)
		{
			auto ch = str[i];
			if (ch == '\n')
			{
				p.y() += font_size;
				p.x() = 0.f;
			}
			else if (ch != '\r')
			{
				if (ch == '\t')
					ch = ' ';

				auto g = atlas->get_glyph(ch, font_size);
				auto adv = g->get_advance();

				Vec2f ps[] = { 
					pos + axes * p,
					pos + axes * (p + Vec2f(adv, 0.f)),
					pos + axes * (p + Vec2f(adv, font_size)),
					pos + axes * (p + Vec2f(0.f, font_size))
				};
				if (convex_contains<float>(mpos, ps))
					return i;

				p.x() += adv;
			}
		}
		return i;
	}

	void cEditPrivate::on_gain_event_receiver()
	{
		key_down_listener = event_receiver->add_key_down_listener([](Capture& c, KeyboardKey key) {
			auto thiz = c.thiz<cEditPrivate>();
			auto& str = thiz->text->text;
			auto& select_start = thiz->select_start;
			auto& select_end = thiz->select_end;
			auto low = min(select_start, select_end);
			auto high = max(select_start, select_end);
			auto ed = thiz->event_receiver->dispatcher;

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
				while (p < str.size())
				{
					if (str[p] == '\n')
						return p;
					p++;
				}
				return (int)str.size();
			};

			switch (key)
			{
			case Keyboard_Enter:
				//if (thiz->enter_to_throw_focus)
				//	throw_focus();
				break;
			case Keyboard_Left:
				if (ed->kbtns[Keyboard_Shift].first)
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
			case Keyboard_Right:
				if (ed->kbtns[Keyboard_Shift].first)
				{
					if (select_end < str.size())
						select_end++;
				}
				else
				{
					if (high < str.size())
						high++;
					select_start = select_end = high;
				}
				break;
			case Keyboard_Up:
			{
				if (ed->kbtns[Keyboard_Shift].first)
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
			case Keyboard_Down:
			{
				if (ed->kbtns[Keyboard_Shift].first)
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
			case Keyboard_Home:
				select_end = (ed->kbtns[Keyboard_Ctrl].first) ? 0 : line_start(select_end);
				if (!(ed->kbtns[Keyboard_Shift].first))
					select_start = select_end;
				break;
			case Keyboard_End:
				select_end = (ed->kbtns[Keyboard_Ctrl].first) ? str.size() : line_end(select_end);
				if (!(ed->kbtns[Keyboard_Shift].first))
					select_start = select_end;
				break;
			case Keyboard_Del:
				if (low == high)
				{
					if (low < str.size())
					{
						str.erase(str.begin() + low);
						thiz->mark_changed();
					}
				}
				else
				{
					str = str.substr(0, low) + str.substr(high);
					thiz->mark_changed();
					select_start = select_end = low;
				}
				break;
			}

			thiz->flash_cursor(2);
		}, Capture().set_thiz(this));
		char_listener = event_receiver->add_char_listener([](Capture& c, wchar_t ch) {
			auto thiz = c.thiz<cEditPrivate>();
			auto& str = thiz->text->text;
			auto& select_start = thiz->select_start;
			auto& select_end = thiz->select_end;
			auto low = min(select_start, select_end);
			auto high = max(select_start, select_end);

			//auto throw_focus = [&]() {
			//	auto dp = thiz->event_receiver->dispatcher;
			//	dp->next_focusing = dp->world_->root->get_component(cEventReceiver);
			//};

			switch (ch)
			{
			case 24: // Ctrl+X
			case 3: // Ctrl+C
				if (low == high)
					break;
				set_clipboard(str.substr(low, high - low).c_str());
				if (ch == 3)
					break;
			case L'\b':
				if (low == high)
				{
					if (low > 0)
					{
						low--;
						str.erase(str.begin() + low);
						thiz->mark_changed();
						select_start = select_end = low;
					}
				}
				else
				{
					str = str.substr(0, low) + str.substr(high);
					thiz->mark_changed();
					select_start = select_end = low;
				}
				break;
			case 22: // Ctrl+V
			{
				std::wstring cb;
				get_clipboard(&cb);
				cb.erase(std::remove(cb.begin(), cb.end(), '\r'), cb.end());
				if (!cb.empty())
				{
					str = str.substr(0, low) + cb + str.substr(high);
					thiz->mark_changed();
					select_start = select_end = high + cb.size() - (high - low);
				}
			}
				break;
			case 27:
				break;
			case '\r':
				break;
			case '\n':
				//if (thiz->enter_to_throw_focus)
				//{
				//	throw_focus();
				//	break;
				//}
				ch = '\n';
			default:
				str = str.substr(0, low) + std::wstring(1, ch) + str.substr(high);
				thiz->mark_changed();
				select_start = select_end = high + 1 - (high - low);
			}

			thiz->flash_cursor(2);
		}, Capture().set_thiz(this));

		mouse_down_listener = event_receiver->add_mouse_left_down_listener([](Capture& c, const Vec2i& pos) {
			auto thiz = c.thiz<cEditPrivate>();
			thiz->select_start = thiz->select_end = thiz->locate_cursor((Vec2f)pos);
			thiz->flash_cursor(2);
		}, Capture().set_thiz(this));
		mouse_move_listener = event_receiver->add_mouse_move_listener([](Capture& c, const Vec2i& disp, const Vec2i& pos) {
			auto thiz = c.thiz<cEditPrivate>();
			if (thiz->event_receiver->dispatcher->active == thiz->event_receiver)
			{
				thiz->select_end = thiz->locate_cursor((Vec2f)pos);
				thiz->flash_cursor(2);
			}
		}, Capture().set_thiz(this));
		mouse_dbclick_listener = event_receiver->add_mouse_dbclick_listener([](Capture& c) {
			auto thiz = c.thiz<cEditPrivate>();
			//	thiz->select_start = 0;
			//	thiz->select_end = thiz->text->text.size();
			//	if (thiz->element)
			//		thiz->element->mark_drawing_dirty();
		}, Capture().set_thiz(this));
	}

	void cEditPrivate::on_lost_event_receiver()
	{
		event_receiver->remove_key_down_listener(key_down_listener);
		event_receiver->remove_char_listener(char_listener);
		event_receiver->remove_mouse_left_down_listener(mouse_down_listener);
		event_receiver->remove_mouse_move_listener(mouse_move_listener);
		event_receiver->remove_mouse_dbclick_listener(mouse_dbclick_listener);
	}

	void cEditPrivate::on_local_message(Message msg, void* p)
	{
		switch (msg)
		{
		case MessageVisibilityChanged:
			if (flash_event && !entity->global_visibility)
			{
				looper().remove_event(flash_event);
				flash_event = nullptr;
			}
			break;
		case MessageStateChanged:
		{
			auto s = entity->state;
			event_receiver->dispatcher->window->set_cursor((s & StateHovering) != 0 ? CursorIBeam : CursorArrow);
			if ((s & StateFocusing) != 0)
			{
				if (!flash_event)
				{
					flash_event = looper().add_event([](Capture& c) {
						c.thiz<cEditPrivate>()->flash_cursor(0);
						c._current = nullptr;
					}, Capture().set_thiz(this), 0.5f);
				}
				//if (select_all_on_focus)
				//	set_select(0, text->text.s);
			}
			else
			{
				if (flash_event)
				{
					looper().remove_event(flash_event);
					flash_event = nullptr;
				}
				select_start = select_end = 0;
				flash_cursor(1);
				//if (thiz->trigger_changed_on_lost_focus && thiz->changed)
				//{
				//	thiz->text->set_text(nullptr, -1, thiz);
				//	thiz->changed = false;
				//}
			}
		}
			break;
		case MessageLeftWorld:
			if (flash_event)
			{
				looper().remove_event(flash_event);
				flash_event = nullptr;
			}
			break;
		}
	}

	void cEditPrivate::draw(graphics::Canvas* canvas)
	{
		const auto& str = text->text;
		auto atlas = (graphics::FontAtlas*)text->canvas->get_resource(graphics::ResourceFontAtlas, text->res_id);
		auto font_size = text->size;
		element->update_transform();
		auto pos = element->points[4];
		auto axes = element->axes;

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

				auto sb = str.c_str() + left, se = str.c_str() + right;
				auto p1 = Vec2f(atlas->text_offset(font_size, str.c_str(), sb));
				auto p2 = Vec2f(atlas->text_offset(font_size, str.c_str(), se));
				if (right < high && str[right] == '\n')
					p2.x() += 4.f;
				canvas->begin_path();
				canvas->move_to(pos + axes * p1);
				canvas->line_to(pos + axes * p2);
				canvas->line_to(pos + axes * (p2 + Vec2f(0.f, font_size)));
				canvas->line_to(pos + axes * (p1 + Vec2f(0.f, font_size)));
				canvas->fill(Vec4c(128, 128, 255, 255));
				canvas->draw_text(0, sb, se, font_size, Vec4c(255), pos + p1, axes);
			}
		}

		if (show_cursor)
		{
			auto off = (Vec2f)atlas->text_offset(font_size, str.c_str(), str.c_str() + select_end);
			off.x() += 0.5f;
			canvas->begin_path();
			canvas->move_to(pos + axes * off);
			canvas->line_to(pos + axes * (off + Vec2f(0.f, font_size)));
			canvas->stroke(Vec4c(0, 0, 0, 255), 1.f);
		}
	}

	cEdit* cEdit::create()
	{
		return f_new<cEditPrivate>();
	}
}
