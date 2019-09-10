#include <flame/graphics/canvas.h>
#include <flame/graphics/font.h>
#include <flame/universe/default_style.h>
#include <flame/universe/components/element.h>
#include "text_private.h"
#include <flame/universe/components/event_receiver.h>
#include <flame/universe/components/aligner.h>
#include <flame/universe/components/edit.h>

namespace flame
{
	struct cEditPrivate : cEdit
	{
		void* key_listener;

		std::vector<std::unique_ptr<Closure<void(void* c, const wchar_t* text)>>> changed_listeners;

		cEditPrivate()
		{
			element = nullptr;
			text = nullptr;
			event_receiver = nullptr;

			cursor = 0;

			key_listener = nullptr;
		}

		~cEditPrivate()
		{
			event_receiver = (cEventReceiver*)(entity->find_component(cH("EventReceiver")));
			if (event_receiver)
				event_receiver->remove_mouse_listener(key_listener);
		}

		void on_change()
		{
			for (auto& l : changed_listeners)
				l->function(l->capture.p, ((cTextPrivate*)text)->text.c_str());
		}

		void start()
		{
			element = (cElement*)(entity->find_component(cH("Element")));
			assert(element);
			text = (cText*)(entity->find_component(cH("Text")));
			assert(text && !text->auto_size);
			event_receiver = (cEventReceiver*)(entity->find_component(cH("EventReceiver")));
			assert(event_receiver);

			key_listener = event_receiver->add_key_listener([](void* c, KeyState action, uint value) {
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
							thiz->on_change();
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
					default:
						str.insert(str.begin() + thiz->cursor, value);
						thiz->cursor++;
						thiz->on_change();
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
							thiz->on_change();
						}
						break;
					}
				}
			}, new_mail_p(this));
		}

		void update()
		{
			if (!element->cliped && event_receiver->focusing && (int(looper().total_time * 2.f) % 2 == 0))
			{
				auto text_scale = text->sdf_scale * element->global_scale;
				element->canvas->add_text(text->font_atlas, Vec2f(element->global_x, element->global_y) +
					Vec2f(element->inner_padding[0], element->inner_padding[1]) * element->global_scale +
					Vec2f(text->font_atlas->get_text_width(std::wstring_view(text->text().c_str(), cursor)) * text_scale, 0.f),
					alpha_mul(text->color, element->alpha), L"|", text_scale);
			}
		}
	};

	void* cEdit::add_changed_listener(void (*listener)(void* c, const wchar_t* text), const Mail<>& capture)
	{
		auto c = new Closure<void(void* c, const wchar_t* text)>;
		c->function = listener;
		c->capture = capture;
		((cEditPrivate*)this)->changed_listeners.emplace_back(c);
		return c;
	}

	void cEdit::remove_changed_listener(void* ret_by_add)
	{
		auto& listeners = ((cEditPrivate*)this)->changed_listeners;
		for (auto it = listeners.begin(); it != listeners.end(); it++)
		{
			if (it->get() == ret_by_add)
			{
				listeners.erase(it);
				return;
			}
		}
	}

	void cEdit::start()
	{
		((cEditPrivate*)this)->start();
	}

	void cEdit::update()
	{
		((cEditPrivate*)this)->update();
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
			c_element->width = width + 8.f;
			c_element->height = font_atlas->pixel_height * sdf_scale + 4;
			c_element->inner_padding = Vec4f(4.f, 2.f, 4.f, 2.f);
			c_element->background_color = default_style.frame_color_normal;
			c_element->background_frame_color = default_style.text_color_normal;
			c_element->background_frame_thickness = 2.f;
			e_edit->add_component(c_element);

			auto c_text = cText::create(font_atlas);
			c_text->sdf_scale = sdf_scale;
			c_text->auto_size = false;
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
