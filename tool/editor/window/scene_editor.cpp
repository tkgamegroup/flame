#include <flame/foundation/serialize.h>
#include <flame/graphics/canvas.h>
#include <flame/universe/topmost.h>
#include <flame/universe/components/element.h>
#include <flame/universe/components/event_receiver.h>
#include <flame/universe/components/aligner.h>
#include <flame/universe/components/layout.h>
#include <flame/universe/components/menu.h>
#include <flame/universe/components/tree.h>
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

		hierarchy = nullptr;
		inspector = nullptr;

		selected = nullptr;

		dbs.push_back(TypeinfoDatabase::load(dbs, L"flame_foundation.typeinfo"));
		dbs.push_back(TypeinfoDatabase::load(dbs, L"flame_graphics.typeinfo"));
		dbs.push_back(TypeinfoDatabase::load(dbs, L"flame_universe.typeinfo"));
	}

	~cSceneEditorPrivate()
	{
		if (hierarchy)
		{
			looper().add_delay_event([](void* c) {
				(*(cDockerTab**)c)->take_away(true);
			}, new_mail_p(hierarchy->tab));
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

void cSceneEditor::update()
{
}

struct cSceneOverlayer : Component
{
	cElement* element;

	cSceneEditorPrivate* editor;

	cSceneOverlayer() :
		Component("SceneOverlayer")
	{
	}

	virtual void start() override
	{
		element = (cElement*)entity->find_component(cH("Element"));
	}

	virtual void update() override
	{
		if (editor->selected)
		{
			auto se = (cElement*)editor->selected->find_component(cH("Element"));
			if (se)
			{
				std::vector<Vec2f> points;
				path_rect(points, Vec2f(se->global_x, se->global_y), Vec2f(se->global_width, se->global_height));
				points.push_back(points[0]);
				element->canvas->stroke(points, Vec4c(0, 0, 0, 255), 6.f);
			}
		}
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
		c_element->clip_children = true;
		e_scene->add_component(c_element);

		auto c_aligner = cAligner::create();
		c_aligner->width_policy = SizeFitParent;
		c_aligner->height_policy = SizeFitParent;
		e_scene->add_component(c_aligner);

		e_scene->add_component(cLayout::create(LayoutFree));
	}

	c_editor->e_scene = e_scene;
	c_editor->load(filename);

	auto e_overlayer = Entity::create();
	e_scene->add_child(e_overlayer);
	{
		e_overlayer->add_component(cElement::create());

		auto c_event_receiver = cEventReceiver::create();
		c_event_receiver->penetrable = true;
		c_event_receiver->add_mouse_listener([](void* c, KeyState action, MouseKey key, const Vec2f& pos) {
			auto thiz = *(cSceneEditorPrivate**)c;
			if (is_mouse_down(action, key, true) && key == Mouse_Left)
			{
				auto prev_selected = thiz->selected;
				thiz->selected = nullptr;
				struct Capture
				{
					cSceneEditorPrivate* thiz;
					Vec2f pos;
				}capture;
				capture.thiz = thiz;
				capture.pos = pos;
				thiz->prefab->traverse_backward([](void* c, Entity* e) {
					auto& capture = *(Capture*)c;
					if (capture.thiz->selected)
						return;

					auto element = (cElement*)e->find_component(cH("Element"));
					if (element && element->contains(capture.pos))
						capture.thiz->selected = e;
				}, new_mail(&capture));
				if (prev_selected != thiz->selected)
				{
					if (thiz->hierarchy)
					{
						auto tree = (cTree*)thiz->hierarchy->e_tree->find_component(cH("Tree"));
						if (!thiz->selected)
							tree->selected = nullptr;
						else
							tree->selected = thiz->hierarchy->find_item(thiz->selected);
					}
					if (thiz->inspector)
						thiz->inspector->on_selected_changed();
				}
			}
		}, new_mail_p(c_editor));
		e_overlayer->add_component(c_event_receiver);

		auto c_aligner = cAligner::create();
		c_aligner->width_policy = SizeFitParent;
		c_aligner->height_policy = SizeFitParent;
		e_overlayer->add_component(c_aligner);

		auto c_overlayer = new_component<cSceneOverlayer>();
		c_overlayer->editor = c_editor;
		e_overlayer->add_component(c_overlayer);
	}

	open_hierachy(c_editor, Vec2f(20.f));
	open_inspector(c_editor, Vec2f(1480, 20.f));
}
