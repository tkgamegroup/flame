#include <flame/serialize.h>
#include <flame/universe/systems/event_dispatcher.h>
#include <flame/universe/ui/utils.h>

#include "../renderpath/canvas/canvas.h"

#include "../app.h"
#include "hierarchy.h"
#include "scene_editor.h"
#include "inspector.h"

struct cHierarchyItem : Component
{
	cElement* element;
	cEventReceiver* event_receiver;

	cHierarchy* hierarchy;
	Entity* e;

	int drop_pos;

	cHierarchyItem() :
		Component("cHierarchyItem")
	{
		drop_pos = -1;
	}

	virtual void on_component_added(Component* c) override
	{
		if (c->name_hash == FLAME_CHASH("cElement"))
		{
			element = (cElement*)c;
			element->cmds.add([](void* c, graphics::Canvas* canvas) {
				(*(cHierarchyItem**)c)->draw(canvas);
			}, new_mail_p(this));
		}
		else if (c->name_hash == FLAME_CHASH("cEventReceiver"))
		{
			event_receiver = (cEventReceiver*)c;
			event_receiver->drag_hash = FLAME_CHASH("cHierarchyItem");
			event_receiver->set_acceptable_drops(1, &FLAME_CHASH("cHierarchyItem"));
			event_receiver->drag_and_drop_listeners.add([](void* c, DragAndDrop action, cEventReceiver* er, const Vec2i& pos) {
				auto thiz = *(cHierarchyItem**)c;
				auto element = thiz->element;

				if (action == DragOvering)
				{
					auto h = element->global_size.y() * 0.3f;
					if (pos.y() < element->global_pos.y() + h)
						thiz->drop_pos = 0;
					else if (pos.y() > element->global_pos.y() + element->global_size.y() - h)
						thiz->drop_pos = 2;
					else
						thiz->drop_pos = 1;
				}
				else if (action == Dropped)
				{
					if (!(thiz->entity->parent()->get_component(cTree) && thiz->drop_pos != 1))
					{
						struct Capture
						{
							cHierarchy* h;
							Entity* dst;
							Entity* src;
							int i;
						}capture;
						capture.h = thiz->hierarchy;
						capture.dst = thiz->e;
						capture.src = er->entity->get_component(cHierarchyItem)->e;
						capture.i = thiz->drop_pos;

						auto ok = true;
						auto p = capture.src->parent();
						while (p)
						{
							if (p == capture.dst)
							{
								ok = false;
								break;
							}
						}

						if (ok)
						{
							looper().add_event([](void* c, bool*) {
								auto& capture = *(Capture*)c;

								capture.src->parent()->remove_child(capture.src, false);

								if (capture.i == 1)
									capture.dst->add_child(capture.src);
								else
								{
									auto idx = capture.dst->order_ & 0xffffff;
									if (capture.i == 2)
										idx++;
									capture.dst->parent()->add_child(capture.src, idx);
								}

								capture.h->refresh();
							}, new_mail(&capture));
						}
					}

				}
			}, new_mail_p(this));
		}
	}

	void draw(graphics::Canvas* canvas)
	{
		if (event_receiver->dispatcher->drag_overing != event_receiver)
			drop_pos = -1;
		if (!element->cliped && drop_pos >= 0)
		{
			std::vector<Vec2f> points;
			switch (drop_pos)
			{
			case 0:
				points.push_back(element->global_pos);
				path_move(points, element->global_size.x(), 0.f);
				break;
			case 1:
				path_rect(points, element->global_pos, element->global_size);
				points.push_back(points[0]);
				break;
			case 2:
				points.push_back(element->global_pos + Vec2f(0.f, element->global_size.y()));
				path_move(points, element->global_size.x(), 0.f);
				break;
			}
			canvas->stroke(points.size(), points.data(), Vec4c(120, 150, 255, 255), 3.f);
		}
	}
};

