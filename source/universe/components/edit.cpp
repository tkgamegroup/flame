#include <flame/graphics/font.h>
#include <flame/universe/components/element.h>
#include <flame/universe/components/text.h>
#include <flame/universe/components/event_receiver.h>
#include <flame/universe/components/aligner.h>
#include <flame/universe/components/edit.h>
#include <flame/universe/ui/style_stack.h>

#include "../renderpath/canvas/canvas.h"

namespace flame
{
	struct cEditPrivate : cEdit
	{
		bool text_changed;
		uint last_font_size;
		uint last_cursor;
		graphics::Glyph* cursor_glyph;
		Vec2f cursor_pos;

		void* text_changed_listener;
		void* key_listener;
		void* mouse_listener;
		void* draw_cmd;

		cEditPrivate()
		{
			element = nullptr;
			text = nullptr;
			event_receiver = nullptr;

			cursor = 0;

			text_changed = true;
			last_font_size = 0;
			last_cursor = 0;
			cursor_glyph = nullptr;

			text_changed_listener = nullptr;
			key_listener = nullptr;
			mouse_listener = nullptr;
			draw_cmd = nullptr;
		}

		~cEditPrivate()
		{
			if (!entity->dying_)
			{
				element->cmds.remove(draw_cmd);
				text->data_changed_listeners.remove(text_changed_listener);
				event_receiver->key_listeners.remove(key_listener);
				event_receiver->mouse_listeners.remove(mouse_listener);
			}
		}

		void on_component_added(Component* c) override
		{
			if (c->name_hash == FLAME_CHASH("cElement"))
			{
				element = (cElement*)c;
				draw_cmd = element->cmds.add([](void* c, graphics::Canvas* canvas) {
					(*(cEditPrivate**)c)->draw(canvas);
				}, new_mail_p(this));
			}
			else if (c->name_hash == FLAME_CHASH("cText"))
			{
				text = (cText*)c;
				text_changed_listener = text->data_changed_listeners.add([](void* c, Component*, uint hash, void* sender) {
					auto thiz = *(cEditPrivate**)c;
					switch (hash)
					{
					case FLAME_CHASH("text"):
						thiz->text_changed = true;
						break;
					}
				}, new_mail_p(this));
			}
			else if (c->name_hash == FLAME_CHASH("cEventReceiver"))
			{
				event_receiver = (cEventReceiver*)c;
				event_receiver->accept_key = true;
				key_listener = event_receiver->key_listeners.add([](void* c, KeyState action, int value) {
					auto thiz = *(cEditPrivate**)c;
					auto text = thiz->text;
					auto lenght = text->text_length();

					if (action == KeyStateNull)
					{
						switch (value)
						{
						case L'\b':
							if (thiz->cursor > 0)
							{
								thiz->cursor--;
								text->erase_char(thiz->cursor);
							}
							break;
						case 22:
						{
							thiz->cursor = 0;
							text->set_text(get_clipboard().v);
						}
							break;
						case 27:
							break;
						case 13:
							value = '\n';
						default:
							text->insert_char(value, thiz->cursor);
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
							if (thiz->cursor < lenght)
								thiz->cursor++;
							break;
						case Key_Home:
							thiz->cursor = 0;
							break;
						case Key_End:
							thiz->cursor = lenght;
							break;
						case Key_Del:
							if (thiz->cursor < lenght)
								text->erase_char(thiz->cursor);
							break;
						}
					}
				}, new_mail_p(this));

				mouse_listener = event_receiver->mouse_listeners.add([](void* c, KeyState action, MouseKey key, const Vec2i& pos) {
					auto thiz = *(cEditPrivate**)c;

					if (is_mouse_down(action, key, true) && key == Mouse_Left)
					{
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
				}, new_mail_p(this));
			}
		}

		void draw(graphics::Canvas* canvas)
		{
			if (!element->cliped && event_receiver->focusing && (int(looper().total_time * 2.f) % 2 == 0))
			{
				auto global_scale = element->global_scale;
				auto font_atlas = text->font_atlas;
				auto font_size = text->last_font_size;
				if (text_changed || font_size != last_font_size || cursor != last_cursor)
				{
					text_changed = false;
					last_font_size = font_size;
					last_cursor = cursor;
					cursor_glyph = font_atlas->get_glyph(L'|', last_font_size);
					cursor_pos = Vec2f(font_atlas->get_text_offset(text->text(), cursor, last_font_size));
				}
				auto scale = text->last_scale;
				canvas->add_text(font_atlas, 1, &cursor_glyph, last_font_size, scale, element->global_pos +
					Vec2f(element->inner_padding_[0], element->inner_padding_[1]) * global_scale + cursor_pos * scale,
					text->color.new_proply<3>(element->alpha_));
			}
		}
	};

	cEdit* cEdit::create()
	{
		return new cEditPrivate();
	}
}
