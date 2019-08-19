#include <flame/graphics/canvas.h>
#include <flame/universe/components/element.h>
#include <flame/universe/components/event_receiver.h>
#include <flame/universe/components/checkbox.h>

namespace flame
{
	struct cCheckboxPrivate : cCheckbox
	{
		void* mouse_listener;

		cCheckboxPrivate()
		{
			element = nullptr;
			event_receiver = nullptr;

			checked = false;
		}

		~cCheckboxPrivate()
		{
			event_receiver->remove_mouse_listener(mouse_listener);
		}

		void on_add_to_parent()
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
	};

	cCheckbox::~cCheckbox()
	{
		((cCheckboxPrivate*)this)->~cCheckboxPrivate();
	}

	void cCheckbox::on_add_to_parent()
	{
		((cCheckboxPrivate*)this)->on_add_to_parent();
	}

	void cCheckbox::update()
	{
		((cCheckboxPrivate*)this)->update();
	}

	cCheckbox* cCheckbox::create()
	{
		return new cCheckboxPrivate();
	}
}
