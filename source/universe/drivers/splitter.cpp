#include "../entity_private.h"
#include "../components/element_private.h"
#include "../components/receiver_private.h"
#include "splitter_private.h"
#include "../systems/dispatcher_private.h"

namespace flame
{
	void dSplitterPrivate::on_load_finished()
	{
		struct cSpy : Component
		{
			dSplitterPrivate* thiz;

			cSpy(dSplitterPrivate* _thiz) :
				Component("cSpy", S<"cSpy"_h>)
			{
				thiz = _thiz;
			}

			void on_state_changed(StateFlags s) override
			{
				thiz->bar_receiver->dispatcher->window->set_cursor((s & StateHovering) != 0 ? (thiz->type == SplitterHorizontal ? CursorSizeWE : CursorSizeNS) : CursorArrow);
			}
		};

		element = entity->get_component_t<cElementPrivate>();
		fassert(element);

		bar = entity->find_child("bar");
		fassert(bar);
		bar_element = bar->get_component_t<cElementPrivate>();
		fassert(bar_element);
		bar_receiver = bar->get_component_t<cReceiverPrivate>();
		fassert(bar_receiver);

		switch (type)
		{
		case SplitterHorizontal:
			element->set_layout_type(LayoutHorizontal);
			bar_element->set_aligny(AlignMinMax);
			break;
		case SplitterVertical:
			element->set_layout_type(LayoutVertical);
			bar_element->set_alignx(AlignMinMax);
			break;
		}

		{
			auto p = bar->get_parent();
			auto idx = bar->index;
			p->remove_child(bar, false);
			bar->add_component(f_new<cSpy>(this));
			p->add_child(bar, idx);
		}

		bar_receiver->add_mouse_move_listener([](Capture& c, const ivec2& disp, const ivec2& pos) {
			auto thiz = c.thiz<dSplitterPrivate>();
			if (thiz->bar_receiver == thiz->bar_receiver->dispatcher->active)
			{
				if (thiz->targets.size() == 2)
				{
					switch (thiz->type)
					{
					case SplitterHorizontal:
						if (disp.x < 0.f)
						{
							auto v = min(thiz->targets[0]->size.x - max(1.f, thiz->targets[0]->padding_size[0]), (float)-disp.x);
							thiz->targets[0]->set_width(thiz->targets[0]->size.x - v);
							thiz->targets[1]->set_width(thiz->targets[1]->size.x + v);
						}
						else if (disp.x > 0.f)
						{
							auto v = min(thiz->targets[1]->size.x - max(1.f, thiz->targets[1]->padding_size[0]), (float)disp.x);
							thiz->targets[0]->set_width(thiz->targets[0]->size.x + v);
							thiz->targets[1]->set_width(thiz->targets[1]->size.x - v);
						}
						thiz->targets[0]->set_width_factor(thiz->targets[0]->size.x);
						thiz->targets[1]->set_width_factor(thiz->targets[1]->size.x);
						break;
					case SplitterVertical:
						if (disp.y < 0.f)
						{
							auto v = min(thiz->targets[0]->size.y - max(1.f, thiz->targets[0]->padding_size[1]), (float)-disp.y);
							thiz->targets[0]->set_height(thiz->targets[0]->size.y - v);
							thiz->targets[1]->set_height(thiz->targets[1]->size.y + v);
						}
						else if (disp.y > 0.f)
						{
							auto v = min(thiz->targets[1]->size.y - max(1.f, thiz->targets[1]->padding_size[1]), (float)disp.y);
							thiz->targets[0]->set_height(thiz->targets[0]->size.y + v);
							thiz->targets[1]->set_height(thiz->targets[1]->size.y - v);
						}
						thiz->targets[0]->set_height_factor(thiz->targets[0]->size.y);
						thiz->targets[1]->set_height_factor(thiz->targets[1]->size.y);
						break;
					}
				}
			}
		}, Capture().set_thiz(this));
	}

	bool dSplitterPrivate::on_child_added(Entity* _e)
	{
		if (load_finished)
		{
			if (targets.size() < 2)
			{
				auto e = (EntityPrivate*)_e;
				auto element = e->get_component_t<cElementPrivate>();
				fassert(element);
				entity->add_child(e, targets.size() == 0 ? 0 : 2);
				targets.push_back(element);
				return true;
			}
			return false;
		}
		return false;
	}

	dSplitter* dSplitter::create()
	{
		return f_new<dSplitterPrivate>();
	}
}
