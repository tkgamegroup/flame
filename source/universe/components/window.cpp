#include <flame/universe/components/element.h>
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
			event_receiver = nullptr;

			mouse_listener = nullptr;
		}

		~cWindowPrivate()
		{
			event_receiver->remove_mouse_listener(mouse_listener);
		}

		void start()
		{
			event_receiver = (cEventReceiver*)(entity->find_component(cH("EventReceiver")));
			assert(event_receiver);

			mouse_listener = event_receiver->add_mouse_listener([](void* c, KeyState action, MouseKey key, const Vec2f& pos) {
				auto thiz = (*(cWindowPrivate**)c);
				if (thiz->event_receiver->dragging && is_mouse_move(action, key))
				{
					auto e = thiz->event_receiver->element;
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
}
