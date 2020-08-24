#include "../entity_private.h"
#include "event_receiver_private.h"
#include "click_bring_to_front_private.h"

namespace flame
{
	void cClickBringToFrontPrivate::on_gain_event_receiver()
	{
		mouse_listener = event_receiver->add_mouse_left_down_listener([](Capture& c, const Vec2i& pos) {
			auto e = c.thiz<cClickBringToFrontPrivate>()->entity;
			auto parent = e->parent;
			if (parent && parent->children.size() > 1)
				parent->reposition_child(parent->children.size() - 1, e->index);
		}, Capture().set_thiz(this));
	}

	void cClickBringToFrontPrivate::on_lost_event_receiver()
	{
		event_receiver->remove_mouse_move_listener(mouse_listener);
	}

	cClickBringToFront* cClickBringToFront::create()
	{
		return f_new<cClickBringToFrontPrivate>();
	}
}
