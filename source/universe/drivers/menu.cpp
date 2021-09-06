#include "../entity_private.h"
#include "../components/element_private.h"
#include "../components/receiver_private.h"
#include "../components/text_private.h"
#include "../world_private.h"
#include "menu_private.h"

namespace flame
{
	static std::vector<dMenuPrivate*> root_menus;
	static void* root_mouse_listener = nullptr;

	void dMenuPrivate::on_load_finished()
	{
		element = entity->get_component_i<cElementPrivate>(0);
		fassert(element);

		receiver = entity->get_component_t<cReceiverPrivate>();
		fassert(receiver);

		text = entity->get_component_t<cTextPrivate>();
		fassert(text);

		arrow = entity->find_child("arrow");

		items = entity->find_child("items");
		fassert(items);

		entity->add_message_listener([](Capture& c, uint msg, void* parm1, void* parm2) {
			auto thiz = c.thiz<dMenuPrivate>();
			switch (msg)
			{
			case S<"entered_world"_h>:
				thiz->root = thiz->entity->world->root.get();
				thiz->root_receiver = thiz->root->get_component_t<cReceiverPrivate>();
				break;
			}
		}, Capture().set_thiz(this));

		receiver->add_mouse_left_down_listener([](Capture& c, const ivec2& pos) {
			auto thiz = c.thiz<dMenuPrivate>();

			if (thiz->opened)
				return;

			auto p = thiz->entity->parent;
			if (p && p->get_driver_t<dMenuBarPrivate>())
			{
				for (auto& e : p->children)
				{
					auto dm = e->get_driver_t<dMenuPrivate>();
					if (dm)
					{
						root_menus.push_back(dm);
						dm->mark_ancestor_opened(true);
					}
				}
			}
			else
			{
				root_menus.push_back(thiz);
				thiz->mark_ancestor_opened(true);
			}

			thiz->open();

			root_mouse_listener = thiz->root_receiver->add_mouse_left_down_listener([](Capture& c, const ivec2& pos) {
				auto thiz = c.thiz<dMenuPrivate>();

				for (auto m : root_menus)
				{
					m->close();
					m->mark_ancestor_opened(false);
				}

				thiz->root_receiver->remove_mouse_left_down_listener(root_mouse_listener);
				root_mouse_listener = nullptr;
			}, Capture().set_thiz(thiz));
		}, Capture().set_thiz(this));

		receiver->add_mouse_move_listener([](Capture& c, const ivec2& disp, const ivec2& pos) {
			auto thiz = c.thiz<dMenuPrivate>();
			if (thiz->ancestor_opened)
				thiz->open();
		}, Capture().set_thiz(this));
	}

	bool dMenuPrivate::on_child_added(EntityPtr e, uint& pos)
	{
		if (load_finished)
		{
			if (e->name != "items")
			{
				auto element = e->get_component_i<cElementPrivate>(0);
				fassert(element);
				element->set_alignx(AlignMinMax);
				auto dm = e->get_driver_t<dMenuPrivate>();
				if (dm)
				{
					dm->type = MenuSub;
					dm->element->add_padding(vec4(0.f, 0.f, text->font_size, 0.f));
					dm->arrow->set_visible(true);
				}
				items->add_child(e);
				return true;
			}
		}
		return false;
	}

	void dMenuPrivate::open()
	{
		if (opened)
			return;

		auto p = entity->parent;
		if (p)
		{
			for (auto& e : p->children)
			{
				auto dm = e->get_driver_t<dMenuPrivate>();
				if (dm)
					dm->close();
			}
		}

		element->update_transform();
		items->get_component_i<cElementPrivate>(0)->set_pos(element->points[(type == MenuTop) ? 3 : 1]);
		entity->remove_child(items, false);
		items->set_visible(true);
		root->add_child(items);

		opened = true;
	}

	void dMenuPrivate::close()
	{
		if (!opened)
			return;

		for (auto& e : items->children)
		{
			auto dm = e->get_driver_t<dMenuPrivate>();
			if (dm)
				dm->close();
		}

		root->remove_child(items, false);
		items->set_visible(false);
		entity->add_child(items);

		opened = false;
	}

	void dMenuPrivate::mark_ancestor_opened(bool v)
	{
		ancestor_opened = v;
		for (auto& e : items->children)
		{
			auto dm = e->get_driver_t<dMenuPrivate>();
			if (dm)
				dm->mark_ancestor_opened(v);
		}
	}

	dMenu* dMenu::create(void* parms)
	{
		return new dMenuPrivate();
	}

	void dMenuItemPrivate::set_checkable(bool v)
	{
		checkable = v;
	}

	void dMenuItemPrivate::set_checked(bool v)
	{
		if (!checkable)
			return;
		if (checked == v)
			return;
		checked = v;
		if (arrow)
			arrow->set_visible(checked);
	}

	void dMenuItemPrivate::set_radio_checked()
	{
		set_checked(true);
		for (auto& c : entity->parent->children)
		{
			if (c.get() == entity)
				continue;
			auto dmi = c->get_driver_t<dMenuItemPrivate>();
			if (dmi)
				dmi->set_checked(false);
		}
	}

	void dMenuItemPrivate::on_load_finished()
	{
		element = entity->get_component_i<cElementPrivate>(0);
		fassert(element);

		receiver = entity->get_component_t<cReceiverPrivate>();
		fassert(receiver);

		text = entity->get_component_t<cTextPrivate>();
		fassert(text);

		arrow = entity->find_child("arrow");

		receiver->add_mouse_move_listener([](Capture& c, const ivec2& disp, const ivec2& pos) {
			auto thiz = c.thiz<dMenuItemPrivate>();
			auto p = thiz->entity->parent;
			if (p)
			{
				for (auto& e : p->children)
				{
					auto dm = e->get_driver_t<dMenuPrivate>();
					if (dm)
						dm->close();
				}
			}
		}, Capture().set_thiz(this));

		element->add_padding(vec4(text->font_size, 0.f, 0.f, 0.f));
	}

	dMenuItem* dMenuItem::create(void* parms)
	{
		return new dMenuItemPrivate();
	}

	dMenuBar* dMenuBar::create(void* parms)
	{
		return new dMenuBarPrivate();
	}
}
