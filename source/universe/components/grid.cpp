#include "../entity_private.h"
#include "element_private.h"
#include "receiver_private.h"
#include "grid_private.h"
#include "../systems/dispatcher_private.h"

namespace flame
{
	void cGridPrivate::on_load_finished()
	{
		element = entity->get_component_i<cElementPrivate>(0);
		assert(element);

		receiver = entity->get_component_t<cReceiverPrivate>();
		assert(receiver);

		anchor = entity->find_child("anchor");
		assert(anchor);

		anchor_element = anchor->get_component_i<cElementPrivate>(0);
		assert(anchor_element);

		receiver->add_mouse_move_listener([](Capture& c, const ivec2& disp, const ivec2&) {
			auto thiz = c.thiz<cGridPrivate>();
			if (thiz->receiver->is_active())
				thiz->anchor_element->add_pos(vec2(disp) / thiz->anchor_element->scl);
		}, Capture().set_thiz(this));

		receiver->add_mouse_scroll_listener([](Capture& c, int scroll) {
			auto thiz = c.thiz<cGridPrivate>();
			auto mp = vec2(thiz->receiver->dispatcher->mpos);
			auto p = (mp - vec2(thiz->anchor_element->pos)) / thiz->anchor_element->scl;
			thiz->anchor_element->add_pos(p * (scroll > 0 ? -0.1f : 0.1f));
			if (scroll < 0)
				thiz->anchor_element->set_scale(max(vec2(0.1f), thiz->anchor_element->scl - 0.1f));
			else
				thiz->anchor_element->set_scale(min(vec2(4.f), thiz->anchor_element->scl + 0.1f));
		}, Capture().set_thiz(this));
	}

	bool cGridPrivate::on_before_adding_child(EntityPtr e)
	{
		if (load_finished)
		{
			anchor->add_child(e);
			return true;
		}
		return false;
	}


	cGrid* cGrid::create(void* parms)
	{
		return new cGridPrivate();
	}
}
