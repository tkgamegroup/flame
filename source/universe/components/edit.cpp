//#include <flame/universe/world.h>
//#include <flame/universe/systems/event_dispatcher.h>
//#include <flame/universe/systems/2d_renderer.h>
//#include <flame/universe/components/element.h>
//#include <flame/universe/components/aligner.h>

#include <flame/graphics/font.h>
#include <flame/graphics/canvas.h>
#include "../entity_private.h"
#include "text_private.h"
#include "event_receiver_private.h"
#include "edit_private.h"

namespace flame
{
	void cEditPrivate::flash_cursor(int mode)
	{
		element->mark_drawing_dirty();
		if (mode == 1)
			show_cursor = false;
		else if (mode == 2)
		{
			if (flash_event)
				get_looper()->reset_event(flash_event);
			show_cursor = true;
		}
		else
			show_cursor = !show_cursor;
	}

	int cEditPrivate::locate_cursor(const Vec2f& mpos)
	{
		const auto& str = text->text;
		auto atlas = text->atlas;
		auto font_size = text->font_size;

		element->update_transform();
		auto pos = element->points[4];
		auto p = Vec2f(0.f);
		auto axisx = Vec2f(element->transform[0]);
		auto axisy = Vec2f(element->transform[1]);

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
					pos + axisx * p.x() + p.y() * axisy,
					pos + axisx * (p.x() + adv) + p.y() * axisy,
					pos + axisx * (p.x() + adv) + (p.y() + font_size) * axisy,
					pos + axisx * p.x() + (p.y() + font_size) * axisy
				};
				if (convex_contains<float>(mpos, ps))
					return i;

