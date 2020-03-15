#include <flame/graphics/font.h>
#include <flame/universe/systems/2d_renderer.h>
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
		void* cusor_flash_event;
		bool show_cursor;

		cEditPrivate()
		{
			element = nullptr;
			text = nullptr;
			event_receiver = nullptr;

			cursor = 0;

			key_listener = nullptr;
			mouse_listener = nullptr;
			focus_listener = nullptr;
			draw_cmd = nullptr;
			cusor_flash_event = nullptr;
			show_cursor = false;
		}

		~cEditPrivate()
		{
			if (!entity->dying_)
			{
				element->cmds.remove(draw_cmd);
				event_receiver->key_listeners.remove(key_listener);
				event_receiver->mouse_listeners.remove(mouse_listener);
				event_receiver->mouse_listeners.remove(focus_listener);
			}
			if (cusor_flash_event)
				looper().remove_event(cusor_flash_event);
		}

		void flash_cursor(bool force = false)
		{
			auto render = element->renderer;
			if (render)
				render->pending_update = true;
			if (force)
				show_cursor = false;
			else
				show_cursor = !show_cursor;
		}

		void on_component_added(Component* c) override
		{
			if (c->name_hash == FLAME_CHASH("cElement"))
			{
				element = (cElement*)c;
				draw_cmd = element->cmds.add([](void* c, graphics::Canvas* canvas) {
					(*(cEditPrivate**)c)->draw(canvas);
					return true;
				}, new_mail_p(this));
			}
			else if (c->name_hash == FLAME_CHASH("cText"))
				text = (cText*)c;
			else if (c->name_hash == FLAME_CHASH("cEventReceiver"))
			{
				event_receiver = (cEventReceiver*)c;
				key_listener = event_receiver->key_listeners.add([](void* c, KeyStateFlags action, int value) {
					auto thiz = *(cEditPrivate**)c;
					auto text = (cTextPrivate*)thiz->text;
					auto& str = text->text;

					if (action == KeyStateNull)
					{
						switch (value)
						{
						case L'\b':
							if (thiz->cursor > 0)
							{
								thiz->cursor--;
								str.erase(str.begin() + thiz->cursor);
								text->data_changed(FLAME_CHASH("text"), thiz);
							}
							break;
						case 22:
						{
							thiz->cursor = 0;
							str = get_clipboard().v;
							text->data_changed(FLAME_CHASH("text"), thiz);
						}
							break;
						case 27:
							break;
						case 13:
							value = '\n';
						default:
							str.insert(str.begin() + thiz->cursor, value);
							text->data_changed(FLAME_CHASH("text"), thiz);
							thiz->cursor++;
						}
					}
					else if (action == KeyStateDown)
					{
						switch (value)
						{
						case Key_Left:
							if (thiz->cursor > 0)
								thiz->cursor--;
							break;
						case Key_Right:
							if (thiz->cursor < str.size())
								thiz->cursor++;
							break;
						case Key_Home:
							thiz->cursor = 0;
							break;
						case Key_End:
							thiz->cursor = str.size();
							break;
						case Key_Del:
							if (thiz->cursor < str.size())
							{
								str.erase(str.begin() + thiz->cursor);
								text->data_changed(FLAME_CHASH("text"), thiz);
							}
							break;
						}
					}

					return true;
				}, new_mail_p(this));

				mouse_listener = event_receiver->mouse_listeners.add([](void* c, KeyStateFlags action, MouseKey key, const Vec2i& pos) {
					if (is_mouse_down(action, key, true) && key == Mouse_Left)
					{
						auto thiz = *(cEditPrivate**)c;
						auto element = thiz->element;
						auto text = thiz->text;
						auto atlas = text->font_atlas;
						auto str = text->text();
						auto scale = element->global_scale;
						if (atlas->draw_type == graphics::FontDrawSdf)
							scale *= text->scale_;
						auto font_size = text->font_size_;
						auto line_space = font_size * scale;
						auto y = element->global_pos.y();
						for (auto p = str; ; p++)
						{
							if (y < pos.y() && pos.y() < y + line_space)
							{
								auto x = element->global_pos.x();
								for (;; p++)
								{
									if (!*p)
										break;
									if (*p == '\n' || *p == '\r')
										break;
									auto w = atlas->get_glyph(*p == '\t' ? ' ' : *p, font_size)->advance * scale;
									if (x <= pos.x() && pos.x() < x + w)
										break;
									x += w;
								}
								thiz->cursor = p - str;
								break;
							}

							if (!*p)
								break;
							if (*p == '\n')
								y += line_space;
						}
					}
					return true;
				}, new_mail_p(this));

				focus_listener = event_receiver->focus_listeners.add([](void* c, bool focusing) {
					auto thiz = *(cEditPrivate**)c;
					if (focusing)
					{
						if (!thiz->cusor_flash_event)
						{
							thiz->cusor_flash_event = looper().add_event([](void* c, bool* go_on) {
								(*(cEditPrivate**)c)->flash_cursor();
								*go_on = true;
							}, new_mail_p(thiz), 0.5f);
						}
					}
					else
					{
						if (thiz->cusor_flash_event)
						{
							looper().remove_event(thiz->cusor_flash_event);
							thiz->cusor_flash_event = nullptr;
						}
						thiz->flash_cursor(true);
					}
					return true;
				}, new_mail_p(this));
			}
		}

		void draw(graphics::Canvas* canvas)
		{
			if (show_cursor && !element->cliped)
			{
				auto font_atlas = text->font_atlas;

				if (font_atlas->draw_type == graphics::FontDrawSdf)
				{
					canvas->add_text(font_atlas, L"|", 0, text->scale_ * element->global_scale, element->global_pos +
						Vec2f(element->inner_padding_[0], element->inner_padding_[1]) * element->global_scale +
						Vec2f(font_atlas->text_offset(0, text->text(), text->text() + cursor)) * text->scale_ * element->global_scale,
						text->color.new_proply<3>(element->alpha_));
				}
				else
				{
					canvas->add_text(font_atlas, L"|", text->font_size_ * element->global_scale, 1.f, element->global_pos +
						Vec2f(element->inner_padding_[0], element->inner_padding_[1]) * element->global_scale +
						Vec2f(font_atlas->text_offset(text->font_size_ * element->global_scale, text->text(), text->text() + cursor)),
						text->color.new_proply<3>(element->alpha_));
				}
			}
		}
	};

	cEdit* cEdit::create()
	{
		return new cEditPrivate();
	}
}
