#include <flame/foundation/serialize.h>
#include <flame/universe/components/element.h>
#include <flame/universe/components/aligner.h>
#include <flame/universe/components/layout.h>
#include <flame/universe/components/window.h>

#include "../app.h"
#include "scene_editor.h"

struct cSceneEditor : Component
{
	std::wstring filename;
	Entity* prefab;
	std::vector<TypeinfoDatabase*> dbs;

	Entity* e_scene;

	cSceneEditor() :
		Component("SceneEditor")
	{
		prefab = nullptr;
		dbs.push_back(TypeinfoDatabase::load(dbs, L"flame_foundation.typeinfo"));
		dbs.push_back(TypeinfoDatabase::load(dbs, L"flame_graphics.typeinfo"));
		dbs.push_back(TypeinfoDatabase::load(dbs, L"flame_universe.typeinfo"));
	}

	~cSceneEditor()
	{
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

	virtual void update() override
	{
	}
};

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
	e_container->add_child(e_docker);

	e_docker->child(0)->add_child(create_standard_docker_tab(app.font_atlas_pixel, L"Scene Editor", app.root));

	auto e_page = get_docker_page_model()->copy();
	{
		auto c_layout = cLayout::create();
		c_layout->type = LayoutVertical;
		c_layout->width_fit_children = false;
		c_layout->height_fit_children = false;
		e_page->add_component(c_layout);
	}
	e_docker->child(1)->add_child(e_page);

	auto c_editor = new_component<cSceneEditor>();
	e_page->add_component(c_editor);

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
}
