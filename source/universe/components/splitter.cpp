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
			left_element = nullptr;
			left_aligner = nullptr;
			right_element = nullptr;
			right_aligner = nullptr;

			mouse_listener = nullptr;
		}

		~cSplitterPrivate()
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
			auto pos = parent->child_position(entity);
			if (pos > 0 && pos < parent->child_count() - 1)
			{
				auto left = parent->child(pos - 1);
				left_element = (cElement*)(left->find_component(cH("Element")));
				assert(left_element);
				left_aligner = (cAligner*)(left->find_component(cH("Aligner")));
				auto right = parent->child(pos + 1);
				right_element = (cElement*)(right->find_component(cH("Element")));
				assert(right_element);
				right_aligner = (cAligner*)(right->find_component(cH("Aligner")));
			}

			mouse_listener = event_receiver->add_mouse_listener([](void* c, KeyState action, MouseKey key, const Vec2f& pos) {
				auto thiz = (*(cSplitterPrivate**)c);
				if (thiz->event_receiver->dragging && is_mouse_move(action, key))
				{
					if (thiz->type == SplitterHorizontal)
					{
						if (pos.x() < 0.f)
						{
							auto v = min(thiz->left_element->width - max(1.f, thiz->left_aligner ? thiz->left_aligner->min_width : thiz->left_element->inner_padding_horizontal()), -pos.x());
							thiz->left_element->width -= v;
							thiz->right_element->width += v;
						}
						else if (pos.x() > 0.f)
						{
							auto v = min(thiz->right_element->width - max(1.f, thiz->right_aligner ? thiz->right_aligner->min_width : thiz->right_element->inner_padding_horizontal()), pos.x());
							thiz->left_element->width += v;
							thiz->right_element->width -= v;
						}
					}
					else
					{
						if (pos.y() < 0.f)
						{
							auto v = min(thiz->left_element->height - max(1.f, thiz->left_aligner ? thiz->left_aligner->min_height : thiz->left_element->inner_padding_vertical()), -pos.y());
							thiz->left_element->height -= v;
							thiz->right_element->height += v;
						}
						else if (pos.y() > 0.f)
						{
							auto v = min(thiz->right_element->height - max(1.f, thiz->right_aligner ? thiz->right_aligner->min_height : thiz->right_element->inner_padding_vertical()), pos.y());
							thiz->left_element->height += v;
							thiz->right_element->height -= v;
						}
					}
				}
			}, new_mail_p(this));
		}

		void update()
		{
			if (type == SplitterHorizontal)
			{
				auto r = left_element->width / right_element->width;
				if (left_aligner)
					left_aligner->width_factor = r;
				if (right_aligner)
					right_aligner->width_factor = 1 / r;
			}
			else
			{
				auto r = left_element->height / right_element->height;
				if (left_aligner)
					left_aligner->height_factor = r;
				if (right_aligner)
					right_aligner->height_factor = 1 / r;
			}
		}
	};

	cSplitter::~cSplitter()
	{
		((cSplitterPrivate*)this)->~cSplitterPrivate();
	}

	void cSplitter::start()
	{
		((cSplitterPrivate*)this)->start();
	}

	void cSplitter::update()
	{
		((cSplitterPrivate*)this)->update();
	}

	cSplitter* cSplitter::create()
	{
		return new cSplitterPrivate;
	}
}
