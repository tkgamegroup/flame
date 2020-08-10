#include "../world_private.h"
#include "element_private.h"
#include "text_private.h"
#include "event_receiver_private.h"
#include "menu_private.h"

namespace flame
{
	static cMenuPrivate* curr_menu = nullptr;

	void cMenuPrivate::on_gain_event_receiver()
	{
		mouse_down_listener = event_receiver->add_mouse_left_down_listener([](Capture& c, const Vec2i& pos) {
			auto thiz = c.thiz<cMenuPrivate>();
			if (thiz->type == MenuTop && thiz->root && !thiz->opened && thiz->items && !curr_menu)
				thiz->open();
		}, Capture().set_thiz(this));
		mouse_move_listener = event_receiver->add_mouse_move_listener([](Capture& c, const Vec2i& disp, const Vec2i& pos) {
			auto thiz = c.thiz<cMenuPrivate>();
			if (thiz->root && !thiz->opened && thiz->items && curr_menu)
				thiz->open();
		}, Capture().set_thiz(this));
	}

	void cMenuPrivate::set_items(Entity* _e) 
	{
		auto e = (EntityPrivate*)_e;
		auto ce = (cElementPrivate*)e->get_component(cElement::type_hash);
		if (!ce)
		{
			printf("cannot set items of menu, items must contains a cElement component\n");
			return;
		}
		items.reset(e); 
		items_element = ce;
	}

	void cMenuPrivate::open()
	{
		if (opened)
			return;

		auto pos = element->get_point(type == MenuTop ? 3 : 1);
		items_element->set_x(pos.x());
		items_element->set_y(pos.y());
		root->add_child(items.get());

		auto parent = ((EntityPrivate*)entity)->parent;
		for (auto& e : parent->children)
		{
			auto cm = (cMenuPrivate*)e->get_component(cMenu::type_hash);
			if (cm)
				cm->close();
		}

		if (type == MenuTop)
		{
			root_mouse_listener = root_event_receiver
				->add_mouse_left_down_listener([](Capture& c, const Vec2i& pos) {
				auto thiz = c.thiz<cMenuPrivate>();
				if (thiz->frame >= looper().get_frame())
					return;
				thiz->root_event_receiver->remove_mouse_left_down_listener(thiz->root_mouse_listener);
				thiz->close();
				curr_menu = nullptr;
			}, Capture().set_thiz(this));
			curr_menu = this;
		}

		opened = true;
		frame = looper().get_frame();
	}

	void cMenuPrivate::close()
	{
		if (!opened)
			return;

		for (auto& e : items->children)
		{
			auto cm = (cMenuPrivate*)e->get_component(cMenu::type_hash);
			if (cm)
				cm->close();
		}

		if (type == MenuTop)
		{
			root_event_receiver->remove_mouse_left_down_listener(root_mouse_listener);
			curr_menu = nullptr;
		}

		root->remove_child(items.get(), false);
		opened = false;
	}

	void cMenuPrivate::on_lost_event_receiver()
	{
		event_receiver->remove_mouse_left_down_listener(mouse_down_listener);
		event_receiver->remove_mouse_move_listener(mouse_move_listener);
	}

	void cMenuPrivate::on_entity_entered_world()
	{
		root = ((EntityPrivate*)entity)->world->root.get();
		root_event_receiver = ((cEventReceiverPrivate*)root->get_component(cEventReceiver::type_hash));
	}

	void cMenuPrivate::on_entity_left_world()
	{
		root = nullptr;
		root_event_receiver = nullptr;
	}

	cMenu* cMenu::create()
	{
		return f_new<cMenuPrivate>();
	}
}
