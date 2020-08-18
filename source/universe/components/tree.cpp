#include "../entity_private.h"
#include "event_receiver_private.h"
#include "tree_private.h"

namespace flame
{
	void cTreePrivate::set_selected(Entity* e)
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

	//	static void expand(Entity* e, Entity* r)
	//	{
	//		if (e == r)
	//			return;
	//		e->set_visible(true);
	//		auto p = e->parent;
	//		p->children[0]->children[0]->get_component(cText)->set_text(Icon_CARET_DOWN);
	//		expand(p->parent, r);
	//	}

	void cTreePrivate::expand_to_selected()
	{
		//		if (!selected)
		//			return;
		//		expand(selected->parent, entity);
		//		looper().add_event([](Capture& c) {
		//			auto thiz = c.thiz<cTreePrivate>();
		//			auto selected = thiz->selected;
		//			if (!selected)
		//				return;
		//			auto parent = thiz->entity->parent;
		//			if (!parent || parent->children.s < 2)
		//				return;
		//			auto e_scrollbar = parent->children[1];
		//			auto c_scrollbar = e_scrollbar->get_component(cScrollbar);
		//			if (!c_scrollbar)
		//				return;
		//			auto e_thumb = e_scrollbar->children[0];
		//			auto e_tree = thiz->entity;
		//			auto c_tree_layout = e_tree->get_component(cLayout);
		//			e_thumb->get_component(cElement)->set_y(e_scrollbar->get_component(cElement)->size.y() *
		//				(selected->get_component(cElement)->global_pos.y() - e_tree->get_component(cElement)->global_pos.y() - c_tree_layout->scroll_offset.y()) / (c_tree_layout->content_size.y() + 20.f));
		//			e_thumb->get_component(cScrollbarThumb)->update(0.f);
		//		}, Capture().set_thiz(this), 1U);
	}

	cTree* cTree::create()
	{
		return f_new<cTreePrivate>();
	}

	void cTreeLeafPrivate::on_gain_event_receiver()
	{
		mouse_listener = event_receiver->add_mouse_left_down_listener([](Capture& c, const Vec2i& pos) {
			auto thiz = c.thiz<cTreeLeafPrivate>();
			thiz->tree->set_selected(thiz->entity);
		}, Capture().set_thiz(this));
	}

	void cTreeLeafPrivate::on_lost_event_receiver()
	{
		event_receiver->remove_mouse_left_down_listener(mouse_listener);
	}

	cTreeLeaf* cTreeLeaf::create()
	{
		return f_new<cTreeLeafPrivate>();
	}

	void cTreeNodePrivate::on_gain_event_receiver()
	{

	}

	void cTreeNodePrivate::on_lost_event_receiver()
	{

	}

	cTreeNode* cTreeNode::create()
	{
		return f_new<cTreeNodePrivate>();
	}

//	struct cTreeNodePrivate : cTreeNode
//	{
//					mouse_listener = event_receiver->mouse_listeners.add([](Capture& c, KeyStateFlags action, MouseKey key, const Vec2i& pos) {
//						if (is_mouse_down(action, key, true) && (key == Mouse_Left || key == Mouse_Right))
//						{
//							auto thiz = c.thiz<cTreeNodeTitlePrivate>();
//							thiz->tree->set_selected(thiz->entity->parent);
//						}
//					}, Capture().set_thiz(this));
//
//					mouse_listener = event_receiver->mouse_listeners.add([](Capture& c, KeyStateFlags action, MouseKey key, const Vec2i& pos) {
//						if (is_mouse_down(action, key, true) && key == Mouse_Left)
//							c.thiz<cTreeNodeArrowPrivate>()->toggle_collapse();
//					}, Capture().set_thiz(this));
//		void toggle_collapse()
//		{
//			auto e = entity->parent->parent->children[1];
//			e->set_visible(!e->visible_);
//			text->set_text(e->visible_ ? Icon_CARET_DOWN : Icon_CARET_RIGHT);
//		}
//	};
//
//	struct cTreePrivate : cTree
//	{
//					mouse_listener = event_receiver->mouse_listeners.add([](Capture& c, KeyStateFlags action, MouseKey key, const Vec2i& pos) {
//						if (is_mouse_down(action, key, true) && key == Mouse_Left)
//							c.thiz<cTreePrivate>()->set_selected(nullptr);
//					}, Capture().set_thiz(this));
//	};
}
