#include <flame/foundation/serialize.h>
#include <flame/universe/topmost.h>
#include <flame/universe/components/element.h>
#include <flame/universe/components/event_receiver.h>
#include <flame/universe/components/aligner.h>
#include <flame/universe/components/layout.h>
#include <flame/universe/components/menu.h>
#include <flame/universe/components/window.h>

#include "../app.h"
#include "scene_editor.h"
#include "hierarchy.h"
#include "inspector.h"

struct cSceneEditorPrivate : cSceneEditor
{
	std::wstring filename;
	std::vector<TypeinfoDatabase*> dbs;

	cSceneEditorPrivate()
	{
		prefab = nullptr;

		hierarchy_tab = nullptr;
		inspector = nullptr;

		selected = nullptr;

		dbs.push_back(TypeinfoDatabase::load(dbs, L"flame_foundation.typeinfo"));
		dbs.push_back(TypeinfoDatabase::load(dbs, L"flame_graphics.typeinfo"));
		dbs.push_back(TypeinfoDatabase::load(dbs, L"flame_universe.typeinfo"));
	}

	~cSceneEditorPrivate()
	{
		if (hierarchy_tab)
		{
			looper().add_delay_event([](void* c) {
				(*(cDockerTab**)c)->take_away(true);
			}, new_mail_p(hierarchy_tab));
		}
		if (inspector)
		{
			looper().add_delay_event([](void* c) {
				(*(cDockerTab**)c)->take_away(true);
			}, new_mail_p(inspector->tab));
		}

		for (auto db : dbs)
			TypeinfoDatabase::destroy(db);
	}

	void load(const std::wstring& _filename)
	{
		filename = _filename;
		if (prefab)
			e_scene->remove_child(prefab);
		prefab = Entity::create_from_file(dbs, filename);
		e_scene->add_child(prefab);
	}
};

const std::vector<TypeinfoDatabase*> cSceneEditor::dbs()
{
	return ((cSceneEditorPrivate*)this)->dbs;
}

void cSceneEditor::on_selected_changed(Entity* e)
{
	auto update_inspector = selected != e;
	selected = e;
	if (inspector && update_inspector)
		inspector->on_selected_changed();
}

void cSceneEditor::update()
{
}

void open_scene_editor(const std::wstring& filename, const Vec2f& pos)
{
	auto e_container = get_docker_container_model()->copy();
	app.root->add_child(e_container);
	{
		auto c_element = (cElement*)e_container->find_component(cH("Element"));
		c_element->x = pos.x();
		c_element->y = pos.y();
		c_element->width = 1000.f;
		c_element->height = 900.f;
	}

	auto e_docker = get_docker_model()->copy();
	e_container->add_child(e_docker, 0);

	e_docker->child(0)->add_child(create_standard_docker_tab(app.font_atlas_pixel, L"Scene Editor", app.root));

	auto e_page = get_docker_page_model()->copy();
	{
		auto c_layout = cLayout::create(LayoutVertical);
		c_layout->width_fit_children = false;
		c_layout->height_fit_children = false;
		e_page->add_component(c_layout);
	}
	e_docker->child(1)->add_child(e_page);

	auto c_editor = new_component<cSceneEditorPrivate>();
	e_page->add_component(c_editor);

	auto e_menubar = create_standard_menubar();
	e_page->add_child(e_menubar);
	{
		auto e_menu = create_standard_menu();
		{
			auto e_item = create_standard_menu_item(app.font_atlas_pixel, 1.f, L"Save");
			e_menu->add_child(e_item);
			((cEventReceiver*)e_item->find_component(cH("EventReceiver")))->add_mouse_listener([](void* c, KeyState action, MouseKey key, const Vec2f& pos) {
				auto editor = *(cSceneEditor**)c;
				if (is_mouse_clicked(action, key))
				{
					destroy_topmost(app.root);

				}
			}, new_mail_p(c_editor));
		}
		auto e_menu_btn = create_standard_menu_button(app.font_atlas_pixel, 1.f, L"File", app.root, e_menu, true, SideS, true, false, true, nullptr);
		e_menubar->add_child(e_menu_btn);
	}

	auto e_scene = Entity::create();
	e_page->add_child(e_scene);
	{
		auto c_element = cElement::create();
		c_element->background_frame_thickness = 2.f;
		e_scene->add_component(c_element);

		auto c_aligner = cAligner::create();
		c_aligner->width_policy = SizeFitParent;
		c_aligner->height_policy = SizeFitParent;
		e_scene->add_component(c_aligner);
	}

	c_editor->e_scene = e_scene;
	c_editor->load(filename);

	open_hierachy(c_editor, Vec2f(20.f));
	open_inspector(c_editor, Vec2f(1480, 20.f));
}
