#include <flame/graphics/font.h>
#include <flame/universe/default_style.h>
#include <flame/universe/components/element.h>
#include <flame/universe/components/text.h>
#include <flame/universe/components/event_receiver.h>
#include <flame/universe/components/aligner.h>
#include <flame/universe/components/edit.h>

#include "../renderpath/canvas/canvas.h"

namespace flame
{
	struct cEditPrivate : cEdit
	{
		uint last_font_size;
		graphics::Glyph* cursor_glyph;
		void* key_listener;
		void* mouse_listener;
		void* draw_cmd;

		cEditPrivate()
		{
			element = nullptr;
			text = nullptr;
			event_receiver = nullptr;

			cursor = 0;

			last_font_size = 0;
			cursor_glyph = nullptr;

			key_listener = nullptr;
			mouse_listener = nullptr;
			draw_cmd = nullptr;
		}

		~cEditPrivate()
		{
			if (!entity->dying_)
			{
				element->cmds.remove(draw_cmd);
				event_receiver->key_listeners.remove(key_listener);
				event_receiver->mouse_listeners.remove(mouse_listener);
			}
		}

		void on_component_added(Component* c) override
		{
			if (c->name_hash == cH("cElement"))
			{
				element = (cElement*)c;
				draw_cmd = element->cmds.add([](void* c, graphics::Canvas* canvas) {
					(*(cEditPrivate**)c)->draw(canvas);
				}, new_mail_p(this));
			}
			else if (c->name_hash == cH("cText"))
				text = (cText*)c;
			else if (c->name_hash == cH("cEventReceiver"))
			{
				event_receiver = (cEventReceiver*)c;
				event_receiver->accept_key = true;
				key_listener = event_receiver->key_listeners.add([](void* c, KeyState action, int value) {
					auto thiz = *(cEditPrivate**)c;
					auto text = thiz->text;
					auto& str = text->text();

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
							auto copied = get_clipboard();
							thiz->cursor = 0;
							text->set_text(*copied.p);
							delete_mail(copied);
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
						auto& str = text->text();
						auto scale = element->global_scale;
						if (atlas->draw_type == graphics::FontDrawSdf)
							scale *= text->scale_;
						auto font_size = text->font_size_;
						auto line_space = font_size * scale;
						auto y = element->global_pos.y();
						for (auto p = str.c_str(); ; p++)
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
								thiz->cursor = p - str.c_str();
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
				auto font_atlas = text->font_atlas;
				auto global_scale = element->global_scale;
				auto fs = text->last_font_size;
				if (fs != last_font_size)
				{
					last_font_size = fs;
					cursor_glyph = font_atlas->get_glyph(L'|', last_font_size);
				}
				auto scale = font_atlas->draw_type == graphics::FontDrawSdf ? text->scale_ : 1.f;
				canvas->add_text(font_atlas, { cursor_glyph }, last_font_size, scale * global_scale, element->global_pos +
					(Vec2f(element->inner_padding_[0], element->inner_padding_[1]) + 
					Vec2f(font_atlas->get_text_offset(std::wstring_view(text->text().c_str(), cursor), last_font_size)) * scale) * global_scale,
					alpha_mul(text->color, element->alpha_));
			}
		}
	};

	cEdit* cEdit::create()
	{
		return new cEditPrivate();
	}

	Entity* create_standard_edit(float width, graphics::FontAtlas* font_atlas, float font_size_scale)
	{
		auto e_edit = Entity::create();
		{
			auto c_element = cElement::create();
			c_element->size_.x() = width + 8.f;
			c_element->inner_padding_ = Vec4f(4.f, 2.f, 4.f, 2.f);
			c_element->color_ = default_style.frame_color_normal;
			c_element->frame_color_ = default_style.text_color_normal;
			c_element->frame_thickness_ = 2.f;
			e_edit->add_component(c_element);

			auto c_text = cText::create(font_atlas);
			c_text->font_size_ = default_style.font_size * font_size_scale;
			c_text->auto_width_ = false;
			e_edit->add_component(c_text);

			e_edit->add_component(cEventReceiver::create());

			if (width == 0.f)
			{
				auto c_aligner = cAligner::create();
				c_aligner->width_policy_ = SizeFitParent;
				e_edit->add_component(c_aligner);
			}

			e_edit->add_component(cEdit::create());
		}

		return e_edit;
	}
}
