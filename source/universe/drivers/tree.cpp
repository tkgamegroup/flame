#include <flame/graphics/font.h>
#include "../entity_private.h"
#include "../components/element_private.h"
#include "../components/text_private.h"
#include "../components/receiver_private.h"
#include "tree_private.h"

namespace flame
{
	static void populate_tree(dTreePrivate* t, EntityPrivate* e)
	{
		auto dtn = e->get_driver_t<dTreeNodePrivate>();
		if (dtn)
		{
			dtn->tree = t;
			if (dtn->load_finished)
			{
				for (auto& e : dtn->items->children)
					populate_tree(t, e.get());
			}
		}
		else
		{
			auto dtl = e->get_driver_t<dTreeLeafPrivate>();
			if (dtl)
				dtl->tree = t;
		}
	}

	void dTreePrivate::set_selected(Entity* _e)
	{
		auto e = (EntityPrivate*)_e;
		if (selected == e)
			return;
		if (selected)
		{
			auto dtn = selected->get_driver_t<dTreeNodePrivate>();
			if (dtn)
				dtn->notify_selected(false);
			else
			{
				auto dtl = selected->get_driver_t<dTreeLeafPrivate>();
				if (dtl)
					dtl->notify_selected(false);
			}
		}
		if (e)
		{
			auto dtn = e->get_driver_t<dTreeNodePrivate>();
			if (dtn)
				dtn->notify_selected(true);
			else
			{
				auto dtl = e->get_driver_t<dTreeLeafPrivate>();
				if (dtl)
					dtl->notify_selected(true);
			}
		}
		selected = (EntityPrivate*)e;
		entity->driver_data_changed(this, S<"selected"_h>);
	}

	static void expand(EntityPrivate* e)
	{
		auto dt = e->get_driver_t<dTreePrivate>();
		if (dt)
			return;
		auto dtn = e->get_driver_t<dTreeNodePrivate>();
		if (dtn)
		{
			if (!dtn->items->visible)
				dtn->toggle_collapse();
		}
		expand(e->parent);
	}

	void dTreePrivate::expand_to_selected()
	{
		if (!selected)
			return;
		expand(selected);

		looper().add_event([](Capture& c) {
			auto thiz = c.thiz<dTreePrivate>();
			auto selected = thiz->selected;
			if (!selected)
				return;
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
		//		(selected->get_component(cElement)->global_pos.y - e_tree->get_component(cElement)->global_pos.y - c_tree_layout->scroll_offset.y) / (c_tree_layout->content_size.y + 20.f));
		//	e_thumb->get_component(cScrollbarThumb)->update(0.f);
		}, Capture().set_thiz(this), 1U);
	}

	bool dTreePrivate::on_child_added(Entity* e)
	{
		if (load_finished)
			populate_tree(this, (EntityPrivate*)e);
		return false;
	}

	void dTreePrivate::on_load_finished()
	{
		auto receiver = entity->get_component_t<cReceiverPrivate>();
		fassert(receiver);

		receiver->add_mouse_left_down_listener([](Capture& c, const ivec2& pos) {
			c.thiz<dTreePrivate>()->set_selected(nullptr);
		}, Capture().set_thiz(this));
	}

	dTree* dTree::create()
	{
		return f_new<dTreePrivate>();
	}

	void dTreeLeafPrivate::set_title(const wchar_t* _title)
	{
		title = _title;
		if (load_finished)
			title_text->set_text(title.c_str());
	}

	void dTreeLeafPrivate::notify_selected(bool v)
	{
		entity->set_state(v ? (StateFlags)(entity->state | StateSelected) : (StateFlags)(entity->state & (~StateSelected)));
	}

	void dTreeLeafPrivate::on_load_finished()
	{
		title_text = entity->get_component_t<cTextPrivate>();
		fassert(title_text);
		auto receiver = entity->get_component_t<cReceiverPrivate>();
		fassert(receiver);

		receiver->add_mouse_left_down_listener([](Capture& c, const ivec2& pos) {
			auto thiz = c.thiz<dTreeLeafPrivate>();
			thiz->tree->set_selected(thiz->entity);
		}, Capture().set_thiz(this));
	}

	dTreeLeaf* dTreeLeaf::create()
	{
		return f_new<dTreeLeafPrivate>();
	}

	void dTreeNodePrivate::set_title(const wchar_t* _title)
	{
		title = _title;
		if (load_finished)
			title_text->set_text(title.c_str());
	}

	void dTreeNodePrivate::notify_selected(bool v)
	{
		e_title->set_state(v ? (StateFlags)(e_title->state | StateSelected) : (StateFlags)(e_title->state & (~StateSelected)));
	}

	void dTreeNodePrivate::on_load_finished()
	{
		e_title = entity->find_child("title");
		fassert(e_title);
		title_text = e_title->get_component_t<cTextPrivate>();
		fassert(title_text);
		auto title_receiver = e_title->get_component_t<cReceiverPrivate>();
		fassert(title_receiver);

		title_receiver->add_mouse_left_down_listener([](Capture& c, const ivec2& pos) {
			auto thiz = c.thiz<dTreeNodePrivate>();
			thiz->tree->set_selected(thiz->entity);
		}, Capture().set_thiz(this));

		auto earrow = entity->find_child("arrow");
		fassert(earrow);
		arrow_text = earrow->get_component_t<cTextPrivate>();
		fassert(arrow_text);
		auto arrow_receiver = earrow->get_component_t<cReceiverPrivate>();
		fassert(arrow_receiver);
		arrow_receiver->add_mouse_left_down_listener([](Capture& c, const ivec2& pos) {
			c.thiz<dTreeNodePrivate>()->toggle_collapse();
		}, Capture().set_thiz(this));

		items = entity->find_child("items");
		fassert(items);
	}

	bool dTreeNodePrivate::on_child_added(Entity* _e)
	{
		if (load_finished)
		{
			auto e = (EntityPrivate*)_e;
			populate_tree(tree, e);
			items->add_child(e);
			return true;
		}
		return false;
	}

	void dTreeNodePrivate::toggle_collapse()
	{
		items->set_visible(!items->visible);
		arrow_text->set_text(items->visible ? Icon_CARET_DOWN : Icon_CARET_RIGHT);
	}

	dTreeNode* dTreeNode::create()
	{
		return f_new<dTreeNodePrivate>();
	}
}
