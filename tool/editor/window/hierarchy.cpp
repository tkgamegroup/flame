#include <flame/graphics/canvas.h>
#include <flame/universe/components/element.h>
#include <flame/universe/components/event_receiver.h>
#include <flame/universe/components/aligner.h>
#include <flame/universe/components/layout.h>
#include <flame/universe/components/menu.h>
#include <flame/universe/components/tree.h>
#include <flame/universe/components/scrollbar.h>
#include <flame/universe/components/window.h>

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
	int last_drop_pos;

	cHierarchyItem() :
		Component("HierarchyItem")
	{
		drop_pos = -1;
		last_drop_pos = -1;
	}

	virtual void start() override
	{
		auto is_leaf = (bool)entity->find_component(cH("TreeLeaf"));

		auto e = is_leaf ? entity : entity->child(0);
		element = (cElement*)(e->find_component(cH("Element")));
		event_receiver =  (cEventReceiver*)(e->find_component(cH("EventReceiver")));
		event_receiver->drag_hash = cH("HierarchyItem");
		event_receiver->set_acceptable_drops({ cH("HierarchyItem") });

		event_receiver->add_drag_and_drop_listener([](void* c, DragAndDrop action, cEventReceiver* er, const Vec2f& pos) {
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
				if (!(thiz->entity->parent()->find_component(cH("Tree")) && thiz->last_drop_pos != 1))
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
					{
						auto e = er->entity;
						if (!e->find_component(cH("TreeLeaf")))
							e = e->parent();
						capture.src = ((cHierarchyItem*)e->find_component(cH("HierarchyItem")))->e;
					}
					capture.i = thiz->last_drop_pos;
					looper().add_delay_event([](void* c) {
						auto& capture = *(Capture*)c;

						capture.src->parent()->remove_child(capture.src, false);

						if (capture.i == 1)
							capture.dst->add_child(capture.src);
						else
						{
							auto p = capture.dst->parent();
							auto idx = p->child_position(capture.dst);
							if (capture.i == 2)
								idx++;
							p->add_child(capture.src, idx);
						}

						capture.h->refresh();
					}, new_mail(&capture));
				}

			}
		}, new_mail_p(this));
	}

	virtual void update() override
	{
		if (drop_pos >= 0)
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
			element->canvas->stroke(points, Vec4c(120, 150, 255, 255), 3.f);

			last_drop_pos = drop_pos;
			drop_pos = -1;
		}
	}
};

static void create_tree_node(cHierarchy* hierarchy, Entity* e, Entity* parent)
{
	if (e->child_count() > 0)
	{
		auto e_tree_node = create_standard_tree_node(app.font_atlas_pixel, s2w(e->name()));
		parent->add_child(e_tree_node);
		{
			auto c_item = new_component<cHierarchyItem>();
			c_item->hierarchy = hierarchy;
			c_item->e = e;
			e_tree_node->add_component(c_item);
		}

		auto e_sub_tree = e_tree_node->child(1);
		for (auto i = 0; i < e->child_count(); i++)
			create_tree_node(hierarchy, e->child(i), e_sub_tree);
	}
	else
	{
		auto e_tree_leaf = create_standard_tree_leaf(app.font_atlas_pixel, s2w(e->name()));
		parent->add_child(e_tree_leaf);
		{
			auto c_item = new_component<cHierarchyItem>();
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
		if (((cHierarchyItem*)item->find_component(cH("HierarchyItem")))->e == e)
			return item;
		if (item->find_component(cH("TreeNode")))
		{
			auto res = find_item_in_tree(item->child(1), e);
			if (res)
				return res;
		}
	}
	return nullptr;
}

void cHierarchy::refresh_selected()
{
	auto tree = (cTree*)e_tree->find_component(cH("Tree"));
	if (!editor->selected)
		tree->selected = nullptr;
	else
		tree->selected = find_item(editor->selected);
}

void cHierarchy::refresh()
{
	e_tree->remove_all_children();
	create_tree_node(this, editor->prefab, e_tree);
	refresh_selected();
}

Entity* cHierarchy::find_item(Entity* e) const
{
	return find_item_in_tree(e_tree, e);
}

void cHierarchy::update()
{
}

void open_hierachy(cSceneEditor* editor, const Vec2f& pos)
{
	auto e_container = get_docker_container_model()->copy();
	app.root->add_child(e_container);
	{
		auto c_element = (cElement*)e_container->find_component(cH("Element"));
		c_element->pos = pos;
		c_element->size.x() = 200.f;
		c_element->size.y() = 900.f;
	}

	auto e_docker = get_docker_model()->copy();
	e_container->add_child(e_docker, 0);

	auto tab = create_standard_docker_tab(app.font_atlas_pixel, L"Hierarchy", app.root);
	e_docker->child(0)->add_child(tab);

	auto e_page = get_docker_page_model()->copy();
	{
		auto c_layout = cLayout::create(LayoutVertical);
		c_layout->width_fit_children = false;
		c_layout->height_fit_children = false;
		e_page->add_component(c_layout);
	}
	e_docker->child(1)->add_child(e_page);

	auto c_hierarchy = new_component<cHierarchy>();
	e_page->add_component(c_hierarchy);
	c_hierarchy->tab = (cDockerTab*)tab->find_component(cH("DockerTab"));
	c_hierarchy->editor = editor;
	editor->hierarchy = c_hierarchy;

	auto e_tree = create_standard_tree(true);
	{
		((cElement*)e_tree->find_component(cH("Element")))->inner_padding = Vec4f(4.f);

		auto c_tree = (cTree*)e_tree->find_component(cH("Tree"));
		c_tree->add_selected_changed_listener([](void* c, Entity* e) {
			auto editor = *(cSceneEditor**)c;

			struct Capture
			{
				cSceneEditor* e;
				Entity* s;
			}capture;
			capture.e = editor;
			capture.s = e;
			looper().add_delay_event([](void* c) {
				auto& capture = *(Capture*)c;
				auto editor = capture.e;

				auto selected = capture.s ? ((cHierarchyItem*)capture.s->find_component(cH("HierarchyItem")))->e : nullptr;
				auto different = selected != editor->selected;
				editor->selected = selected;
				if (editor->inspector && different)
					editor->inspector->refresh();
			}, new_mail(&capture));
		}, new_mail_p(editor));

		auto c_event_receiver = cEventReceiver::create();
		c_event_receiver->add_mouse_listener([](void* c, KeyState action, MouseKey key, const Vec2f& pos) {
			auto tree = *(cTree**)c;
			if (is_mouse_down(action, key, true) && key == Mouse_Left)
				tree->set_selected(nullptr);
		}, new_mail_p(c_tree));
		e_tree->add_component(c_event_receiver);
	}

	create_tree_node(c_hierarchy, editor->prefab, e_tree);

	c_hierarchy->e_tree = e_tree;

	c_hierarchy->e_item_menu = create_standard_menu();

	e_page->add_child(wrap_standard_scrollbar(e_tree, ScrollbarVertical, true, 1.f));
}
