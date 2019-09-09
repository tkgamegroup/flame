#include <flame/graphics/canvas.h>
#include <flame/universe/default_style.h>
#include <flame/universe/components/element.h>
#include <flame/universe/components/text.h>
#include <flame/universe/components/event_receiver.h>
#include <flame/universe/components/style.h>
#include <flame/universe/components/checkbox.h>

namespace flame
{
	struct cCheckboxPrivate : cCheckbox
	{
		void* mouse_listener;

		std::vector<std::unique_ptr<Closure<void(void* c, bool checked)>>> changed_listeners;

		cCheckboxPrivate()
		{
			element = nullptr;
			event_receiver = nullptr;

			checked = false;

			mouse_listener = nullptr;
		}

		~cCheckboxPrivate()
		{
			event_receiver = (cEventReceiver*)(entity->find_component(cH("EventReceiver")));
			if (event_receiver)
				event_receiver->remove_mouse_listener(mouse_listener);
		}

		void start()
		{
			element = (cElement*)(entity->find_component(cH("Element")));
			assert(element);
			event_receiver = (cEventReceiver*)(entity->find_component(cH("EventReceiver")));
			assert(event_receiver);

			mouse_listener = event_receiver->add_mouse_listener([](void* c, KeyState action, MouseKey key, const Vec2f& pos) {
				if (is_mouse_clicked(action, key))
				{
					auto thiz = *(cCheckboxPrivate**)c;
					thiz->checked = !thiz->checked;
				}

			}, new_mail_p(this));
		}

		void update()
		{
			if (!element->cliped)
			{
				std::vector<Vec2f> points;
				path_rect(points, Vec2f(element->global_x, element->global_y), Vec2f(element->global_height, element->global_height));
				points.push_back(points[0]);
				element->canvas->stroke(points, element->background_color, 2.f * element->global_scale);
				if (checked)
				{
					std::vector<Vec2f> points;
					path_rect(points, Vec2f(element->global_x, element->global_y) + 3.f * element->global_scale, Vec2f(element->global_height, element->global_height) - 6.f * element->global_scale);
					element->canvas->fill(points, element->background_color);
				}
			}
		}
	};

	void cCheckbox::start()
	{
		((cCheckboxPrivate*)this)->start();
	}

	void cCheckbox::update()
	{
		((cCheckboxPrivate*)this)->update();
	}

	cCheckbox* cCheckbox::create()
	{
		return new cCheckboxPrivate();
	}

	void* cCheckbox::add_changed_listener(void (*listener)(void* c, bool checked), const Mail<>& capture)
	{
		auto c = new Closure<void(void* c, bool checked)>;
		c->function = listener;
		c->capture = capture;
		((cCheckboxPrivate*)this)->changed_listeners.emplace_back(c);
		return c;
	}

	void cCheckbox::remove_changed_listener(void* ret_by_add)
	{
		auto& listeners = ((cCheckboxPrivate*)this)->changed_listeners;
		for (auto it = listeners.begin(); it != listeners.end(); it++)
		{
			if (it->get() == ret_by_add)
			{
				listeners.erase(it);
				return;
			}
		}
	}

	void cCheckbox::set_checked(bool _checked)
	{
		checked = _checked;
		for (auto& l : ((cCheckboxPrivate*)this)->changed_listeners)
			l->function(l->capture.p, checked);
	}

	Entity* create_standard_checkbox(graphics::FontAtlas* font_atlas, float sdf_scale, const std::wstring& text, bool checked)
	{
		auto e_checkbox = Entity::create();
		{
			auto c_element = cElement::create();
			c_element->width = 16.f;
			c_element->height = 16.f;
			c_element->inner_padding = Vec4f(20.f, 1.f, 1.f, 1.f);
			c_element->draw = false;
			e_checkbox->add_component(c_element);

			auto c_text = cText::create(font_atlas);
			c_text->sdf_scale = sdf_scale;
			c_text->set_text(text);
			e_checkbox->add_component(c_text);

			e_checkbox->add_component(cEventReceiver::create());

			e_checkbox->add_component(cStyleBackgroundColor::create(default_style.button_color_normal, default_style.button_color_hovering, default_style.button_color_active));

			auto c_checkbox = cCheckbox::create();
			c_checkbox->checked = checked;
			e_checkbox->add_component(c_checkbox);
		}

		return e_checkbox;
	}
}
