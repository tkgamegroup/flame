#include <flame/universe/components/element.h>
#include <flame/universe/components/event_receiver.h>
#include <flame/universe/components/aligner.h>
#include <flame/universe/components/layout.h>
#include <flame/universe/components/tree.h>
#include <flame/universe/components/scrollbar.h>
#include <flame/universe/components/window.h>

#include "../app.h"
#include "hierarchy.h"
#include "scene_editor.h"

struct cHierarchyItem : Component
{
	Entity* e;

	cHierarchyItem() :
		Component("HierarchyItem")
	{
	}

	virtual void update() override
	{
	}
};

struct cHierarchy : Component
{
	cSceneEditor* editor;

	cHierarchy() :
		Component("Hierarchy")
	{
	}

	~cHierarchy()
	{
		editor->hierarchy_tab = nullptr;
	}

	virtual void update() override
	{
	}
};

void create_tree_node(Entity* e, Entity* parent)
{
	if (e->child_count() > 0)
	{
		auto e_tree_node = create_standard_tree_node(app.font_atlas_pixel, s2w(e->name()));
		parent->add_child(e_tree_node);
		{
			auto e_item = new_component< cHierarchyItem>();
			e_item->e = e;
			e_tree_node->add_component(e_item);
		}

		auto e_sub_tree = e_tree_node->child(1);
		for (auto i = 0; i < e->child_count(); i++)
			create_tree_node(e->child(i), e_sub_tree);
	}
	else
	{
		auto e_tree_leaf = create_standard_tree_leaf(app.font_atlas_pixel, s2w(e->name()));
		parent->add_child(e_tree_leaf);
		{
			auto e_item = new_component< cHierarchyItem>();
			e_item->e = e;
			e_tree_leaf->add_component(e_item);
		}
	}
}

void open_hierachy(cSceneEditor* editor, const Vec2f& pos)
{
	auto e_container = get_docker_container_model()->copy();
	app.root->add_child(e_container);
	{
		auto c_element = (cElement*)e_container->find_component(cH("Element"));
		c_element->x = pos.x();
		c_element->y = pos.y();
		c_element->width = 200.f;
		c_element->height = 900.f;
	}

	auto e_docker = get_docker_model()->copy();
	e_container->add_child(e_docker);

	auto tab = create_standard_docker_tab(app.font_atlas_pixel, L"Hierarchy", app.root);
	e_docker->child(0)->add_child(tab);

	auto e_page = get_docker_page_model()->copy();
	{
		auto c_layout = cLayout::create();
		c_layout->type = LayoutVertical;
		c_layout->width_fit_children = false;
		c_layout->height_fit_children = false;
		e_page->add_component(c_layout);
	}
	e_docker->child(1)->add_child(e_page);

	auto c_hierarchy = new_component<cHierarchy>();
	e_page->add_component(c_hierarchy);
	c_hierarchy->editor = editor;
	editor->hierarchy_tab = (cDockerTab*)tab->find_component(cH("DockerTab"));

	auto e_tree = create_standard_tree(true);
	{
		((cElement*)e_tree->find_component(cH("Element")))->inner_padding = Vec4f(4.f);

		auto c_tree = (cTree*)e_tree->find_component(cH("Tree"));
		c_tree->add_selected_changed_listener([](void* c, Entity* e) {
			auto editor = *(cSceneEditor**)c;
			editor->on_selected_changed(e ? ((cHierarchyItem*)e->find_component(cH("HierarchyItem")))->e : nullptr);
		}, new_mail_p(editor));

		auto c_event_receiver = cEventReceiver::create();
		c_event_receiver->add_mouse_listener([](void* c, KeyState action, MouseKey key, const Vec2f& pos) {
			auto tree = *(cTree**)c;
			if (is_mouse_down(action, key, true) && key == Mouse_Left)
				tree->set_selected(nullptr);
		}, new_mail_p(c_tree));
		e_tree->add_component(c_event_receiver);
	}

	create_tree_node(editor->prefab, e_tree);

	e_page->add_child(wrap_standard_scrollbar(e_tree, ScrollbarVertical, true, 1.f));
}
