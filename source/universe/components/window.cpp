#include <flame/universe/topmost.h>
#include <flame/universe/components/element.h>
#include <flame/universe/components/event_dispatcher.h>
#include <flame/universe/components/event_receiver.h>
#include <flame/universe/components/window.h>

namespace flame
{
	struct cWindowPrivate : cWindow
	{
		void* mouse_listener;

		std::vector<std::unique_ptr<Closure<void(void* c)>>> pos_listeners;

		cWindowPrivate()
		{
			element = nullptr;
			event_receiver = nullptr;

			mouse_listener = nullptr;
		}

		~cWindowPrivate()
		{
			event_receiver->remove_mouse_listener(mouse_listener);
		}

		void start()
		{
			element = (cElement*)(entity->find_component(cH("Element")));
			assert(element);
			event_receiver = (cEventReceiver*)(entity->find_component(cH("EventReceiver")));
			assert(event_receiver);

			mouse_listener = event_receiver->add_mouse_listener([](void* c, KeyState action, MouseKey key, const Vec2f& pos) {
				auto thiz = (*(cWindowPrivate**)c);
				if (is_mouse_down(action, key, true) && key == Mouse_Left)
				{
					looper().add_delay_event([](void* c) {
						auto e = *(Entity**)c;
						e->parent()->reposition_child(e, -1);
					}, new_mail_p(thiz->entity));
				}
				else if (thiz->event_receiver->dragging && is_mouse_move(action, key))
				{
					auto e = thiz->element;
					auto x = pos.x() / e->global_scale;
					auto y = pos.y() / e->global_scale;
					e->x += x;
					e->y += y;
					for (auto& l : thiz->pos_listeners)
						l->function(l->capture.p);
				}
			}, new_mail_p(this));
		}
	};

	cWindow::~cWindow()
	{
		((cWindowPrivate*)this)->~cWindowPrivate();
	}

	void cWindow::start()
	{
		((cWindowPrivate*)this)->start();
	}

	void cWindow::update() 
	{
	}

	void* cWindow::add_pos_listener(void (*listener)(void* c), const Mail<>& capture)
	{
		auto c = new Closure<void(void* c)>;
		c->function = listener;
		c->capture = capture;
		((cWindowPrivate*)this)->pos_listeners.emplace_back(c);
		return c;
	}

	void cWindow::remove_pos_listener(void* ret_by_add)
	{
		auto& listeners = ((cWindowPrivate*)this)->pos_listeners;
		for (auto it = listeners.begin(); it != listeners.end(); it++)
		{
			if (it->get() == ret_by_add)
			{
				listeners.erase(it);
				return;
			}
		}
	}

	cWindow* cWindow::create()
	{
		return new cWindowPrivate;
	}

	struct cSizeDraggerPrivate : cSizeDragger
	{
		void* mouse_listener;

		cSizeDraggerPrivate()
		{
			event_receiver = nullptr;

			mouse_listener = nullptr;
		}

		~cSizeDraggerPrivate()
		{
			event_receiver->remove_mouse_listener(mouse_listener);
		}

		void start()
		{
			event_receiver = (cEventReceiver*)(entity->find_component(cH("EventReceiver")));
			assert(event_receiver);

			mouse_listener = event_receiver->add_mouse_listener([](void* c, KeyState action, MouseKey key, const Vec2f& pos) {
				auto thiz = (*(cSizeDraggerPrivate**)c);
				if (is_mouse_move(action, key) && thiz->event_receiver->dragging)
				{
					auto w = thiz->entity->parent();
					auto element = (cElement*)w->find_component(cH("Element"));
					element->width += pos.x();
					element->height += pos.y();
				}
			}, new_mail_p(this));
		}
	};

	cSizeDragger::~cSizeDragger()
	{
		((cSizeDraggerPrivate*)this)->~cSizeDraggerPrivate();
	}

	void cSizeDragger::start()
	{
		((cSizeDraggerPrivate*)this)->start();
	}

	void cSizeDragger::update()
	{
	}

	cSizeDragger* cSizeDragger::create()
	{
		return new cSizeDraggerPrivate;
	}

	struct cDockableTitlePrivate : cDockableTitle
	{
		void* mouse_listener;

		cDockableTitlePrivate()
		{
			element = nullptr;
			event_receiver = nullptr;

			flying = false;

			mouse_listener = nullptr;
		}

		~cDockableTitlePrivate()
		{
			event_receiver->remove_mouse_listener(mouse_listener);
		}

		void start()
		{
			element = (cElement*)(entity->find_component(cH("Element")));
			assert(element);
			event_receiver = (cEventReceiver*)(entity->find_component(cH("EventReceiver")));
			assert(event_receiver);

			mouse_listener = event_receiver->add_mouse_listener([](void* c, KeyState action, MouseKey key, const Vec2f& pos) {
				auto thiz = (*(cDockableTitle**)c);
				if (is_mouse_move(action, key) && thiz->event_receiver->dragging)
				{
					if (!thiz->flying && !thiz->element->contains(Vec2f(thiz->event_receiver->event_dispatcher->mouse_pos)))
					{
						thiz->flying = true;

						looper().add_delay_event([](void* c) {
							auto thiz = *(cDockableTitle**)c;

							auto e = thiz->entity;
							e->parent()->take_child(e);

							thiz->element->x = thiz->element->global_x;
							thiz->element->y = thiz->element->global_y;
							thiz->root->add_child(e);
						}, new_mail_p(thiz));
					}

					if (thiz->flying)
					{
						auto e = thiz->element;
						auto x = pos.x() / e->global_scale;
						auto y = pos.y() / e->global_scale;
						e->x += x;
						e->y += y;
					}
				}
				else if (is_mouse_up(action, key, true) && key == Mouse_Left)
				{
					if (thiz->flying)
						thiz->flying = false;
				}
			}, new_mail_p(this));
		}
	};

	cDockableTitle::~cDockableTitle()
	{
		((cDockableTitlePrivate*)this)->~cDockableTitlePrivate();
	}

	void cDockableTitle::start()
	{
		((cDockableTitlePrivate*)this)->start();
	}

	void cDockableTitle::update()
	{
	}

	cDockableTitle* cDockableTitle::create()
	{
		return new cDockableTitlePrivate;
	}
}
