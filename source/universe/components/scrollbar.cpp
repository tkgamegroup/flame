#include <flame/universe/default_style.h>
#include <flame/universe/components/element.h>
#include <flame/universe/components/event_receiver.h>
#include <flame/universe/components/aligner.h>
#include <flame/universe/components/style.h>
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

		cScrollbarThumbPrivate(ScrollbarType _type)
		{
			element = nullptr;
			event_receiver = nullptr;
			scrollbar = nullptr;

			type = _type;
			target_layout = nullptr;

			mouse_listener = nullptr;
		}

		~cScrollbarThumbPrivate()
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
					auto content_size = target_layout->content_size.y() + 20.f;
					if (content_size > target_element->height)
						element->height = target_element->height / content_size * scrollbar->element->height;
					else
						element->height = 0.f;
				}
				else
					element->height = 0.f;
			}
			else
			{
				if (target_element->width > 0.f)
				{
					auto content_size = target_layout->content_size.x() + 20.f;
					if (content_size > target_element->width)
						element->width = target_element->width / content_size * scrollbar->element->width;
					else
						element->width = 0.f;
				}
				else
					element->width = 0.f;
			}
		}
	};

	void cScrollbarThumb::start()
	{
		((cScrollbarThumbPrivate*)this)->start();
	}

	void cScrollbarThumb::update()
	{
		((cScrollbarThumbPrivate*)this)->update();
	}

	cScrollbarThumb* cScrollbarThumb::create(ScrollbarType type)
	{
		return new cScrollbarThumbPrivate(type);
	}

	Entity* create_standard_scrollbar(ScrollbarType type)
	{
		auto e_scrollbar = Entity::create();
		{
			auto c_element = cElement::create();
			c_element->width = 10.f;
			c_element->background_color = default_style.scrollbar_color;
			e_scrollbar->add_component(c_element);

			auto c_aligner = cAligner::create();
			c_aligner->height_policy = SizeFitLayout;
			e_scrollbar->add_component(c_aligner);

			e_scrollbar->add_component(cEventReceiver::create());

			e_scrollbar->add_component(cScrollbar::create());
		}

		auto e_scrollbar_thumb = Entity::create();
		e_scrollbar->add_child(e_scrollbar_thumb);
		{
			auto c_element = cElement::create();
			c_element->width = 10.f;
			c_element->height = 10.f;
			e_scrollbar_thumb->add_component(c_element);

			e_scrollbar_thumb->add_component(cEventReceiver::create());

			e_scrollbar_thumb->add_component(cStyleBackgroundColor::create(default_style.scrollbar_thumb_color_normal, default_style.scrollbar_thumb_color_hovering, default_style.scrollbar_thumb_color_active));

			e_scrollbar_thumb->add_component(cScrollbarThumb::create(type));
		}

		return e_scrollbar;
	}
}
