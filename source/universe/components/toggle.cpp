#include <flame/graphics/canvas.h>
#include <flame/universe/default_style.h>
#include <flame/universe/components/element.h>
#include <flame/universe/components/event_receiver.h>
#include <flame/universe/components/style.h>
#include <flame/universe/components/toggle.h>

namespace flame
{
	struct cTogglePrivate : cToggle
	{
		void* mouse_listener;

		cTogglePrivate()
		{
			element = nullptr;
			event_receiver = nullptr;
			style = nullptr;

			toggled = false;

			untoggled_color_normal = Vec4c(color(Vec3f(52.f, 0.23f, 0.97f)), 0.40f * 255.f);
			untoggled_color_hovering = Vec4c(color(Vec3f(52.f, 0.23f, 0.97f)), 1.00f * 255.f);
			untoggled_color_active = Vec4c(color(Vec3f(49.f, 0.43f, 0.97f)), 1.00f * 255.f);
			toggled_color_normal = default_style.button_color_normal;
			toggled_color_hovering = default_style.button_color_hovering;
			toggled_color_active = default_style.button_color_active;

			mouse_listener = nullptr;
		}

		~cTogglePrivate()
		{
			event_receiver->remove_mouse_listener(mouse_listener);
		}

		void start()
		{
			element = (cElement*)(entity->find_component(cH("Element")));
			assert(element);
			event_receiver = (cEventReceiver*)(entity->find_component(cH("EventReceiver")));
			assert(event_receiver);
			style = (cStyleBackgroundColor*)(entity->find_component(cH("StyleBackgroundColor")));

			mouse_listener = event_receiver->add_mouse_listener([](void* c, KeyState action, MouseKey key, const Vec2f& pos) {
				if (is_mouse_clicked(action, key))
				{
					auto thiz = *(cTogglePrivate**)c;
					thiz->toggled = !thiz->toggled;
				}

			}, new_mail_p(this));
		}

		void update()
		{
			if (style)
			{
				if (!toggled)
				{
					style->color_normal = untoggled_color_normal;
					style->color_hovering = untoggled_color_hovering;
					style->color_active = untoggled_color_active;
				}
				else
				{
					style->color_normal = toggled_color_normal;
					style->color_hovering = toggled_color_hovering;
					style->color_active = toggled_color_active;
				}
			}
		}
	};

	cToggle::~cToggle()
	{
		((cTogglePrivate*)this)->~cTogglePrivate();
	}

	void cToggle::start()
	{
		((cTogglePrivate*)this)->start();
	}

	void cToggle::update()
	{
		((cTogglePrivate*)this)->update();
	}

	cToggle* cToggle::create()
	{
		return new cTogglePrivate();
	}
}