static void create_tree_node(cHierarchy* hierarchy, Entity* e)
{
	if (e->child_count() > 0)
	{
		auto e_tree_node = ui::e_begin_tree_node(s2w(e->name()).c_str());
		{
			auto c_item = new_u_object<cHierarchyItem>();
			c_item->hierarchy = hierarchy;
			c_item->e = e;
			e_tree_node->child(0)->add_component(c_item);
		}

		auto e_sub_tree = e_tree_node->child(1);
		for (auto i = 0; i < e->child_count(); i++)
			create_tree_node(hierarchy, e->child(i));
		ui::e_end_tree_node();
	}
	else
	{
		auto e_tree_leaf = ui::e_tree_leaf(s2w(e->name()).c_str());
		{
			auto c_item = new_u_object<cHierarchyItem>();
			c_item->hierarchy = hierarchy;
			c_item->e = e;
			e_tree_leaf->add_component(c_item);
		}
	}
}

cHierarchy::~cHierarchy()
{
	editor->hierarchy = nullptr;
}

static Entity* find_item_in_tree(Entity* sub_tree, Entity* e)
{
	for (auto i = 0; i < sub_tree->child_count(); i++)
	{
		auto item = sub_tree->child(i);
		if (item->get_component(cTreeLeaf))
		{
			if (item->get_component(cHierarchyItem)->e == e)
				return item;
		}
		else
		{
			if (item->child(0)->get_component(cHierarchyItem)->e == e)
				return item;
			auto res = find_item_in_tree(item->child(1), e);
			if (res)
				return res;
		}
	}
	return nullptr;
}

void cHierarchy::refresh_selected()
{
	auto tree = e_tree->get_component(cTree);
	if (!editor->selected)
		tree->set_selected(nullptr, false);
	else
		tree->set_selected(find_item(editor->selected), false);
}

void cHierarchy::refresh()
{
	e_tree->remove_children(0, -1);
	ui::push_parent(e_tree);
	create_tree_node(this, editor->prefab);
	ui::pop_parent();
	refresh_selected();
}

Entity* cHierarchy::find_item(Entity* e) const
{
	return find_item_in_tree(e_tree, e);
}

void open_hierachy(cSceneEditor* editor, const Vec2f& pos)
{
	ui::push_parent(app.root);
	ui::next_element_pos = pos;
	ui::next_element_size = Vec2f(200.f, 900.f);
	ui::e_begin_docker_floating_container();
	ui::e_begin_docker();
	auto c_tab = ui::e_begin_docker_page(L"Hierarchy").tab->get_component(cDockerTab);
	ui::current_entity();
	{
		auto c_layout = ui::c_layout(LayoutVertical);
		c_layout->width_fit_children = false;
		c_layout->height_fit_children = false;
	}
	auto c_hierarchy = new_u_object<cHierarchy>();
	ui::current_entity()->add_component(c_hierarchy);
	c_hierarchy->tab = c_tab;
	c_hierarchy->editor = editor;
	editor->hierarchy = c_hierarchy;

	ui::e_begin_scroll_view1(ScrollbarVertical, Vec2f(0.f));
	auto e_tree = ui::e_begin_tree(true, 8.f);
	{
		auto c_tree = e_tree->get_component(cTree);
		c_tree->data_changed_listeners.add([](void* c, Component* t, uint hash, void*) {
			auto editor = *(cSceneEditor**)c;

			struct Capture
			{
				cSceneEditor* e;
				Entity* s;
			}capture;
			capture.e = editor;
			capture.s = ((cTree*)t)->selected;
			looper().add_event([](void* c, bool*) {
				auto& capture = *(Capture*)c;
				auto editor = capture.e;
				Entity* selected = nullptr;
				if (capture.s)
				{
					if (capture.s->get_component(cTreeLeaf))
						selected = capture.s->get_component(cHierarchyItem)->e;
					else
						selected = capture.s->child(0)->get_component(cHierarchyItem)->e;
				}
				auto different = selected != editor->selected;
				editor->selected = selected;
				if (editor->inspector && different)
					editor->inspector->refresh();
			}, new_mail(&capture));
		}, new_mail_p(editor));
	}
	ui::e_end_tree();
	ui::e_end_scroll_view1();

	ui::e_end_docker_page();
	ui::e_end_docker();
	ui::e_end_docker_floating_container();
	ui::pop_parent();

	c_hierarchy->e_tree = e_tree;
	ui::push_parent(e_tree);
	create_tree_node(c_hierarchy, editor->prefab);
	ui::pop_parent();
}
