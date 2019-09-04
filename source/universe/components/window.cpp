#include <flame/graphics/canvas.h>
#include <flame/universe/topmost.h>
#include <flame/universe/components/element.h>
#include <flame/universe/components/event_dispatcher.h>
#include <flame/universe/components/event_receiver.h>
#include <flame/universe/components/layout.h>
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
				else if (thiz->event_receiver->active && is_mouse_move(action, key))
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
				if (is_mouse_move(action, key) && thiz->event_receiver->active)
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

	struct cDockerTabPrivate : cDockerTab
	{
		void* mouse_listener;
		void* drag_and_drop_listener;

		cDockerTabPrivate()
		{
			element = nullptr;
			event_receiver = nullptr;

			mouse_listener = nullptr;
			drag_and_drop_listener = nullptr;
		}

		~cDockerTabPrivate()
		{
			event_receiver->remove_mouse_listener(mouse_listener);
			event_receiver->remove_mouse_listener(drag_and_drop_listener);
		}

		void start()
		{
			element = (cElement*)(entity->find_component(cH("Element")));
			assert(element);
			event_receiver = (cEventReceiver*)(entity->find_component(cH("EventReceiver")));
			assert(event_receiver);
			event_receiver->drag_hash = cH("cDockerTab");

			mouse_listener = event_receiver->add_mouse_listener([](void* c, KeyState action, MouseKey key, const Vec2f& pos) {
				auto thiz = (*(cDockerTabPrivate**)c);
				if (is_mouse_move(action, key) && thiz->event_receiver->dragging)
				{
					auto e = thiz->element;
					auto x = pos.x() / e->global_scale;
					auto y = pos.y() / e->global_scale;
					e->x += x;
					e->y += y;
				}
			}, new_mail_p(this));

			drag_and_drop_listener = event_receiver->add_drag_and_drop_listener([](void* c, DragAndDrop action, cEventReceiver* er, const Vec2f& pos) {
				if (action == DragStart)
				{
					auto thiz = (*(cDockerTabPrivate**)c);
					looper().add_delay_event([](void* c) {
						auto thiz = *(cDockerTabPrivate**)c;

						auto e = thiz->entity;
						e->parent()->take_child(e);

						thiz->element->x = thiz->element->global_x;
						thiz->element->y = thiz->element->global_y;
						thiz->root->add_child(e);
					}, new_mail_p(thiz));
				}
			}, new_mail_p(this));
		}
	};

	cDockerTab::~cDockerTab()
	{
		((cDockerTabPrivate*)this)->~cDockerTabPrivate();
	}

	void cDockerTab::start()
	{
		((cDockerTabPrivate*)this)->start();
	}

	void cDockerTab::update()
	{
	}

	cDockerTab* cDockerTab::create()
	{
		return new cDockerTabPrivate;
	}

	struct cDockerTabbarPrivate : cDockerTabbar
	{
		void* drag_and_drop_listener;
		cEventReceiver* drop_er;
		uint drop_er_pos;
		bool show_drop_tip;
		float show_drop_pos;

		cDockerTabbarPrivate()
		{
			element = nullptr;
			event_receiver = nullptr;

			drag_and_drop_listener = nullptr;
			drop_er = nullptr;
			drop_er_pos = 0;
			show_drop_tip = false;
		}

		~cDockerTabbarPrivate()
		{
			event_receiver->remove_mouse_listener(drag_and_drop_listener);
		}

		uint calc_pos(float x, float* out)
		{
			for (auto i = 0; i < entity->child_count(); i++)
			{
				auto element = (cElement*)entity->child(i)->find_component(cH("Element"));
				auto half = element->global_x + element->width * 0.5f;
				if (x >= element->global_x && x < half)
				{
					if (out)
						*out = element->global_x;
					return i;
				}
				if (x >= half && x < element->global_x + element->width)
				{
					if (out)
						*out = element->global_x + element->width;
					return i + 1;
				}
			}
			if (out)
			{
				auto element = (cElement*)entity->child(entity->child_count() - 1)->find_component(cH("Element"));
				*out = element->global_x + element->global_width;
			}
			return entity->child_count();
		}

		void start()
		{
			element = (cElement*)(entity->find_component(cH("Element")));
			assert(element);
			event_receiver = (cEventReceiver*)(entity->find_component(cH("EventReceiver")));
			assert(event_receiver);
			event_receiver->set_acceptable_drops({ cH("DockerTitle") });

			drag_and_drop_listener = event_receiver->add_drag_and_drop_listener([](void* c, DragAndDrop action, cEventReceiver* er, const Vec2f& pos) {
				auto thiz = (*(cDockerTabbarPrivate**)c);
				if (action == DragOvering)
				{
					auto idx = thiz->calc_pos(pos.x(), &thiz->show_drop_pos);
					if (idx == thiz->entity->child_count())
						thiz->show_drop_pos -= 10.f;
					else if (idx != 0)
						thiz->show_drop_pos -= 5.f;
					thiz->show_drop_tip = true;
				}
				else if (action == Dropped)
				{
					thiz->drop_er = er;
					thiz->drop_er_pos = thiz->calc_pos(pos.x(), nullptr);
					looper().add_delay_event([](void* c) {
						auto thiz = *(cDockerTabbarPrivate**)c;

						auto e = thiz->drop_er->entity;
						e->parent()->take_child(e);

						thiz->entity->add_child(e, thiz->drop_er_pos);
					}, new_mail_p(thiz));
				}
			}, new_mail_p(this));
		}

		void update()
		{
			if (show_drop_tip)
			{
				show_drop_tip = false;
				std::vector<Vec2f> points;
				path_rect(points, Vec2f(show_drop_pos, element->global_y), Vec2f(10.f, element->global_height));
				element->canvas->fill(points, Vec4c(50, 80, 200, 128));
			}
		}
	};

	cDockerTabbar::~cDockerTabbar()
	{
		((cDockerTabbarPrivate*)this)->~cDockerTabbarPrivate();
	}

	void cDockerTabbar::start()
	{
		((cDockerTabbarPrivate*)this)->start();
	}

	void cDockerTabbar::update()
	{
		((cDockerTabbarPrivate*)this)->update();
	}

	cDockerTabbar* cDockerTabbar::create()
	{
		return new cDockerTabbarPrivate;
	}
}
