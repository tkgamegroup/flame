#include "../entity_private.h"
#include "event_receiver_private.h"
#include "list_private.h"

namespace flame
{
	void cListPrivate::on_gain_event_receiver()
	{
		mouse_listener = event_receiver->add_mouse_left_down_listener([](Capture& c, const Vec2i& pos) {
			auto thiz = c.thiz<cListPrivate>();
			if (thiz->select_air_when_clicked)
				thiz->set_selected(nullptr);
		}, Capture().set_thiz(this));
	}

	void cListPrivate::on_lost_event_receiver()
	{
		event_receiver->remove_mouse_left_down_listener(mouse_listener);
	}

	void cListPrivate::set_selected(Entity* e)
	{
		if (selected == e)
			return;
		if (selected)
			selected->set_state((StateFlags)(((EntityPrivate*)selected)->state & (~StateSelected)));
		if (e)
			e->set_state((StateFlags)(((EntityPrivate*)e)->state | StateSelected));
		selected = e;
		Entity::report_data_changed(this, S<ch("selected")>::v);
	}

	cList* cList::create()
	{
		return f_new<cListPrivate>();
	}

	void cListItemPrivate::on_gain_event_receiver()
	{
		mouse_listener = event_receiver->add_mouse_left_down_listener([](Capture& c, const Vec2i& pos) {
			auto thiz = c.thiz<cListItemPrivate>();
			if (thiz->list)
				thiz->list->set_selected(thiz->entity);
		}, Capture().set_thiz(this));
	}

	void cListItemPrivate::on_lost_event_receiver()
	{
		event_receiver->remove_mouse_left_down_listener(mouse_listener);
	}

	cListItem* cListItem::create()
	{
		return f_new<cListItemPrivate>();
	}
}
