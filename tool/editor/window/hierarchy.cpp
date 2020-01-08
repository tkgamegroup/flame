#include <flame/serialize.h>
#include <flame/universe/systems/event_dispatcher.h>
#include <flame/universe/components/element.h>
#include <flame/universe/components/event_receiver.h>
#include <flame/universe/components/aligner.h>
#include <flame/universe/components/layout.h>
#include <flame/universe/components/menu.h>
#include <flame/universe/components/tree.h>
#include <flame/universe/components/scrollbar.h>
#include <flame/universe/components/window.h>

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
							looper().add_event([](void* c) {
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

static void create_tree_node(cHierarchy* hierarchy, Entity* e, Entity* parent)
{
	if (e->child_count() > 0)
	{
		auto e_tree_node = create_standard_tree_node(app.font_atlas_pixel, s2w(e->name()).c_str());
		parent->add_child(e_tree_node);
		{
			auto e_item = e_tree_node->child(0);

			auto c_item = new_u_object<cHierarchyItem>();
			c_item->hierarchy = hierarchy;
			c_item->e = e;
			e_item->add_component(c_item);
		}

		auto e_sub_tree = e_tree_node->child(1);
		for (auto i = 0; i < e->child_count(); i++)
			create_tree_node(hierarchy, e->child(i), e_sub_tree);
	}
	else
	{
		auto e_tree_leaf = create_standard_tree_leaf(app.font_atlas_pixel, s2w(e->name()).c_str());
		parent->add_child(e_tree_leaf);
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
	e_tree->remove_child((Entity*)FLAME_INVALID_POINTER);
	create_tree_node(this, editor->prefab, e_tree);
	refresh_selected();
}

Entity* cHierarchy::find_item(Entity* e) const
{
	return find_item_in_tree(e_tree, e);
}

void open_hierachy(cSceneEditor* editor, const Vec2f& pos)
{
	auto e_container = get_docker_container_model()->copy();
	app.root->add_child(e_container);
	{
		auto c_element = e_container->get_component(cElement);
		c_element->pos_ = pos;
		c_element->size_.x() = 200.f;
		c_element->size_.y() = 900.f;
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

	auto c_hierarchy = new_u_object<cHierarchy>();
	e_page->add_component(c_hierarchy);
	c_hierarchy->tab = tab->get_component(cDockerTab);
	c_hierarchy->editor = editor;
	editor->hierarchy = c_hierarchy;

	auto e_tree = create_standard_tree(true);
	{
		e_tree->get_component(cElement)->inner_padding_ = Vec4f(4.f);

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
			looper().add_event([](void* c) {
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

	create_tree_node(c_hierarchy, editor->prefab, e_tree);

	c_hierarchy->e_tree = e_tree;

	c_hierarchy->e_item_menu = create_standard_menu();

	e_page->add_child(wrap_standard_scrollbar(e_tree, ScrollbarVertical, true, 1.f));
}
