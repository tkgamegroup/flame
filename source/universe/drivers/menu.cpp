#include "../entity_private.h"
#include "../components/element_private.h"
#include "../components/receiver_private.h"
#include "../world_private.h"
#include "menu_private.h"

namespace flame
{
	void dMenuPrivate::on_load_finished()
	{
		struct cSpy : Component
		{
			cSpy() :
				Component("cSpy", S<"cSpy"_h>)
			{
			}

			void on_entered_world() override
			{
				((dMenuPrivate*)entity->driver.get())->root = entity->world->root.get();
			}
		};

		element = entity->get_component_t<cElementPrivate>();
		fassert(element);

		receiver = entity->get_component_t<cReceiverPrivate>();
		fassert(receiver);

		items = entity->find_child("items");
		fassert(items);

		entity->add_component(f_new<cSpy>());

		receiver->add_mouse_left_down_listener([](Capture& c, const ivec2& pos) {
			auto thiz = c.thiz<dMenuPrivate>();
			thiz->open();
		}, Capture().set_thiz(this));

		//receiver->add_mouse_move_listener([](Capture& c, const ivec2& disp, const ivec2& pos) {
		//	auto thiz = c.thiz<dMenuPrivate>();
		//	if (thiz->root && !thiz->opened && curr_menu)
		//		thiz->open();
		//}, Capture().set_thiz(this));
	}

	bool dMenuPrivate::on_child_added(Entity* e)
	{
		if (load_finished)
		{
			items->add_child((EntityPrivate*)e);
			if (((EntityPrivate*)e)->driver && ((EntityPrivate*)e)->driver->type_hash == type_hash)
				((dMenuPrivate*)((EntityPrivate*)e)->driver.get())->type = MenuSub;
			return true;
		}
		return false;
	}

	void dMenuPrivate::open()
	{
		if (opened)
			return;

		//auto parent = entity->parent;
		//for (auto& e : parent->children)
		//{
		//	auto cm = e->get_component_t<cMenuPrivate>();
		//	if (cm)
		//		cm->close();
		//}

		auto items_element = items->get_component_t<cElementPrivate>();
		if (items_element)
		{
			element->update_transform();
			auto pos = element->points[(type == MenuTop) ? 3 : 1];
			items_element->set_x(pos.x);
			items_element->set_y(pos.y);
		}
		entity->remove_child(items, false);
		items->set_visible(true);
		root->add_child(items);

		//if (type != MenuSub)
		//{
		//	root_mouse_listener = root_receiver->add_mouse_left_down_listener([](Capture& c, const ivec2& pos) {
		//		auto thiz = c.thiz<cMenuPrivate>();
		//		if (thiz->frame >= looper().get_frame())
		//			return;
		//		thiz->root_receiver->remove_mouse_left_down_listener(thiz->root_mouse_listener);
		//		thiz->close();
		//		if (thiz->type == MenuTop)
		//			curr_menu = nullptr;
		//	}, Capture().set_thiz(this));
		//	if (type == MenuTop)
		//		curr_menu = this;
		//}

		//frame = looper().get_frame();

		opened = true;
	}

	void dMenuPrivate::close()
	{
		if (!opened)
			return;

		//for (auto& e : items->children)
		//{
		//	auto cm = e->get_component_t<cMenuPrivate>();
		//	if (cm)
		//		cm->close();
		//}

		//if (type != MenuSub)
		//{
		//	root_receiver->remove_mouse_left_down_listener(root_mouse_listener);
		//	if (type == MenuTop)
		//		curr_menu = nullptr;
		//}

		root->remove_child(items, false);
		items->set_visible(false);
		entity->add_child(items);

		opened = false;
	}

	dMenu* dMenu::create()
	{
		return f_new<dMenuPrivate>();
	}
}
