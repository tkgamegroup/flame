#include <flame/universe/components/element.h>
#include <flame/universe/components/event_receiver.h>
#include <flame/universe/components/aligner.h>
#include <flame/universe/components/splitter.h>
#include <flame/universe/utils/event.h>

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
				mouse_listener = event_receiver->mouse_listeners.add([](void* c, KeyStateFlags action, MouseKey key, const Vec2i& pos) {
					auto thiz = (*(cSplitterPrivate**)c);
					if (utils::is_active(thiz->event_receiver) && is_mouse_move(action, key))
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
									auto v = min(left_element->size.x() - max(1.f, left_aligner ? left_aligner->min_width_ : left_element->padding_h()), (float)-pos.x());
									left_element->set_width(-v, true);
									right_element->set_width(v, true);
								}
								else if (pos.x() > 0.f)
								{
									auto v = min(right_element->size.x() - max(1.f, right_aligner ? right_aligner->min_width_ : right_element->padding_h()), (float)pos.x());
									left_element->set_width(v, true);
									right_element->set_width(-v, true);
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
									auto v = min(left_element->size.y() - max(1.f, left_aligner ? left_aligner->min_height_ : left_element->padding_v()), (float)-pos.y());
									left_element->set_height(-v, true);
									right_element->set_height(v, true);
								}
								else if (pos.y() > 0.f)
								{
									auto v = min(right_element->size.y() - max(1.f, right_aligner ? right_aligner->min_height_ : right_element->padding_v()), (float)pos.y());
									left_element->set_height(v, true);
									right_element->set_height(-v, true);
								}
								if (left_aligner)
									left_aligner->set_height_factor(left_element->size.y());
								if (right_aligner)
									right_aligner->set_height_factor(right_element->size.y());
							}
						}
					}
					return true;
				}, Mail::from_p(this));
				state_listener = event_receiver->state_listeners.add([](void* c, EventReceiverState s) {
					auto thiz = *(cSplitterPrivate**)c;
					sEventDispatcher::current()->window->set_cursor(s ? (thiz->type == SplitterHorizontal ? CursorSizeWE : CursorSizeNS) : CursorArrow);
					return true;
				}, Mail::from_p(this));
			}
		}
	};

	cSplitter* cSplitter::create(SplitterType type)
	{
		return new cSplitterPrivate(type);
	}
}
