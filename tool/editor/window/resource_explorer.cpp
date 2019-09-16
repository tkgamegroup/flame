#include <flame/graphics/font.h>
#include <flame/universe/topmost.h>
#include <flame/universe/components/element.h>
#include <flame/universe/components/text.h>
#include <flame/universe/components/event_receiver.h>
#include <flame/universe/components/aligner.h>
#include <flame/universe/components/layout.h>
#include <flame/universe/components/scrollbar.h>
#include <flame/universe/components/tree.h>
#include <flame/universe/components/menu.h>
#include <flame/universe/components/window.h>

#include "../app.h"
#include "resource_explorer.h"
#include "blueprint_editor.h"

struct cResourceExplorer : Component
{
	Entity* root;

	std::wstring selected_filename;
	Entity* dir_menu;
	Entity* bp_menu;

	cResourceExplorer() :
		Component("ResourceExplorer")
	{
	}

	virtual void update() override
	{
	}
};

void create_directory_tree_node(cResourceExplorer* explorer, const std::filesystem::path& path, Entity* parent)
{
	auto e_tree_node = create_standard_tree_node(app.font_atlas_pixel, Icon_FOLDER_O + std::wstring(L" ") + path.filename().wstring());
	parent->add_child(e_tree_node);
	{
		struct Capture
		{
			std::wstring fn;
			cResourceExplorer* e;
		}capture;
		capture.fn = path.wstring();
		capture.e = explorer;
		((cEventReceiver*)e_tree_node->child(0)->find_component(cH("EventReceiver")))->add_mouse_listener([](void* c, KeyState action, MouseKey key, const Vec2f& pos) {
			auto& capture = *(Capture*)c;
			if (is_mouse_down(action, key, true) && key == Mouse_Right)
			{
				capture.e->selected_filename = capture.fn;
			}
		}, new_mail(&capture));
	}

	auto e_sub_tree = e_tree_node->child(1);
	for (std::filesystem::directory_iterator end, it(path); it != end; it++)
	{
		if (std::filesystem::is_directory(it->status()))
		{
			if (it->path().filename().wstring() != L"build")
				create_directory_tree_node(explorer, it->path(), e_sub_tree);
		}
		else
		{
			auto e_tree_leaf = create_standard_tree_leaf(app.font_atlas_pixel, Icon_FILE_O + std::wstring(L" ") + it->path().filename().wstring());
			e_sub_tree->add_child(e_tree_leaf);
			struct Capture
			{
				std::wstring fn;
				cResourceExplorer* e;
			}capture;
			capture.fn = it->path().wstring();
			capture.e = explorer;
			((cEventReceiver*)e_tree_leaf->find_component(cH("EventReceiver")))->add_mouse_listener([](void* c, KeyState action, MouseKey key, const Vec2f& pos) {
				auto& capture = *(Capture*)c;
				if (is_mouse_down(action, key, true) && key == Mouse_Right)
				{
					capture.e->selected_filename = capture.fn;
					auto fn = std::filesystem::path(capture.fn).filename().wstring();
					if (fn == L"bp")
						popup_menu(capture.e->bp_menu, capture.e->root, pos);
				}
			}, new_mail(&capture));
		}
	}
}

void open_resource_explorer(const Vec2f& pos)
{
	auto e_container = get_docker_container_model()->copy();
	app.root->add_child(e_container);
	{
		auto c_element = (cElement*)e_container->find_component(cH("Element"));
		c_element->x = pos.x();
		c_element->y = pos.y();
		c_element->width = 300.f;
		c_element->height = 600.f;
	}

	auto e_docker = get_docker_model()->copy();
	e_container->add_child(e_docker);

	e_docker->child(0)->add_child(create_standard_docker_tab(app.font_atlas_pixel, L"Resource Explorer", app.root));

	auto e_page = get_docker_page_model()->copy();
	{
		auto c_layout = cLayout::create();
		c_layout->type = LayoutVertical;
		c_layout->item_padding = 4.f;
		c_layout->width_fit_children = false;
		c_layout->height_fit_children = false;
		e_page->add_component(c_layout);
	}
	auto c_explorer = new_component<cResourceExplorer>();
	{
		c_explorer->root = app.root;

		c_explorer->dir_menu = create_standard_menu();
		{

		}

		c_explorer->bp_menu = create_standard_menu();
		{
			auto mi_open = create_standard_menu_item(app.font_atlas_pixel, 1.f, L"Open");
			c_explorer->bp_menu->add_child(mi_open);
			((cEventReceiver*)mi_open->find_component(cH("EventReceiver")))->add_mouse_listener([](void* c, KeyState action, MouseKey key, const Vec2f& pos) {
				auto c_explorer = *(cResourceExplorer**)c;
				if (is_mouse_down(action, key, true) && key == Mouse_Left)
				{
					destroy_topmost(c_explorer->root);
					open_blueprint_editor(c_explorer->selected_filename, false, Vec2f(350.f, 20.f));
				}
			}, new_mail_p(c_explorer));
			auto mi_open_no_compile = create_standard_menu_item(app.font_atlas_pixel, 1.f, L"Open (No Compile)");
			c_explorer->bp_menu->add_child(mi_open_no_compile);
			((cEventReceiver*)mi_open_no_compile->find_component(cH("EventReceiver")))->add_mouse_listener([](void* c, KeyState action, MouseKey key, const Vec2f& pos) {
				auto c_explorer = *(cResourceExplorer**)c;
				if (is_mouse_down(action, key, true) && key == Mouse_Left)
				{
					destroy_topmost(c_explorer->root);
					open_blueprint_editor(c_explorer->selected_filename, true, Vec2f(350.f, 20.f));
				}
			}, new_mail_p(c_explorer));
		}
	}
	e_page->add_component(c_explorer);
	e_docker->child(1)->add_child(e_page);

	auto e_tree = Entity::create();
	{
		auto c_element = cElement::create();
		c_element->inner_padding = Vec4f(4.f);
		e_tree->add_component(c_element);

		auto c_aligner = cAligner::create();
		c_aligner->width_policy = SizeFitParent;
		c_aligner->height_policy = SizeFitParent;
		e_tree->add_component(c_aligner);

		auto c_layout = cLayout::create();
		c_layout->type = LayoutVertical;
		c_layout->item_padding = 4.f;
		c_layout->width_fit_children = false;
		c_layout->height_fit_children = false;
		e_tree->add_component(c_layout);

		e_tree->add_component(cTree::create());
	}

	create_directory_tree_node(c_explorer, L"../renderpath", e_tree);

	e_page->add_child(wrap_standard_scrollbar(e_tree, ScrollbarVertical, true, 1.f));
}
