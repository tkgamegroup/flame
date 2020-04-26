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
			if (c->name_hash == FLAME_CHASH("cElement"))
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
		void* parent_element_listener;
		void* target_element_listener;
		void* target_layout_listener;

		cScrollbarThumbPrivate(ScrollbarType _type)
		{
			element = nullptr;
			event_receiver = nullptr;
			scrollbar = nullptr;

			parent_element = nullptr;

			type = _type;
			target_layout = nullptr;
			step = 1.f;

			mouse_listener = nullptr;
			parent_element_listener = nullptr;
			target_element_listener = nullptr;
			target_layout_listener = nullptr;
		}

		~cScrollbarThumbPrivate()
		{
			if (!entity->dying_)
			{
				event_receiver->mouse_listeners.remove(mouse_listener);
				parent_element->data_changed_listeners.remove(parent_element_listener);
				target_layout->element->data_changed_listeners.remove(target_element_listener);
				target_layout->data_changed_listeners.remove(target_layout_listener);
			}
		}

		void on_added() override
		{
			auto parent = entity->parent();
			parent_element = parent->get_component(cElement);
			parent_element_listener = parent_element->data_changed_listeners.add([](Capture& c, uint hash, void*) {
				if (hash == FLAME_CHASH("size"))
					c.thiz<cScrollbarThumbPrivate>()->update(0.f);
				return true;
			}, Capture().set_thiz(this));
			scrollbar = parent->get_component(cScrollbar);
			target_layout = parent->parent()->child(0)->get_component(cLayout);
			target_element_listener = target_layout->element->data_changed_listeners.add([](Capture& c, uint hash, void*) {
				if (hash == FLAME_CHASH("size"))
					c.thiz<cScrollbarThumbPrivate>()->update(0.f);
				return true;
			}, Capture().set_thiz(this));
			target_layout_listener = target_layout->data_changed_listeners.add([](Capture& c, uint hash, void*) {
				if (hash == FLAME_CHASH("content_size"))
					c.thiz<cScrollbarThumbPrivate>()->update(0.f);
				return true;
			}, Capture().set_thiz(this));
			update(0.f);
		}

		void on_component_added(Component* c) override
		{
			if (c->name_hash == FLAME_CHASH("cElement"))
				element = (cElement*)c;
			else if (c->name_hash == FLAME_CHASH("cEventReceiver"))
			{
				event_receiver = (cEventReceiver*)c;
				mouse_listener = event_receiver->mouse_listeners.add([](Capture& c, KeyStateFlags action, MouseKey key, const Vec2i& pos) {
					auto thiz = c.thiz<cScrollbarThumbPrivate>();
					if (thiz->event_receiver->is_active() && is_mouse_move(action, key))
					{
						if (thiz->type == ScrollbarVertical)
							thiz->update(pos.y());
						else
							thiz->update(pos.x());
					}
					return true;
				}, Capture().set_thiz(this));
			}
		}

		void update(float v)
		{
			auto target_element = target_layout->element;
			if (type == ScrollbarVertical)
			{
				auto content_size = target_layout->content_size.y() + 20.f;
				if (target_element->size.y() > 0.f)
				{
					if (content_size > target_element->size.y())
						element->set_height(target_element->size.y() / content_size * scrollbar->element->size.y());
					else
						element->set_height(0.f);
				}
				else
					element->set_height(0.f);
				v += element->pos.y();
				element->set_y(element->size.y() > 0.f ? clamp(v, 0.f, scrollbar->element->size.y() - element->size.y()) : 0.f);
				target_layout->set_y_scroll_offset(-int(element->pos.y() / scrollbar->element->size.y() * content_size / step) * step);
			}
			else
			{
				auto content_size = target_layout->content_size.x() + 20.f;
				if (target_element->size.x() > 0.f)
				{
					if (content_size > target_element->size.x())
						element->set_width(target_element->size.x() / content_size * scrollbar->element->size.x());
					else
						element->set_width(0.f);
				}
				else
					element->set_width(0.f);
				v += element->pos.x();
				element->set_x(element->size.x() > 0.f ? clamp(v, 0.f, scrollbar->element->size.x() - element->size.x()) : 0.f);
				target_layout->set_x_scroll_offset(-int(element->pos.x() / scrollbar->element->size.x() * content_size / step) * step);
			}
			v = 0.f;
		}
	};

	void cScrollbarThumb::update(float v)
	{
		((cScrollbarThumbPrivate*)this)->update(v);
	}

	cScrollbarThumb* cScrollbarThumb::create(ScrollbarType type)
	{
		return new cScrollbarThumbPrivate(type);
	}
}
