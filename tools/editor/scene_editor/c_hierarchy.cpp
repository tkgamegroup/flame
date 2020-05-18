#include "scene_editor.h"

struct cHierarchyItem : Component
{
	cElement* element;
	cEventReceiver* event_receiver;

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
			element->cmds.add([](Capture& c, graphics::Canvas* canvas) {
				c.thiz<cHierarchyItem>()->draw(canvas);
				return true;
			}, Capture().set_thiz(this));
		}
		else if (c->name_hash == FLAME_CHASH("cEventReceiver"))
		{
			event_receiver = (cEventReceiver*)c;
			event_receiver->drag_hash = FLAME_CHASH("cHierarchyItem");
			event_receiver->set_acceptable_drops(1, &FLAME_CHASH("cHierarchyItem"));
			event_receiver->drag_and_drop_listeners.add([](Capture& c, DragAndDrop action, cEventReceiver* er, const Vec2i& pos) {
				auto thiz = c.thiz<cHierarchyItem>();
				auto element = thiz->element;

				if (action == BeingOvering)
				{
					auto h = element->global_size.y() * 0.3f;
					if (pos.y() < element->global_pos.y() + h)
						thiz->drop_pos = 0;
					else if (pos.y() > element->global_pos.y() + element->global_size.y() - h)
						thiz->drop_pos = 2;
					else
						thiz->drop_pos = 1;
				}
				else if (action == BeenDropped)
				{
					if (!(thiz->entity->parent()->get_component(cTree) && thiz->drop_pos != 1))
					{
						struct Capturing
						{
							Entity* dst;
							Entity* src;
							int i;
						}capture;
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
							looper().add_event([](Capture& c) {
								auto& capture = c.data<Capturing>();

								capture.src->parent()->remove_child(capture.src, false);

								if (capture.i == 1)
									capture.dst->add_child(capture.src);
								else
								{
									auto idx = capture.dst->index_;
									if (capture.i == 2)
										idx++;
									capture.dst->parent()->add_child(capture.src, idx);
								}

								scene_editor.hierarchy->refresh();
							}, Capture().set_data(&capture));
						}
					}

				}

				return true;
			}, Capture().set_thiz(this));
		}
	}

	void draw(graphics::Canvas* canvas)
	{
		if (event_receiver->dispatcher->drag_overing != event_receiver)
			drop_pos = -1;
		if (!element->clipped && drop_pos >= 0)
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

static void create_tree_node(Entity* e)
{
	auto& ui = scene_editor.window->ui;
	if (e->child_count() > 0)
	{
		auto e_tree_node = ui.e_begin_tree_node(s2w(e->name()).c_str());
		{
			auto c_item = f_new<cHierarchyItem>();
			c_item->e = e;
			e_tree_node->child(0)->add_component(c_item);
		}

		auto e_sub_tree = e_tree_node->child(1);
		for (auto i = 0; i < e->child_count(); i++)
			create_tree_node(e->child(i));
		ui.e_end_tree_node();
	}
	else
	{
		auto e_tree_leaf = ui.e_tree_leaf(s2w(e->name()).c_str());
		{
			auto c_item = f_new<cHierarchyItem>();
			c_item->e = e;
			e_tree_leaf->add_component(c_item);
		}
	}
}

cHierarchy::cHierarchy() :
	Component("cHierarchy")
{
	auto& ui = scene_editor.window->ui;

	auto e_page = ui.e_begin_docker_page(L"Hierarchy").second;
	{
		auto c_layout = ui.c_layout(LayoutVertical);
		c_layout->width_fit_children = false;
		c_layout->height_fit_children = false;

		e_page->add_component(this);
	}

		ui.e_begin_scrollbar(ScrollbarVertical, true);
			ui.next_element_padding = 8.f;
			e_tree = ui.e_begin_tree(true);
			{
				auto c_tree = e_tree->get_component(cTree);
				c_tree->data_changed_listeners.add([](Capture& c, uint hash, void*) {
					looper().add_event([](Capture& c) {
						auto s = c.thiz<Entity>();
						Entity* selected = nullptr;
						if (s)
						{
							if (s->get_component(cTreeLeaf))
								selected = s->get_component(cHierarchyItem)->e;
							else
								selected = s->child(0)->get_component(cHierarchyItem)->e;
						}
						auto different = selected != scene_editor.selected;
						scene_editor.selected = selected;
						if (scene_editor.inspector && different)
							scene_editor.inspector->refresh();
					}, Capture().set_thiz(c.thiz<cTree>()->selected));
					return true;
				}, Capture().set_thiz(c_tree));
			}
			ui.e_end_tree();
		ui.e_end_scrollbar();

		ui.e_end_docker_page();

	refresh();
}

cHierarchy::~cHierarchy()
{
	scene_editor.hierarchy = nullptr;
}

static Entity* _find_item(Entity* sub_tree, Entity* e)
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
			auto res = _find_item(item->child(1), e);
			if (res)
				return res;
		}
	}
	return nullptr;
}

void cHierarchy::refresh_selected()
{
	e_tree->get_component(cTree)->set_selected(scene_editor.selected ? find_item(scene_editor.selected) : nullptr, false);
}

void cHierarchy::refresh()
{
	auto& ui = scene_editor.window->ui;
	e_tree->remove_children(0, -1);
	e_tree->get_component(cTree)->selected = nullptr;
	if (scene_editor.prefab)
	{
		ui.parents.push(e_tree);
		create_tree_node(scene_editor.prefab);
		ui.parents.pop();
	}
	refresh_selected();
}

Entity* cHierarchy::find_item(Entity* e) const
{
	return _find_item(e_tree, e);
}