				p.x() += adv;
			}
		}
		return i;
	}

	void cEditPrivate::on_added()
	{
		element = (cElementPrivate*)((EntityPrivate*)entity)->get_component(cElement::type_hash);
		text = (cTextPrivate*)((EntityPrivate*)entity)->get_component(cText::type_hash);
		event_receiver = (cEventReceiverPrivate*)((EntityPrivate*)entity)->get_component(cEventReceiver::type_hash);

		element->drawers.push_back(this);

		//					key_listener = event_receiver->key_listeners.add([](Capture& c, KeyStateFlags action, int value) {
		//						auto thiz = c.thiz<cEditPrivate>();
		//						auto c_text = (cTextPrivate*)thiz->text;
		//						auto& text = c_text->text;
		//						const auto str = text.v;
		//						auto len = text.s;
		//						auto& select_start = thiz->select_start;
		//						auto& select_end = thiz->select_end;
		//						auto low = min(select_start, select_end);
		//						auto high = max(select_start, select_end);
		//						auto ed = c.current<cEventReceiver>()->dispatcher;
		//
		//						auto line_start = [&](int p) {
		//							p--;
		//							do
		//							{
		//								if (p < 0)
		//									return 0;
		//								if (str[p] == '\n')
		//									return p + 1;
		//								p--;
		//							} while (p >= 0);
		//							return 0;
		//						};
		//						auto line_end = [&](int p) {
		//							while (p < len)
		//							{
		//								if (str[p] == '\n')
		//									return p;
		//								p++;
		//							}
		//							return (int)len;
		//						};
		//						auto on_changed = [&]() {
		//							if (thiz->trigger_changed_on_lost_focus)
		//								thiz->changed = true;
		//							else
		//								c_text->set_text(nullptr, -1, thiz);
		//						};
		//						auto throw_focus = [&]() {
		//							auto dp = thiz->event_receiver->dispatcher;
		//							dp->next_focusing = dp->world_->root->get_component(cEventReceiver);
		//						};
		//
		//						if (action == KeyStateNull)
		//						{
		//							switch (value)
		//							{
		//							case 24: // Ctrl+X
		//							case 3: // Ctrl+C
		//								if (low == high)
		//									break;
		//								set_clipboard(text.str().substr(low, high - low).c_str());
		//								if (value == 3)
		//									break;
		//							case L'\b':
		//								if (low == high)
		//								{
		//									if (low > 0)
		//									{
		//										low--;
		//										auto temp = text.str();
		//										temp.erase(temp.begin() + low);
		//										text = temp;
		//										on_changed();
		//										select_start = select_end = low;
		//									}
		//								}
		//								else
		//								{
		//									auto temp = text.str();
		//									temp = temp.substr(0, low) + temp.substr(high);
		//									text = temp;
		//									on_changed();
		//									select_start = select_end = low;
		//								}
		//								break;
		//							case 22: // Ctrl+V
		//							{
		//								auto cb = get_clipboard().str();
		//								cb.erase(std::remove(cb.begin(), cb.end(), '\r'), cb.end());
		//								if (!cb.empty())
		//								{
		//									auto temp = text.str();
		//									temp = temp.substr(0, low) + cb + temp.substr(high);
		//									text = temp;
		//									on_changed();
		//									select_start = select_end = high + cb.size() - (high - low);
		//								}
		//							}
		//							break;
		//							case 27:
		//								break;
		//							case '\r':
		//								break;
		//							case '\n':
		//								if (thiz->enter_to_throw_focus)
		//								{
		//									throw_focus();
		//									break;
		//								}
		//								value = '\n';
		//							default:
		//								auto temp = text.str();
		//								temp = temp.substr(0, low) + std::wstring(1, value) + temp.substr(high);
		//								text = temp;
		//								on_changed();
		//								select_start = select_end = high + 1 - (high - low);
		//							}
		//						}
		//						else if (action == KeyStateDown)
		//						{
		//							switch (value)
		//							{
		//							case Key_Enter:
		//								if (thiz->enter_to_throw_focus)
		//									throw_focus();
		//								break;
		//							case Key_Left:
		//								if (ed->key_states[Key_Shift] & KeyStateDown)
		//								{
		//									if (select_end > 0)
		//										select_end--;
		//								}
		//								else
		//								{
		//									if (low > 0)
		//										low--;
		//									select_start = select_end = low;
		//								}
		//								break;
		//							case Key_Right:
		//								if (ed->key_states[Key_Shift] & KeyStateDown)
		//								{
		//									if (select_end < len)
		//										select_end++;
		//								}
		//								else
		//								{
		//									if (high < len)
		//										high++;
		//									select_start = select_end = high;
		//								}
		//								break;
		//							case Key_Up:
		//							{
		//								if (ed->key_states[Key_Shift] & KeyStateDown)
		//								{
		//									auto end_s = line_start(select_end);
		//									auto up_s = line_start(end_s - 1);
		//									if (end_s != up_s)
		//										select_end = up_s + min((int)select_end - end_s, max(0, end_s - up_s - 1));
		//								}
		//								else
		//								{
		//									auto end_s = line_start(select_end);
		//									auto low_s = low == select_end ? end_s : line_start(low);
		//									auto up_s = line_start(low_s - 1);
		//									if (low_s != up_s)
		//										select_end = select_start = up_s + min((int)select_end - end_s, max(0, low_s - up_s - 1));
		//									else
		//										select_start = select_end;
		//								}
		//							}
		//							break;
		//							case Key_Down:
		//							{
		//								if (ed->key_states[Key_Shift] & KeyStateDown)
		//								{
		//									auto end_s = line_start(select_end);
		//									auto end_e = line_end(select_end);
		//									auto down_l = line_end(end_e + 1) - end_e - 1;
		//									if (down_l >= 0)
		//										select_end = end_e + 1 + min(down_l, (int)select_end - end_s);
		//								}
		//								else
		//								{
		//									auto end_s = line_start(select_end);
		//									auto high_e = line_end(high);
		//									auto down_l = line_end(high_e + 1) - high_e - 1;
		//									if (down_l >= 0)
		//										select_end = select_start = high_e + 1 + min(down_l, (int)select_end - end_s);
		//									else
		//										select_start = select_end;
		//								}
		//							}
		//							break;
		//							case Key_Home:
		//								select_end = (ed->key_states[Key_Ctrl] & KeyStateDown) ? 0 : line_start(select_end);
		//								if (!(ed->key_states[Key_Shift] & KeyStateDown))
		//									select_start = select_end;
		//								break;
		//							case Key_End:
		//								select_end = (ed->key_states[Key_Ctrl] & KeyStateDown) ? len : line_end(select_end);
		//								if (!(ed->key_states[Key_Shift] & KeyStateDown))
		//									select_start = select_end;
		//								break;
		//							case Key_Del:
		//								if (low == high)
		//								{
		//									if (low < len)
		//									{
		//										auto temp = text.str();
		//										temp.erase(temp.begin() + low);
		//										text = temp;
		//										on_changed();
		//									}
		//								}
		//								else
		//								{
		//									auto temp = text.str();
		//									temp = temp.substr(0, low) + temp.substr(high);
		//									text = temp;
		//									on_changed();
		//									select_start = select_end = low;
		//								}
		//								break;
		//							}
		//						}
		//
		//						thiz->flash_cursor(2);
		//
		//						return true;
		//					}, Capture().set_thiz(this));
		
		mouse_listener = event_receiver->add_mouse_listener([](Capture& c, KeyStateFlags action, MouseKey key, const Vec2i& pos) {
			auto thiz = c.thiz<cEditPrivate>();
			if (action == (KeyStateDown | KeyStateJust) && key == Mouse_Left)
			{
				thiz->select_start = thiz->select_end = thiz->locate_cursor((Vec2f)pos);
				thiz->flash_cursor(2);
			}
			//else if (is_mouse_move(action, key) && thiz->event_receiver->is_active())
			//{
			//	thiz->select_end = thiz->locate_cursor(c.current<cEventReceiver>()->dispatcher->mouse_pos);
			//	thiz->flash_cursor(2);
			//}
			//else if (is_mouse_clicked(action, key) && (action & KeyStateDouble) && thiz->select_all_on_dbclicked)
			//{
			//	thiz->set_select(0, thiz->text->text.s);
			//	thiz->element->mark_dirty();
			//}
			return true;
		}, Capture().set_thiz(this));

		//					state_listener = event_receiver->state_listeners.add([](Capture& c, EventReceiverState s) {
		//						c.current<cEventReceiver>()->dispatcher->window->set_cursor(s ? CursorIBeam : CursorArrow);
		//						return true;
		//					}, Capture());
	}

	void cEditPrivate::on_removed()
	{
		std::erase_if(element->drawers, [&](const auto& i) {
			return i == this;
		});

		//event_receiver->key_listeners.remove(key_listener);
		event_receiver->remove_mouse_listener(mouse_listener);
		//event_receiver->focus_listeners.remove(focus_listener);

		if (flash_event)
			get_looper()->remove_event(flash_event);
	}

	void cEditPrivate::on_left_world()
	{
		if (flash_event)
		{
			get_looper()->remove_event(flash_event);
			flash_event = nullptr;
		}
	}

	void cEditPrivate::on_entity_visibility_changed()
	{
		if (flash_event && !((EntityPrivate*)entity)->global_visibility)
		{
			get_looper()->remove_event(flash_event);
			flash_event = nullptr;
		}
	}

	void cEditPrivate::on_entity_state_changed()
	{
		auto s = ((EntityPrivate*)entity)->state;
		if ((s & StateFocusing) != 0)
		{
			if (!flash_event)
			{
				flash_event = get_looper()->add_event([](Capture& c) {
					c.thiz<cEditPrivate>()->flash_cursor(0);
					c._current = INVALID_POINTER;
				}, Capture().set_thiz(this), 0.5f);
			}
			//if (select_all_on_focus)
			//	set_select(0, text->text.s);
		}
		else
		{
			if (flash_event)
			{
				get_looper()->remove_event(flash_event);
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

	void cEditPrivate::draw(graphics::Canvas* canvas)
	{
		const auto& str = text->text;
		auto atlas = text->atlas;
		auto font_size = text->font_size;
		element->update_transform();
		auto pos = element->points[4];
		auto axisx = Vec2f(element->transform[0]);
		auto axisy = Vec2f(element->transform[1]);

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
				std::vector<Vec2f> points;
				auto pos1 = Vec2f(atlas->text_offset(font_size, str.c_str(), sb));
				auto pos2 = Vec2f(atlas->text_offset(font_size, str.c_str(), se));
				if (right < high && str[right] == '\n')
					pos2.x() += 4.f;
				//path_rect(points, pos1 + p, Vec2f(pos2.x() - pos1.x(), font_size));
				//canvas->fill(points.size(), points.data(), Vec4c(128, 128, 255, 255));
				//canvas->add_text(0, sb, se, font_size, pos1 + p, Vec4c(255));
			}
		}

		if (show_cursor)
		{
			auto off = (Vec2f)atlas->text_offset(font_size, str.c_str(), str.c_str() + select_end);
			canvas->begin_path();
			auto p0 = pos + off.x() * axisx + off.y() * axisy;
			canvas->move_to(p0.x(), p0.y());
			auto p1 = pos + (off.x() + 2.f) * axisx + off.y() * axisy;
			canvas->line_to(p1.x(), p1.y());
			auto p2 = pos + (off.x() + 2.f) * axisx + (off.y() + font_size) * axisy;
			canvas->line_to(p2.x(), p2.y());
			auto p3 = pos + off.x() * axisx + (off.y() + font_size) * axisy;
			canvas->line_to(p3.x(), p3.y());
			canvas->fill(Vec4c(0, 0, 0, 255));
		}
	}

//	struct cEditPrivate : cEdit
//	{
//		cEditPrivate()
//		{
//			select_all_on_dbclicked = true;
//			select_all_on_focus = true;
//			enter_to_throw_focus = false;
//			trigger_changed_on_lost_focus = false;
//		}
//	};

	cEdit* cEdit::create()
	{
		return f_new<cEditPrivate>();
	}
}
