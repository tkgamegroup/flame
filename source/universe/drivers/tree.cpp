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

	void dTreePrivate::set_selected(Entity* e)
	{
		if (selected == e)
			return;
		if (selected)
			selected->set_state((StateFlags)(selected->state & (~StateSelected)));
		if (e)
			e->set_state((StateFlags)(((EntityPrivate*)e)->state | StateSelected));
		selected = (EntityPrivate*)e;
	}

	//	static void expand(Entity* e, Entity* r)
	//	{
	//		if (e == r)
	//			return;
	//		e->set_visible(true);
	//		auto p = e->parent;
	//		p->children[0]->children[0]->get_component(cText)->set_text(Icon_CARET_DOWN);
	//		expand(p->parent, r);
	//	}

	void dTreePrivate::expand_to_selected()
	{
		//if (!selected)
		//	return;
		//expand(selected->parent, entity);
		//looper().add_event([](Capture& c) {
		//	auto thiz = c.thiz<cTreePrivate>();
		//	auto selected = thiz->selected;
		//	if (!selected)
		//		return;
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
		//}, Capture().set_thiz(this), 1U);
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

	void dTreeNodePrivate::on_load_finished()
	{
		auto etitle = entity->find_child("title");
		fassert(etitle);
		title_text = etitle->get_component_t<cTextPrivate>();
		fassert(title_text);
		auto title_receiver = etitle->get_component_t<cReceiverPrivate>();
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
