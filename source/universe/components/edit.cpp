#include <flame/graphics/font.h>
#include <flame/universe/default_style.h>
#include <flame/universe/components/element.h>
#include <flame/universe/components/text.h>
#include <flame/universe/components/event_receiver.h>
#include <flame/universe/components/custom_draw.h>
#include <flame/universe/components/aligner.h>
#include <flame/universe/components/edit.h>

#include "../renderpath/canvas_make_cmd/canvas.h"

namespace flame
{
	struct cEditPrivate : cEdit
	{
		void* key_listener;
		void* mouse_listener;
		void* draw_cmd;

		cEditPrivate()
		{
			element = nullptr;
			text = nullptr;
			event_receiver = nullptr;
			custom_draw = nullptr;

			cursor = 0;

			key_listener = nullptr;
			mouse_listener = nullptr;
			draw_cmd = nullptr;
		}

		~cEditPrivate()
		{
			if (!entity->dying_)
			{
				event_receiver->key_listeners.remove(key_listener);
				event_receiver->mouse_listeners.remove(mouse_listener);
				custom_draw->cmds.remove(draw_cmd);
			}
		}

		void on_component_added(Component* c) override
		{
			if (c->name_hash == cH("Element"))
				element = (cElement*)c;
			else if (c->name_hash == cH("Text"))
				text = (cText*)c;
			else if (c->name_hash == cH("EventReceiver"))
			{
				event_receiver = (cEventReceiver*)c;
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
					auto element = thiz->element;
					auto text = thiz->text;
					auto& str = text->text();

					if (is_mouse_down(action, key, true) && key == Mouse_Left)
					{
						auto scl = text->sdf_scale_ * element->global_scale;

						auto lh = text->font_atlas->pixel_height * scl;
						auto y = element->global_pos.y();
						for (auto p = str.c_str(); ; p++)
						{
							if (y < pos.y() && pos.y() < y + lh)
							{
								auto x = element->global_pos.x();
								for (;; p++)
								{
									if (!*p)
										break;
									if (*p == '\n' || *p == '\r')
										break;
									auto w = text->font_atlas->get_glyph(*p == '\t' ? ' ' : *p)->advance * scl;
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
								y += lh;
						}
					}
				}, new_mail_p(this));
			}
			else if (c->name_hash == cH("CustomDraw"))
			{
				custom_draw = (cCustomDraw*)c;
				draw_cmd = custom_draw->cmds.add([](void* c, graphics::Canvas* canvas) {
					(*(cEditPrivate**)c)->draw(canvas);
				}, new_mail_p(this));
			}
		}

		void draw(graphics::Canvas* canvas)
		{
			if (!element->cliped && event_receiver->focusing && (int(looper().total_time * 2.f) % 2 == 0))
			{
				auto text_scale = text->sdf_scale_ * element->global_scale;
				canvas->add_text(text->font_atlas, element->global_pos +
					Vec2f(element->inner_padding_[0], element->inner_padding_[1]) * element->global_scale +
					Vec2f(text->font_atlas->get_text_offset(std::wstring_view(text->text().c_str(), cursor))) * text_scale,
					alpha_mul(text->color, element->alpha), L"|", text_scale);
			}
		}
	};

	cEdit* cEdit::create()
	{
		return new cEditPrivate();
	}

	Entity* create_standard_edit(float width, graphics::FontAtlas* font_atlas, float sdf_scale)
	{
		auto e_edit = Entity::create();
		{
			auto c_element = cElement::create();
			c_element->size_.x() = width + 8.f;
			c_element->inner_padding_ = Vec4f(4.f, 2.f, 4.f, 2.f);
			c_element->color = default_style.frame_color_normal;
			c_element->frame_color = default_style.text_color_normal;
			c_element->frame_thickness = 2.f;
			e_edit->add_component(c_element);

			auto c_text = cText::create(font_atlas);
			c_text->sdf_scale_ = sdf_scale;
			c_text->auto_width_ = false;
			e_edit->add_component(c_text);

			e_edit->add_component(cEventReceiver::create());

			e_edit->add_component(cCustomDraw::create());

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
