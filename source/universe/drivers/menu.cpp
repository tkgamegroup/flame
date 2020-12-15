#include "../entity_private.h"
#include "../components/element_private.h"
#include "../components/receiver_private.h"
#include "../components/text_private.h"
#include "../world_private.h"
#include "menu_private.h"

namespace flame
{
	void dMenuPrivate::on_load_finished()
	{
		struct cSpy : Component
		{
			dMenuPrivate* thiz;

			cSpy() :
				Component("cSpy", S<"cSpy"_h>)
			{
			}

			void on_entered_world() override
			{
				thiz->root = entity->world->root.get();
			}
		};

		element = entity->get_component_t<cElementPrivate>();
		fassert(element);

		receiver = entity->get_component_t<cReceiverPrivate>();
		fassert(receiver);

		text = entity->get_component_t<cTextPrivate>();
		fassert(text);

		items = entity->find_child("items");
		fassert(items);

		auto c_spy = f_new<cSpy>();
		c_spy->thiz = this;
		entity->add_component(c_spy);

		receiver->add_mouse_left_down_listener([](Capture& c, const ivec2& pos) {
			auto thiz = c.thiz<dMenuPrivate>();
			thiz->open();
		}, Capture().set_thiz(this));

		receiver->add_mouse_move_listener([](Capture& c, const ivec2& disp, const ivec2& pos) {
			auto thiz = c.thiz<dMenuPrivate>();
			if (!thiz->first)
				thiz->open();
		}, Capture().set_thiz(this));
	}

	bool dMenuPrivate::on_child_added(Entity* _e)
	{
		if (load_finished)
		{
			auto e = (EntityPrivate*)_e;
			items->add_child(e);
			auto dm = e->get_driver_t<dMenuPrivate>();
			if (dm)
			{
				dm->type = MenuSub;
				dm->element->padding.z += text->font_size;
				dm->entity->find_child("arrow")->set_visible(true);
			}
			return true;
		}
		return false;
	}

	void dMenuPrivate::open()
	{
		if (opened)
			return;

		if (entity->parent)
		{
			for (auto& e : entity->parent->children)
			{
				auto dm = e->get_driver_t<dMenuPrivate>();
				if (dm)
					dm->close();
			}
		}

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

		if (first)
		{
			root_mouse_listener = root->get_component_t<cReceiver>()->add_mouse_left_down_listener([](Capture& c, const ivec2& pos) {
				c.thiz<dMenuPrivate>()->close();
			}, Capture().set_thiz(this));
		}

		for (auto& e : items->children)
		{
			auto dm = e->get_driver_t<dMenuPrivate>();
			if (dm)
				dm->first = false;
		}

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

		first = true;
		if (root_mouse_listener)
		{
			root->get_component_t<cReceiver>()->remove_mouse_left_down_listener(root_mouse_listener);
			root_mouse_listener = nullptr;
		}

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
