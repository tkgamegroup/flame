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
		}

		void on_component_added(Component* c)
		{
			if (c->type_hash == cH("Element"))
				element = (cElement*)c;
		}
	};

	void cScrollbar::on_component_added(Component* c)
	{
		((cScrollbarPrivate*)this)->on_component_added(c);
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
			if (!entity->dying)
				event_receiver->mouse_listeners.remove(mouse_listener);
		}

		void on_added()
		{
			auto parent = entity->parent();
			scrollbar = (cScrollbar*)(parent->find_component(cH("Scrollbar")));
			target_layout = (cLayout*)(parent->parent()->child(0)->find_component(cH("Layout")));
		}

		void on_component_added(Component* c)
		{
			if (c->type_hash == cH("Element"))
				element = (cElement*)c;
			else if (c->type_hash == cH("EventReceiver"))
			{
				event_receiver = (cEventReceiver*)c;
				mouse_listener = event_receiver->mouse_listeners.add([](void* c, KeyState action, MouseKey key, const Vec2i& pos) {
					auto thiz = (*(cScrollbarThumbPrivate**)c);
					if (thiz->event_receiver->active && is_mouse_move(action, key))
					{
						if (thiz->type == ScrollbarVertical)
							thiz->v = pos.y();
						else
							thiz->v = pos.x();
					}
				}, new_mail_p(this));
			}
		}

		void update()
		{
			auto target_element = target_layout->element;
			if (type == ScrollbarVertical)
			{
				if (target_element->size.y() > 0.f)
				{
					content_size = target_layout->content_size.y() + 20.f;
					if (content_size > target_element->size.y())
						element->size.y() = target_element->size.y() / content_size * scrollbar->element->size.y();
					else
						element->size.y() = 0.f;
				}
				else
					element->size.y() = 0.f;
				element->pos.y() += v;
				element->pos.y() = element->size.y() > 0.f ? clamp(element->pos.y(), 0.f, scrollbar->element->size.y() - element->size.y()) : 0.f;
				target_layout->scroll_offset.y() = -int(element->pos.y() / scrollbar->element->size.y() * content_size / step) * step;
			}
			else
			{
				if (target_element->size.x() > 0.f)
				{
					content_size = target_layout->content_size.x() + 20.f;
					if (content_size > target_element->size.x())
						element->size.x() = target_element->size.x() / content_size * scrollbar->element->size.x();
					else
						element->size.x() = 0.f;
				}
				else
					element->size.x() = 0.f;
				element->pos.x() += v;
				element->pos.x() = element->size.x() > 0.f ? clamp(element->pos.x(), 0.f, scrollbar->element->size.x() - element->size.x()) : 0.f;
				target_layout->scroll_offset.x() = -int(element->pos.x() / scrollbar->element->size.x() * content_size / step) * step;
			}
			v = 0.f;
		}
	};

	void cScrollbarThumb::on_added()
	{
		((cScrollbarThumbPrivate*)this)->on_added();
	}

	void cScrollbarThumb::on_component_added(Component* c)
	{
		((cScrollbarThumbPrivate*)this)->on_component_added(c);
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
			auto c_element = cElement::create();
			c_element->clip_children = true;
			e_container->add_component(c_element);

			if (container_fit_parent)
			{
				auto c_aligner = cAligner::create();
				c_aligner->width_policy = SizeFitParent;
				c_aligner->height_policy = SizeFitParent;
				e_container->add_component(c_aligner);
			}

			auto c_layout = cLayout::create(type == ScrollbarVertical ? LayoutHorizontal : LayoutVertical);
			c_layout->item_padding = 4.f;
			c_layout->width_fit_children = false;
			c_layout->height_fit_children = false;
			c_layout->fence = 2;
			e_container->add_component(c_layout);
		}

		e_container->add_child(e);

		auto e_scrollbar = Entity::create();
		e_container->add_child(e_scrollbar);
		{
			auto c_element = cElement::create();
			if (type == ScrollbarVertical)
				c_element->size.x() = 10.f;
			else
				c_element->size.y() = 10.f;
			c_element->color = default_style.scrollbar_color;
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
			c_element->size = 10.f;
			e_scrollbar_thumb->add_component(c_element);

			e_scrollbar_thumb->add_component(cEventReceiver::create());

			e_scrollbar_thumb->add_component(cStyleColor::create(default_style.scrollbar_thumb_color_normal, default_style.scrollbar_thumb_color_hovering, default_style.scrollbar_thumb_color_active));
		}
		auto c_scrollbar_thumb = cScrollbarThumb::create(type);
		c_scrollbar_thumb->step = scrollbar_step;
		e_scrollbar_thumb->add_component(c_scrollbar_thumb);

		auto e_overlayer = Entity::create();
		e_container->add_child(e_overlayer);
		{
			e_overlayer->add_component(cElement::create());

			auto c_event_receiver = cEventReceiver::create();
			c_event_receiver->penetrable = true;
			c_event_receiver->mouse_listeners.add([](void* c, KeyState action, MouseKey key, const Vec2i& pos) {
				auto thumb = (*(cScrollbarThumb**)c);
				if (is_mouse_scroll(action, key))
					thumb->v -= pos.x() * 20.f;
			}, new_mail_p(c_scrollbar_thumb));
			e_overlayer->add_component(c_event_receiver);

			auto c_aligner = cAligner::create();
			c_aligner->width_policy = SizeFitParent;
			c_aligner->height_policy = SizeFitParent;
			e_overlayer->add_component(c_aligner);
		}

		return e_container;
	}
}
