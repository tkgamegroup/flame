#include "../entity_private.h"
#include "../world_private.h"
#include "element_private.h"
#include "text_private.h"
#include "receiver_private.h"
#include "menu_bar_private.h"
#include "menu_private.h"

namespace flame
{
	static std::vector<cMenuPrivate*> root_menus;
	static void* root_mouse_listener = nullptr;

	void cMenuPrivate::on_entered_world()
	{
		root = entity->world->root.get();
		root_receiver = root->get_component_t<cReceiverPrivate>();
	}
	
	void cMenuPrivate::on_load_finished()
	{
		element = entity->get_component_i<cElementPrivate>(0);
		assert(element);
		
		receiver = entity->get_component_t<cReceiverPrivate>();
		assert(receiver);
		
		text = entity->get_component_t<cTextPrivate>();
		assert(text);
		
		arrow = entity->find_child("arrow");
		
		items = entity->find_child("items");
		assert(items);
		
		receiver->add_mouse_left_down_listener([](Capture& c, const ivec2& pos) {
		auto thiz = c.thiz<cMenuPrivate>();
		
		if (thiz->opened)
		return;
		
		auto p = thiz->entity->parent;
		if (p && p->get_component_t<cMenuBarPrivate>())
		{
			for (auto& e : p->children)
			{
				auto cm = e->get_component_t<cMenuPrivate>();
				if (cm)
				{
					root_menus.push_back(cm);
					cm->mark_ancestor_opened(true);
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
		auto thiz = c.thiz<cMenuPrivate>();
		
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
		auto thiz = c.thiz<cMenuPrivate>();
		if (thiz->ancestor_opened)
		thiz->open();
		}, Capture().set_thiz(this));
	}
	
	bool cMenuPrivate::on_before_adding_child(EntityPtr e)
	{
		if (load_finished)
		{
			if (e->name != "items")
			{
				auto element = e->get_component_i<cElementPrivate>(0);
				assert(element);
				element->set_alignx(AlignMinMax);
				auto cm = e->get_component_t<cMenuPrivate>();
				if (cm)
				{
					cm->type = MenuSub;
					cm->element->add_padding(vec4(0.f, 0.f, text->font_size, 0.f));
					cm->arrow->set_visible(true);
				}
				items->add_child(e);
				return true;
			}
		}
		return false;
	}
	
	void cMenuPrivate::open()
	{
		if (opened)
		return;
		
		auto p = entity->parent;
		if (p)
		{
			for (auto& e : p->children)
			{
				auto cm = e->get_component_t<cMenuPrivate>();
				if (cm)
				cm->close();
			}
		}
		
		element->update_transform();
		items->get_component_i<cElementPrivate>(0)->set_pos(element->points[(type == MenuTop) ? 3 : 1]);
		entity->remove_child(items, false);
		items->set_visible(true);
		root->add_child(items);
		
		opened = true;
	}
	
	void cMenuPrivate::close()
	{
		if (!opened)
		return;
		
		for (auto& e : items->children)
		{
			auto cm = e->get_component_t<cMenuPrivate>();
			if (cm)
			cm->close();
		}
		
		root->remove_child(items, false);
		items->set_visible(false);
		entity->add_child(items);
		
		opened = false;
	}
	
	void cMenuPrivate::mark_ancestor_opened(bool v)
	{
		ancestor_opened = v;
		for (auto& e : items->children)
		{
			auto cm = e->get_component_t<cMenuPrivate>();
			if (cm)
			cm->mark_ancestor_opened(v);
		}
	}
	
	cMenu* cMenu::create(void* parms)
	{
		return new cMenuPrivate();
	}
}
