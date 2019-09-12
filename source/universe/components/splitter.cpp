#include <flame/universe/components/element.h>
#include <flame/universe/components/event_receiver.h>
#include <flame/universe/components/aligner.h>
#include <flame/universe/components/splitter.h>

namespace flame
{
	struct cSplitterPrivate : cSplitter
	{
		void* mouse_listener;

		cSplitterPrivate()
		{
			element = nullptr;
			event_receiver = nullptr;

			type = SplitterHorizontal;

			mouse_listener = nullptr;
		}

		~cSplitterPrivate()
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

			mouse_listener = event_receiver->add_mouse_listener([](void* c, KeyState action, MouseKey key, const Vec2f& pos) {
				auto thiz = (*(cSplitterPrivate**)c);
				if (thiz->event_receiver->active && is_mouse_move(action, key))
				{
					auto parent = thiz->entity->parent();
					auto idx = parent->child_position(thiz->entity);
					if (idx > 0 && idx < parent->child_count() - 1)
					{
						auto left = parent->child(idx - 1);
						auto left_element = (cElement*)(left->find_component(cH("Element")));
						assert(left_element);
						auto left_aligner = (cAligner*)(left->find_component(cH("Aligner")));
						auto right = parent->child(idx + 1);
						auto right_element = (cElement*)(right->find_component(cH("Element")));
						assert(right_element);
						auto right_aligner = (cAligner*)(right->find_component(cH("Aligner")));

						if (thiz->type == SplitterHorizontal)
						{
							if (pos.x() < 0.f)
							{
								auto v = min(left_element->width - max(1.f, left_aligner ? left_aligner->min_width : left_element->inner_padding_horizontal()), -pos.x());
								left_element->width -= v;
								right_element->width += v;
							}
							else if (pos.x() > 0.f)
							{
								auto v = min(right_element->width - max(1.f, right_aligner ? right_aligner->min_width : right_element->inner_padding_horizontal()), pos.x());
								left_element->width += v;
								right_element->width -= v;
							}
							if (left_aligner)
								left_aligner->width_factor = left_element->width;
							if (right_aligner)
								right_aligner->width_factor = right_element->width;
						}
						else
						{
							if (pos.y() < 0.f)
							{
								auto v = min(left_element->height - max(1.f, left_aligner ? left_aligner->min_height : left_element->inner_padding_vertical()), -pos.y());
								left_element->height -= v;
								right_element->height += v;
							}
							else if (pos.y() > 0.f)
							{
								auto v = min(right_element->height - max(1.f, right_aligner ? right_aligner->min_height : right_element->inner_padding_vertical()), pos.y());
								left_element->height += v;
								right_element->height -= v;
							}
							if (left_aligner)
								left_aligner->height_factor = left_element->height;
							if (right_aligner)
								right_aligner->height_factor = right_element->height;
						}
					}
				}
			}, new_mail_p(this));
		}

		Component* copy()
		{
			auto copy = new cSplitterPrivate();

			copy->type = type;

			return copy;
		}
	};

	void cSplitter::start()
	{
		((cSplitterPrivate*)this)->start();
	}

	void cSplitter::update()
	{
	}

	Component* cSplitter::copy()
	{
		return ((cSplitterPrivate*)this)->copy();
	}

	cSplitter* cSplitter::create()
	{
		return new cSplitterPrivate;
	}
}
