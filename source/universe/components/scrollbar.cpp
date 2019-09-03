#include <flame/universe/components/element.h>
#include <flame/universe/components/event_receiver.h>
#include <flame/universe/components/layout.h>
#include <flame/universe/components/scrollbar.h>

namespace flame
{
	struct cScrollbarPrivate : cScrollbar
	{
		cScrollbarPrivate()
		{
			element = nullptr;
			event_receiver = nullptr;
		}

		void start()
		{
			element = (cElement*)(entity->find_component(cH("Element")));
			assert(element);
			event_receiver = (cEventReceiver*)(entity->find_component(cH("EventReceiver")));
			assert(event_receiver);
		}
	};

	cScrollbar::~cScrollbar()
	{
	}

	void cScrollbar::start()
	{
		((cScrollbarPrivate*)this)->start();
	}

	void cScrollbar::update()
	{
	}

	cScrollbar* cScrollbar::create()
	{
		return new cScrollbarPrivate();
	}

	struct cScrollbarThumbPrivate : cScrollbarThumb
	{
		void* mouse_listener;

		cScrollbarThumbPrivate()
		{
			element = nullptr;
			event_receiver = nullptr;
			scrollbar = nullptr;

			type = ScrollbarVertical;
			target_layout = nullptr;

			mouse_listener = nullptr;
		}

		~cScrollbarThumbPrivate()
		{
			event_receiver->remove_mouse_listener(mouse_listener);
		}

		void start()
		{
			element = (cElement*)(entity->find_component(cH("Element")));
			assert(element);
			event_receiver = (cEventReceiver*)(entity->find_component(cH("EventReceiver")));
			assert(event_receiver);
			auto parent = entity->parent();
			scrollbar = (cScrollbar*)(parent->find_component(cH("Scrollbar")));
			assert(scrollbar);
			target_layout = (cLayout*)(parent->parent()->child(0)->find_component(cH("Layout")));
			assert(target_layout);

			mouse_listener = event_receiver->add_mouse_listener([](void* c, KeyState action, MouseKey key, const Vec2f& pos) {
				auto thiz = (*(cScrollbarThumbPrivate**)c);
				if (thiz->event_receiver->active && is_mouse_move(action, key))
				{
					if (thiz->type == ScrollbarVertical)
					{
						thiz->element->y += pos.y();
						if (thiz->element->y < 0.f)
							thiz->element->y = 0.f;
						if (thiz->element->y + thiz->element->height > thiz->scrollbar->element->height)
							thiz->element->y = thiz->scrollbar->element->height - thiz->element->height;
						thiz->target_layout->scroll_offset.y() = -thiz->element->y / thiz->scrollbar->element->height * thiz->target_layout->content_size.y();
					}
					else
					{
						thiz->element->x += pos.x();
						if (thiz->element->x < 0.f)
							thiz->element->x = 0.f;
						if (thiz->element->x + thiz->element->width > thiz->scrollbar->element->width)
							thiz->element->x = thiz->scrollbar->element->width - thiz->element->width;
						thiz->target_layout->scroll_offset.x() = -thiz->element->x / thiz->scrollbar->element->width * thiz->target_layout->content_size.x();
					}
				}
			}, new_mail_p(this));
		}

		void update()
		{
			auto target_element = target_layout->element;
			if (type == ScrollbarVertical)
			{
				if (target_element->height > 0.f)
				{
					if (target_layout->content_size.y() > target_element->height)
					{
						entity->visible = true;
						element->height = target_element->height / target_layout->content_size.y() * scrollbar->element->height;
					}
					else
						entity->visible = false;
				}
				else
					element->height = 0.f;
			}
			else
			{
				if (target_element->width > 0.f)
				{
					if (target_layout->content_size.x() > target_element->width)
					{
						entity->visible = true;
						element->width = target_element->width / target_layout->content_size.x() * scrollbar->element->width;
					}
					else
						entity->visible = false;
				}
				else
					element->width = 0.f;
			}
		}
	};

	cScrollbarThumb::~cScrollbarThumb()
	{
		((cScrollbarThumbPrivate*)this)->~cScrollbarThumbPrivate();
	}

	void cScrollbarThumb::start()
	{
		((cScrollbarThumbPrivate*)this)->start();
	}

	void cScrollbarThumb::update()
	{
		((cScrollbarThumbPrivate*)this)->update();
	}

	cScrollbarThumb* cScrollbarThumb::create()
	{
		return new cScrollbarThumbPrivate;
	}
}
