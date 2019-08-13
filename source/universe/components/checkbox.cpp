#include <flame/graphics/canvas.h>
#include <flame/universe/components/element.h>
#include <flame/universe/components/event_receiver.h>
#include <flame/universe/components/checkbox.h>

namespace flame
{
	struct cCheckboxPrivate : cCheckbox
	{
		void* mouse_listener;

		cCheckboxPrivate(Entity* e) :
			cCheckbox(e)
		{
			element = (cElement*)(e->find_component(cH("Element")));
			assert(element);
			event_receiver = (cEventReceiver*)(e->find_component(cH("EventReceiver")));
			assert(event_receiver);

			checked = false;

			mouse_listener = event_receiver->add_mouse_listener([](void* c, KeyState action, MouseKey key, const Vec2f& pos) {
				if (is_mouse_clicked(action, key))
				{
					auto thiz = *(cCheckboxPrivate * *)c;
					thiz->checked = !thiz->checked;
				}

			}, new_mail_p(this));
		}

		~cCheckboxPrivate()
		{
			event_receiver->remove_mouse_listener(mouse_listener);
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

	cCheckbox::cCheckbox(Entity* e) :
		Component("Checkbox", e)
	{
	}

	cCheckbox::~cCheckbox()
	{
		((cCheckboxPrivate*)this)->~cCheckboxPrivate();
	}

	void cCheckbox::update()
	{
		((cCheckboxPrivate*)this)->update();
	}

	cCheckbox* cCheckbox::create(Entity* e)
	{
		return new cCheckboxPrivate(e);
	}
}
