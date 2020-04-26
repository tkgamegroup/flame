#include <flame/universe/components/element.h>
#include <flame/universe/components/event_receiver.h>
#include <flame/universe/components/aligner.h>
#include <flame/universe/components/splitter.h>
#include <flame/universe/components/style.h>

namespace flame
{
	struct cSplitterPrivate : cSplitter
	{
		void* mouse_listener;
		void* state_listener;

		cSplitterPrivate(SplitterType _type)
		{
			event_receiver = nullptr;

			type = _type;

			mouse_listener = nullptr;
			state_listener = nullptr;
		}

		~cSplitterPrivate()
		{
			if (!entity->dying_)
			{
				event_receiver->mouse_listeners.remove(mouse_listener);
				event_receiver->state_listeners.remove(state_listener);
			}
		}

		void on_component_added(Component* c) override
		{
			if (c->name_hash == FLAME_CHASH("cEventReceiver"))
			{
				event_receiver = (cEventReceiver*)c;
				mouse_listener = event_receiver->mouse_listeners.add([](Capture& c, KeyStateFlags action, MouseKey key, const Vec2i& pos) {
					auto thiz = c.thiz<cSplitterPrivate>();
					if (thiz->event_receiver->is_active() && is_mouse_move(action, key))
					{
						auto parent = thiz->entity->parent();
						auto idx = thiz->entity->index_;
						if (idx > 0 && idx < parent->child_count() - 1)
						{
							auto left = parent->child(idx - 1);
							auto left_element = left->get_component(cElement);
							assert(left_element);
							auto left_aligner = left->get_component(cAligner);
							auto right = parent->child(idx + 1);
							auto right_element = right->get_component(cElement);
							assert(right_element);
							auto right_aligner = right->get_component(cAligner);

							if (thiz->type == SplitterHorizontal)
							{
								if (pos.x() < 0.f)
								{
									auto v = min(left_element->size.x() - max(1.f, left_aligner ? left_aligner->min_width : left_element->padding.xz().sum()), (float)-pos.x());
									left_element->add_width(-v);
									right_element->add_width(v);
								}
								else if (pos.x() > 0.f)
								{
									auto v = min(right_element->size.x() - max(1.f, right_aligner ? right_aligner->min_width : right_element->padding.xz().sum()), (float)pos.x());
									left_element->add_width(v);
									right_element->add_width(-v);
								}
								if (left_aligner)
									left_aligner->set_width_factor(left_element->size.x());
								if (right_aligner)
									right_aligner->set_width_factor(right_element->size.x());
							}
							else
							{
								if (pos.y() < 0.f)
								{
									auto v = min(left_element->size.y() - max(1.f, left_aligner ? left_aligner->min_height : left_element->padding.yw().sum()), (float)-pos.y());
									left_element->add_height(-v);
									right_element->add_height(v);
								}
								else if (pos.y() > 0.f)
								{
									auto v = min(right_element->size.y() - max(1.f, right_aligner ? right_aligner->min_height : right_element->padding.yw().sum()), (float)pos.y());
									left_element->add_height(v);
									right_element->add_height(-v);
								}
								if (left_aligner)
									left_aligner->set_height_factor(left_element->size.y());
								if (right_aligner)
									right_aligner->set_height_factor(right_element->size.y());
							}
						}
					}
					return true;
				}, Capture().set_thiz(this));
				state_listener = event_receiver->state_listeners.add([](Capture& c, EventReceiverState s) {
					auto thiz = c.thiz<cSplitterPrivate>();
					c.current<cEventReceiver>()->dispatcher->window->set_cursor(s ? (thiz->type == SplitterHorizontal ? CursorSizeWE : CursorSizeNS) : CursorArrow);
					return true;
				}, Capture().set_thiz(this));
			}
		}
	};

	cSplitter* cSplitter::create(SplitterType type)
	{
		return new cSplitterPrivate(type);
	}

	void cSplitter::make(Entity* e, SplitterType type)
	{
		auto ce = cElement::create();
		ce->size = 8.f;
		e->add_component(ce);
		e->add_component(cEventReceiver::create());
		auto cs = cStyleColor::create();
		cs->color_normal = Vec4c(0);
		cs->color_hovering = get_style(FrameColorHovering).c;
		cs->color_active = get_style(FrameColorActive).c;
		e->add_component(cs);
		e->add_component(cSplitter::create(type));
		auto ca = cAligner::create();
		if (type == SplitterHorizontal)
			ca->y_align_flags = AlignMinMax;
		else
			ca->x_align_flags = AlignMinMax;
		e->add_component(ca);
	}
}
