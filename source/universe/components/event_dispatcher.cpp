#include <flame/foundation/window.h>
#include <flame/universe/entity.h>
#include <flame/universe/components/element.h>
#include <flame/universe/components/event_dispatcher.h>
#include <flame/universe/components/event_receiver.h>

namespace flame
{
	struct cEventDispatcherPrivate : cEventDispatcher
	{
		Window* window;

		Vec2i mouse_pos, mouse_pos_prev, mouse_disp;
		int mouse_scroll;
		uint mouse_buttons[3];

		cEventDispatcherPrivate(Entity* e, Window* window) :
			cEventDispatcher(e),
			window(window)
		{
			hovering = nullptr;
			focusing = nullptr;

			mouse_pos = Vec2i(0);
			mouse_pos_prev = Vec2i(0);
			mouse_disp = Vec2i(0);
			mouse_scroll = 0;

			for (auto i = 0; i < FLAME_ARRAYSIZE(mouse_buttons); i++)
				mouse_buttons[i] = KeyStateUp;

			if (window)
			{
				window->add_mouse_listener([](void* c, KeyState action, MouseKey key, const Vec2i& pos) {
					auto thiz = *(cEventDispatcherPrivate **)c;

					if (action == KeyStateNull)
					{
						if (key == Mouse_Middle)
							thiz->mouse_scroll = pos.x();
						else if (key == Mouse_Null)
							thiz->mouse_pos = pos;
					}
					else
					{
						thiz->mouse_buttons[key] = action | KeyStateJust;
						thiz->mouse_pos = pos;
					}
				}, new_mail_p(this));
			}
		}

		void update()
		{
			mouse_disp = mouse_pos - mouse_pos_prev;

			if (focusing)
			{
				if (!focusing->entity->global_visible)
				{
					focusing->hovering = false;
					focusing->focusing = false;
					focusing->dragging = false;
					focusing = nullptr;
				}
				else
				{
					if (focusing->dragging)
					{
						if (is_mouse_up((KeyState)mouse_buttons[Mouse_Left], Mouse_Left))
							focusing->dragging = false;
					}
				}
			}

			if (!focusing || !focusing->dragging)
			{
				hovering->hovering = false;
				hovering = nullptr;
				entity->traverse_backward([](void* c, Entity* e) {
					auto thiz = *(cEventDispatcherPrivate * *)c;
					if (thiz->hovering)
						return;

					auto er = (cEventReceiver*)e->find_component(cH("EventReceiver"));
					if (er)
					{
						if (er->element->contains(Vec2f(thiz->mouse_pos)))
						{
							er->hovering = true;
							thiz->hovering = er;
						}
					}
				}, new_mail_p(this));
			}
			if (is_mouse_down((KeyState)mouse_buttons[Mouse_Left], Mouse_Left, true))
			{
				focusing->focusing = false;
				focusing = nullptr;
				if (hovering)
				{
					focusing = hovering;
					hovering->focusing = true;
					hovering->dragging = true;
				}
			}
			if (hovering)
			{
				if (mouse_disp != 0)
					hovering->on_mouse(KeyStateNull, Mouse_Null, Vec2f(mouse_disp));
				for (auto i = 0; i < FLAME_ARRAYSIZE(mouse_buttons); i++)
				{
					auto s = mouse_buttons[i];
					if (s & KeyStateJust)
						hovering->on_mouse((KeyState)s, (MouseKey)i, Vec2f(0.f));
				}
			}

			for (auto i = 0; i < FLAME_ARRAYSIZE(mouse_buttons); i++)
				mouse_buttons[i] &= ~KeyStateJust;

			mouse_pos_prev = mouse_pos;
			mouse_scroll = 0;
		}
	};

	cEventDispatcher::cEventDispatcher(Entity* e) :
		Component("EventDispatcher", e)
	{
	}

	cEventDispatcher::~cEventDispatcher()
	{
	}

	void cEventDispatcher::update()
	{
		((cEventDispatcherPrivate*)this)->update();
	}

	cEventDispatcher* cEventDispatcher::create(Entity* e, Window* window)
	{
		return new cEventDispatcherPrivate(e, window);
	}
}
