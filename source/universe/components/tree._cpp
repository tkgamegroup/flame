#include "../entity_private.h"
#include "receiver_private.h"
#include "tree_leaf_private.h"
#include "tree_node_private.h"
#include "tree_private.h"

namespace flame
{
	void populate_tree(cTreePrivate* t, EntityPrivate* e)
	{
		auto ctn = e->get_component_t<cTreeNodePrivate>();
		if (ctn)
		{
			ctn->tree = t;
			if (ctn->load_finished)
			{
				for (auto& e : ctn->items->children)
					populate_tree(t, e.get());
			}
		}
		else
		{
			auto ctl = e->get_component_t<cTreeLeafPrivate>();
			if (ctl)
				ctl->tree = t;
		}
	}

	void cTreePrivate::set_selected(EntityPtr e)
	{
		if (selected == e)
			return;
		if (selected)
		{
			auto dtn = selected->get_component_t<cTreeNodePrivate>();
			if (dtn)
				dtn->notify_selected(false);
			else
			{
				auto dtl = selected->get_component_t<cTreeLeafPrivate>();
				if (dtl)
					dtl->notify_selected(false);
			}
		}
		if (e)
		{
			auto dtn = e->get_component_t<cTreeNodePrivate>();
			if (dtn)
				dtn->notify_selected(true);
			else
			{
				auto dtl = e->get_component_t<cTreeLeafPrivate>();
				if (dtl)
					dtl->notify_selected(true);
			}
		}
		selected = (EntityPrivate*)e;
		data_changed(S<"selected"_h>);
	}

	static void expand(EntityPrivate* e)
	{
		auto ct = e->get_component_t<cTreePrivate>();
		if (ct)
			return;
		auto ctn = e->get_component_t<cTreeNodePrivate>();
		if (ctn)
		{
			if (!ctn->items->visible)
				ctn->toggle_collapse();
		}
		expand(e->parent);
	}

	void cTreePrivate::expand_to_selected()
	{
		if (!selected)
			return;
		expand(selected);

		add_event([this]() {
			if (!selected)
				return false;
			//	auto parent = thiz->entity->parent;
			//	if (!parent || parent->children.s < 2)
			//		return;
			//	auto e_scrollbar = parent->children[1];
			//	auto c_scrollbar = e_scrollbar->get_component(cScrollbar);
			//	if (!c_scrollbar)
			//		return;
			//	auto e_thumb = e_scrollbar->children[0];
			//	auto e_tree = thiz->entity;
			//	auto c_tree_layout = e_tree->get_component(cLayout);
			//	e_thumb->get_component(cElement)->set_y(e_scrollbar->get_component(cElement)->size.y *
			//		(selected->get_component(cElement)->global_pos.y - e_tree->get_component(cElement)->global_pos.y - 
			// c_tree_layout->scroll_offset.y) / (c_tree_layout->content_size.y + 20.f));
			//	e_thumb->get_component(cScrollbarThumb)->update(0.f);
		});
	}

	void cTreePrivate::on_load_finished()
	{
		auto receiver = entity->get_component_t<cReceiverPrivate>();
		assert(receiver);

		receiver->add_mouse_left_down_listener([](Capture& c, const ivec2& pos) {
			c.thiz<cTreePrivate>()->set_selected(nullptr);
		}, Capture().set_thiz(this));
	}

	bool cTreePrivate::on_before_adding_child(EntityPtr e)
	{
		if (load_finished)
			populate_tree(this, e);
		return false;
	}

	cTree* cTree::create(void* parms)
	{
		return new cTreePrivate();
	}
}
