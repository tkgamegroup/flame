#include "event_receiver_private.h"
#include "list_private.h"

namespace flame
{
	//void cListPrivate::on_gain_event_receiver()
	//{
	//	//if (select_air_when_clicked)
	//	mouse_listener = event_receiver->mouse_listeners.add([](Capture& c, KeyStateFlags action, MouseKey key, const Vec2i& pos) {
	//		if (is_mouse_down(action, key, true) && key == Mouse_Left)
	//			c.thiz<cListPrivate>()->set_selected(nullptr);
	//		return true;
	//	}, Capture().set_thiz(this));
	//}

	//void cListPrivate::on_lost_event_receiver()
	//{
	//	event_receiver->mouse_listeners.remove(mouse_listener);
	//}

	//void cListPrivate::set_selected(Entity* e)
	//{
	//	if (selected == e)
	//		return;
	//	if (selected)
	//	{
	//		auto listitem = (cListItemPrivate*)selected->get_component(cListItem);
	//		if (listitem)
	//			listitem->do_style(false);
	//	}
	//	if (e)
	//	{
	//		auto listitem = (cListItemPrivate*)e->get_component(cListItem);
	//		if (listitem)
	//			listitem->do_style(true);
	//	}
	//	selected = e;
	//	report_data_changed(FLAME_CHASH("selected"), sender);
	//}

	//cList* cList::create()
	//{
	//	return f_new<cListPrivate>();
	//}

	//struct cListItemPrivate : cListItem
	//{
	//	cListItemPrivate()
	//	{
	//		event_receiver = nullptr;
	//		list = nullptr;

	//		mouse_listener = nullptr;
	//	}

	//	~cListItemPrivate()
	//	{
	//		if (!entity->dying_)
	//			event_receiver->mouse_listeners.remove(mouse_listener);
	//	}

	//	void on_event(EntityEvent e, void* t) override
	//	{
	//		switch (e)
	//		{
	//		case EntityComponentAdded:
	//			if (t == this)
	//			{
	//				event_receiver = entity->get_component(cEventReceiver);
	//				assert(event_receiver);

	//				mouse_listener = event_receiver->mouse_listeners.add([](Capture& c, KeyStateFlags action, MouseKey key, const Vec2i& pos) {
	//					if (is_mouse_down(action, key, true) && (key == Mouse_Left || key == Mouse_Right))
	//					{
	//						auto thiz = c.thiz<cListItemPrivate>();
	//						if (thiz->list)
	//							thiz->list->set_selected(thiz->entity);
	//					}
	//					return true;
	//				}, Capture().set_thiz(this));
	//			}
	//			break;
	//		}
	//	}
	//};

	//cListItem* cListItem::create()
	//{
	//	return f_new<cListItemPrivate>();
	//}
}
