#include "../../foundation/window.h"
#include "../entity_private.h"
#include "element_private.h"
#include "receiver_private.h"
#include "splitter_private.h"
#include "../systems/dispatcher_private.h"

namespace flame
{
	void cSplitterPrivate::set_type(SplitterType _type)
	{
		type = _type;
		if (load_finished)
		{
			switch (type)
			{
			case SplitterHorizontal:
				element->set_layout_type(LayoutHorizontal);
				bar_element->set_alignx(AlignNone);
				bar_element->set_aligny(AlignMinMax);
				break;
			case SplitterVertical:
				element->set_layout_type(LayoutVertical);
				bar_element->set_alignx(AlignMinMax);
				bar_element->set_aligny(AlignNone);
				break;
			}
		}
	}

	void cSplitterPrivate::on_load_finished()
	{
		element = entity->get_component_i<cElementPrivate>(0);
		assert(element);

		bar = entity->find_child("bar");
		assert(bar);
		bar_element = bar->get_component_i<cElementPrivate>(0);
		assert(bar_element);
		bar_receiver = bar->get_component_t<cReceiverPrivate>();
		assert(bar_receiver);

		set_type(type);

		bar->add_message_listener([](Capture& c, uint msg, void* parm1, void* parm2) {
			auto thiz = c.thiz<cSplitterPrivate>();
			switch (msg)
			{
			case S<"state_changed"_h>:
				thiz->bar_receiver->dispatcher->window->set_cursor(
					((int)parm1 & StateHovering) != 0 ? (thiz->type == SplitterHorizontal ? CursorSizeWE : CursorSizeNS) : CursorArrow);
				break;
			}
			}, Capture().set_thiz(this));

		bar_receiver->add_mouse_move_listener([](Capture& c, const ivec2& disp, const ivec2& pos) {
			auto thiz = c.thiz<cSplitterPrivate>();
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

	bool cSplitterPrivate::on_before_adding_child(EntityPtr e)
	{
		if (load_finished)
		{
			if (targets.size() < 2)
			{
				auto element = e->get_component_i<cElementPrivate>(0);
				assert(element);
				entity->add_child(e, targets.size() == 0 ? 0 : 2);
				targets.push_back(element);
				return true;
			}
		}
		return false;
	}

	cSplitter* cSplitter::create(void* parms)
	{
		return new cSplitterPrivate();
	}
}
