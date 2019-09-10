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
		void* mouse_listener;

		cScrollbarPrivate()
		{
			element = nullptr;
			event_receiver = nullptr;

			mouse_listener = nullptr;
		}

		~cScrollbarPrivate()
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
			thumb = (cScrollbarThumb*)(entity->child(0)->find_component(cH("ScrollbarThumb")));
			assert(thumb);

			mouse_listener = event_receiver->add_mouse_listener([](void* c, KeyState action, MouseKey key, const Vec2f& pos) {
				auto thiz = (*(cScrollbarPrivate**)c);
				if (is_mouse_scroll(action, key))
					thiz->thumb->v -= pos.x() * 5.f;
			}, new_mail_p(this));
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
			step = 1.f;

			content_size = 0.f;

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
						thiz->v = pos.y();
					else
						thiz->v = pos.x();
				}
				else if (is_mouse_scroll(action, key))
					thiz->v -= pos.x() * 5.f;
			}, new_mail_p(this));
		}

		void update()
		{
			auto target_element = target_layout->element;
			if (type == ScrollbarVertical)
			{
				if (target_element->height > 0.f)
				{
					content_size = target_layout->content_size.y() + 20.f;
					if (content_size > target_element->height)
						element->height = target_element->height / content_size * scrollbar->element->height;
					else
						element->height = 0.f;
				}
				else
					element->height = 0.f;
				element->y += v;
				element->y = clamp(element->y, 0.f, scrollbar->element->height - element->height);
				target_layout->scroll_offset.y() = -int(element->y / scrollbar->element->height * content_size / step) * step;
			}
			else
			{
				if (target_element->width > 0.f)
				{
					content_size = target_layout->content_size.x() + 20.f;
					if (content_size > target_element->width)
						element->width = target_element->width / content_size * scrollbar->element->width;
					else
						element->width = 0.f;
				}
				else
					element->width = 0.f;
				element->x += v;
				element->x = clamp(element->x, 0.f, scrollbar->element->width - element->width);
				target_layout->scroll_offset.x() = -int(element->x / scrollbar->element->width * content_size / step) * step;
			}
			v = 0.f;
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

	Entity* wrap_standard_scrollbar(Entity* e, ScrollbarType type, bool container_fit_parent, float scrollbar_step)
	{
		auto e_container = Entity::create();
		{
			e_container->add_component(cElement::create());

			if (container_fit_parent)
			{
				auto c_aligner = cAligner::create();
				c_aligner->width_policy = SizeFitParent;
				c_aligner->height_policy = SizeFitParent;
				e_container->add_component(c_aligner);
			}

			auto c_layout = cLayout::create();
			c_layout->type = type == ScrollbarVertical ? LayoutHorizontal : LayoutVertical;
			c_layout->item_padding = 4.f;
			c_layout->width_fit_children = false;
			c_layout->height_fit_children = false;
			e_container->add_component(c_layout);
		}

		e_container->add_child(e);

		auto e_scrollbar = Entity::create();
		e_container->add_child(e_scrollbar);
		{
			auto c_element = cElement::create();
			if (type == ScrollbarVertical)
				c_element->width = 10.f;
			else
				c_element->height = 10.f;
			c_element->background_color = default_style.scrollbar_color;
			e_scrollbar->add_component(c_element);

			auto c_aligner = cAligner::create();
			if (type == ScrollbarVertical)
				c_aligner->height_policy = SizeFitParent;
			else
				c_aligner->width_policy = SizeFitParent;
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

			auto c_scrollbar_thumb = cScrollbarThumb::create(type);
			c_scrollbar_thumb->step = scrollbar_step;
			e_scrollbar_thumb->add_component(c_scrollbar_thumb);
		}

		return e_container;
	}
}
