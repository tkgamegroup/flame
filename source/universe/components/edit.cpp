#include "../universe_private.h"
#include <flame/graphics/font.h>
#include <flame/universe/default_style.h>
#include <flame/universe/components/element.h>
#include "text_private.h"
#include <flame/universe/components/event_receiver.h>
#include <flame/universe/components/aligner.h>
#include <flame/universe/components/edit.h>

#include "../renderpath/canvas_make_cmd/canvas.h"

namespace flame
{
	struct cEditPrivate : cEdit
	{
		void* key_listener;
		void* mouse_listener;

		cEditPrivate()
		{
			element = nullptr;
			text = nullptr;
			event_receiver = nullptr;

			cursor = 0;

			key_listener = nullptr;
			mouse_listener = nullptr;

			changed_listeners.hub = new ListenerHub;
		}

		~cEditPrivate()
		{
			if (!entity->dying_)
			{
				event_receiver->key_listeners.remove(key_listener);
				event_receiver->mouse_listeners.remove(mouse_listener);
			}

			delete (ListenerHub*)changed_listeners.hub;
		}

		void on_changed()
		{
			auto& listeners = ((ListenerHub*)changed_listeners.hub)->listeners;
			auto str = ((cTextPrivate*)text)->text.c_str();
			for (auto& l : listeners)
				((void(*)(void*, const wchar_t*))l->function)(l->capture.p, str);
		}

		virtual void on_component_added(Component* c) override
		{
			if (c->type_hash == cH("Element"))
				element = (cElement*)c;
			else if (c->type_hash == cH("Text"))
				text = (cText*)c;
			else if (c->type_hash == cH("EventReceiver"))
			{
				event_receiver = (cEventReceiver*)c;
				key_listener = event_receiver->key_listeners.add([](void* c, KeyState action, int value) {
					auto thiz = *(cEditPrivate**)c;
					auto& str = ((cTextPrivate*)thiz->text)->text;

					if (action == KeyStateNull)
					{
						switch (value)
						{
						case L'\b':
							if (thiz->cursor > 0)
							{
								thiz->cursor--;
								str.erase(str.begin() + thiz->cursor);
								thiz->on_changed();
							}
							break;
						case 22:
						{
							auto copied = get_clipboard();
							thiz->cursor = 0;
							str = *copied.p;
							delete_mail(copied);
						}
							break;
						case 27:
							break;
						case 13:
							value = '\n';
						default:
							str.insert(str.begin() + thiz->cursor, value);
							thiz->cursor++;
							thiz->on_changed();
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
								thiz->on_changed();
							}
							break;
						}
					}
				}, new_mail_p(this));

				mouse_listener = event_receiver->mouse_listeners.add([](void* c, KeyState action, MouseKey key, const Vec2i& pos) {
					auto thiz = *(cEditPrivate**)c;
					auto element = thiz->element;
					auto text = thiz->text;
					auto& str = ((cTextPrivate*)text)->text;

					if (is_mouse_down(action, key, true) && key == Mouse_Left)
					{
						auto scl = text->sdf_scale * element->global_scale;

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
		}

		virtual void update() override
		{
			if (!element->cliped && event_receiver->focusing && (int(looper().total_time * 2.f) % 2 == 0))
			{
				auto text_scale = text->sdf_scale * element->global_scale;
				element->canvas->add_text(text->font_atlas, element->global_pos +
					Vec2f(element->inner_padding[0], element->inner_padding[1]) * element->global_scale +
					Vec2f(text->font_atlas->get_text_offset(std::wstring_view(text->text().c_str(), cursor))) * text_scale,
					alpha_mul(text->color, element->alpha), L"|", text_scale);
			}
		}
	};

	void cEdit::trigger_changed()
	{
		((cEditPrivate*)this)->on_changed();
	}

	cEdit* cEdit::create()
	{
		return new cEditPrivate();
	}

	Entity* create_standard_edit(float width, graphics::FontAtlas* font_atlas, float sdf_scale)
	{
		auto e_edit = Entity::create();
		{
			auto c_element = cElement::create();
			c_element->size.x() = width + 8.f;
			c_element->inner_padding = Vec4f(4.f, 2.f, 4.f, 2.f);
			c_element->color = default_style.frame_color_normal;
			c_element->frame_color = default_style.text_color_normal;
			c_element->frame_thickness = 2.f;
			e_edit->add_component(c_element);

			auto c_text = cText::create(font_atlas);
			c_text->sdf_scale = sdf_scale;
			c_text->auto_width = false;
			e_edit->add_component(c_text);

			if (width == 0.f)
			{
				auto c_aligner = cAligner::create();
				c_aligner->width_policy = SizeFitParent;
				e_edit->add_component(c_aligner);
			}

			e_edit->add_component(cEventReceiver::create());

			e_edit->add_component(cEdit::create());
		}

		return e_edit;
	}
}
