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

		void on_component_added(Component* c) override
		{
			if (c->name_hash == cH("Element"))
				element = (cElement*)c;
		}
	};

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
			if (!entity->dying_)
				event_receiver->mouse_listeners.remove(mouse_listener);
		}

		void on_added() override
		{
			auto parent = entity->parent();
			scrollbar = parent->get_component(Scrollbar);
			target_layout = parent->parent()->child(0)->get_component(Layout);
		}

		void on_component_added(Component* c) override
		{
			if (c->name_hash == cH("Element"))
				element = (cElement*)c;
			else if (c->name_hash == cH("EventReceiver"))
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
				if (target_element->size_.y() > 0.f)
				{
					content_size = target_layout->content_size.y() + 20.f;
					if (content_size > target_element->size_.y())
						element->set_height(target_element->size_.y() / content_size * scrollbar->element->size_.y());
					else
						element->set_height(0.f);
				}
				else
					element->set_height(0.f);
				v += element->size_.y();
				element->set_y(element->size_.y() > 0.f ? clamp(v, 0.f, scrollbar->element->size_.y() - element->size_.y()) : 0.f);
				target_layout->set_y_scroll_offset(-int(element->pos_.y() / scrollbar->element->size_.y() * content_size / step) * step);
			}
			else
			{
				if (target_element->size_.x() > 0.f)
				{
					content_size = target_layout->content_size.x() + 20.f;
					if (content_size > target_element->size_.x())
						element->set_width(target_element->size_.x() / content_size * scrollbar->element->size_.x());
					else
						element->set_width(0.f);
				}
				else
					element->set_width(0.f);
				v += element->size_.x();
				element->set_x(element->size_.x() > 0.f ? clamp(v, 0.f, scrollbar->element->size_.x() - element->size_.x()) : 0.f);
				target_layout->set_x_scroll_offset(-int(element->pos_.x() / scrollbar->element->size_.x() * content_size / step) * step);
			}
			v = 0.f;
		}
	};

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
				c_aligner->width_policy_ = SizeFitParent;
				c_aligner->height_policy_ = SizeFitParent;
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
				c_element->size_.x() = 10.f;
			else
				c_element->size_.y() = 10.f;
			c_element->color = default_style.scrollbar_color;
			e_scrollbar->add_component(c_element);

			auto c_aligner = cAligner::create();
			if (type == ScrollbarVertical)
				c_aligner->height_policy_ = SizeFitParent;
			else
				c_aligner->width_policy_ = SizeFitParent;
			e_scrollbar->add_component(c_aligner);

			e_scrollbar->add_component(cEventReceiver::create());

			e_scrollbar->add_component(cScrollbar::create());
		}

		auto e_scrollbar_thumb = Entity::create();
		e_scrollbar->add_child(e_scrollbar_thumb);
		{
			auto c_element = cElement::create();
			c_element->size_ = 10.f;
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
			c_aligner->width_policy_ = SizeFitParent;
			c_aligner->height_policy_ = SizeFitParent;
			e_overlayer->add_component(c_aligner);
		}

		return e_container;
	}
}
