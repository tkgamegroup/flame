#include <flame/foundation/serialize.h>
#include <flame/universe/topmost.h>
#include <flame/universe/default_style.h>
#include <flame/universe/components/element.h>
#include <flame/universe/components/text.h>
#include <flame/universe/components/event_receiver.h>
#include <flame/universe/components/checkbox.h>
#include <flame/universe/components/aligner.h>
#include <flame/universe/components/layout.h>
#include <flame/universe/components/menu.h>
#include <flame/universe/components/combobox.h>
#include <flame/universe/components/tree.h>
#include <flame/universe/components/style.h>
#include <flame/universe/components/window.h>

#include "../renderpath/canvas_make_cmd/canvas.h"

#include "../app.h"
#include "../data_tracker.h"
#include "scene_editor.h"
#include "hierarchy.h"
#include "inspector.h"

struct cSceneEditorPrivate : cSceneEditor
{
	std::wstring filename;

	Vec2f mpos;

	cSceneEditorPrivate()
	{
		prefab = nullptr;

		hierarchy = nullptr;
		inspector = nullptr;

		selected = nullptr;
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
	}

	void load(const std::wstring& _filename)
	{
		filename = _filename;
		if (prefab)
			e_scene->remove_child(prefab);
		prefab = Entity::create_from_file(app.dbs, filename);
		e_scene->add_child(prefab);
	}

	void search_hover(Entity* e)
	{
		for (auto i = e->child_count() - 1; i >= 0; i--)
		{
			auto c = e->child(i);
			if (c->global_visible_)
				search_hover(c);
		}
		if (selected)
			return;

		auto element = (cElement*)e->find_component(cH("Element"));
		if (element && element->contains(mpos))
			selected = e;
	}
};

void cSceneEditor::update()
{
}

struct cSceneOverlayer : Component
{
	cElement* element;

	cSceneEditorPrivate* editor;
	int tool_type;
	cElement* transform_tool_element;

	cSceneOverlayer() :
		Component("SceneOverlayer")
	{
		tool_type = 0;
	}

	virtual void on_component_added(Component* c) override
	{
		if (c->type_hash == cH("Element"))
			element = (cElement*)c;
	}

	virtual void update() override
	{
		transform_tool_element->pos = -200.f;
		if (editor->selected)
		{
			auto se = (cElement*)editor->selected->find_component(cH("Element"));
			if (se)
			{
				std::vector<Vec2f> points;
				path_rect(points, se->global_pos, se->global_size);
				points.push_back(points[0]);
				element->canvas->stroke(points, Vec4c(0, 0, 0, 255), Vec4c(255, 255, 255, 255), 6.f);

				if (tool_type > 0)
					transform_tool_element->pos = (se->global_pos + se->global_size * 0.5f) - element->global_pos - transform_tool_element->size * 0.5f;
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
		c_element->pos = pos;
		c_element->size.x() = 1000.f;
		c_element->size.y() = 900.f;
	}

	auto e_docker = get_docker_model()->copy();
	e_container->add_child(e_docker, 0);

	e_docker->child(0)->add_child(create_standard_docker_tab(app.font_atlas_pixel, L"Scene Editor", app.root));

	auto e_page = get_docker_page_model()->copy();
	{
		auto c_layout = cLayout::create(LayoutVertical);
		c_layout->item_padding = 4.f;
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
			auto e_item = create_standard_menu_item(app.font_atlas_pixel, 1.f, L"New Entity");
			e_menu->add_child(e_item);
			((cEventReceiver*)e_item->find_component(cH("EventReceiver")))->mouse_listeners.add([](void* c, KeyState action, MouseKey key, const Vec2i& pos) {
				auto editor = *(cSceneEditor**)c;
				if (is_mouse_clicked(action, key))
				{
					destroy_topmost(app.root);

					looper().add_delay_event([](void* c) {
						auto editor = *(cSceneEditor**)c;

						auto e = Entity::create();
						e->set_name("unnamed");
						if (editor->selected)
							editor->selected->add_child(e);
						else
							editor->prefab->add_child(e);
						if (editor->hierarchy)
							editor->hierarchy->refresh();
					}, new_mail_p(editor));
				}
			}, new_mail_p(c_editor));
		}
		{
			auto e_item = create_standard_menu_item(app.font_atlas_pixel, 1.f, L"Save");
			e_menu->add_child(e_item);
			((cEventReceiver*)e_item->find_component(cH("EventReceiver")))->mouse_listeners.add([](void* c, KeyState action, MouseKey key, const Vec2i& pos) {
				auto editor = *(cSceneEditor**)c;
				if (is_mouse_clicked(action, key))
				{
					destroy_topmost(app.root);

				}
			}, new_mail_p(c_editor));
		}
		auto e_menu_btn = create_standard_menu_button(app.font_atlas_pixel, 1.f, L"Scene", app.root, e_menu, true, SideS, true, false, true, nullptr);
		e_menubar->add_child(e_menu_btn);
	}
	{
		auto e_menu = create_standard_menu();
		{
			auto e_item = create_standard_menu_item(app.font_atlas_pixel, 1.f, L"Delete");
			e_menu->add_child(e_item);
			((cEventReceiver*)e_item->find_component(cH("EventReceiver")))->mouse_listeners.add([](void* c, KeyState action, MouseKey key, const Vec2i& pos) {
				auto editor = *(cSceneEditor**)c;
				if (is_mouse_clicked(action, key))
				{
					destroy_topmost(app.root);

					looper().add_delay_event([](void* c) {
						auto editor = *(cSceneEditor**)c;

						auto sel = editor->selected;
						if (sel)
						{
							editor->selected = nullptr;
							if (editor->inspector)
								editor->inspector->refresh();
							sel->parent()->remove_child(sel);
							if (editor->hierarchy)
								editor->hierarchy->refresh();
						}
					}, new_mail_p(editor));
				}
			}, new_mail_p(c_editor));
		}
		{
			auto e_item = create_standard_menu_item(app.font_atlas_pixel, 1.f, L"Duplicate");
			e_menu->add_child(e_item);
			((cEventReceiver*)e_item->find_component(cH("EventReceiver")))->mouse_listeners.add([](void* c, KeyState action, MouseKey key, const Vec2i& pos) {
				auto editor = *(cSceneEditor**)c;
				if (is_mouse_clicked(action, key))
				{
					destroy_topmost(app.root);

				}
			}, new_mail_p(c_editor));
		}
		auto e_menu_btn = create_standard_menu_button(app.font_atlas_pixel, 1.f, L"Edit", app.root, e_menu, true, SideS, true, false, true, nullptr);
		e_menubar->add_child(e_menu_btn);
	}

	auto e_tool = create_standard_combobox(50.f, app.font_atlas_pixel, 1.f, app.root, { L"Null", L"Move", L"Scale" });
	e_page->add_child(wrap_standard_text(e_tool, true, app.font_atlas_pixel, 1.f, L"Tool"));

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
		c_event_receiver->mouse_listeners.add([](void* c, KeyState action, MouseKey key, const Vec2i& pos) {
			auto editor = *(cSceneEditorPrivate**)c;
			if (is_mouse_down(action, key, true) && key == Mouse_Left)
			{
				struct Capture
				{
					cSceneEditorPrivate* e;
					Vec2f pos;
				}capture;
				capture.e = editor;
				capture.pos = pos;
				looper().add_delay_event([](void* c) {
					auto& capture = *(Capture*)c;
					auto editor = capture.e;

					auto prev_selected = editor->selected;
					editor->selected = nullptr;
					editor->mpos = capture.pos;
					editor->search_hover(editor->prefab);
					if (prev_selected != editor->selected)
					{
						if (editor->hierarchy)
							editor->hierarchy->refresh_selected();
						if (editor->inspector)
							editor->inspector->refresh();
					}
				}, new_mail(&capture));
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

		auto udt_element = find_udt(app.dbs, cH("ComponentElement"));
		assert(udt_element);
		auto element_pos_offset = udt_element->find_variable("pos")->offset();

		auto e_transform_tool = Entity::create();
		e_overlayer->add_child(e_transform_tool);
		{
			auto c_element = cElement::create();
			c_element->size = 20.f;
			c_element->frame_thickness = 2.f;
			e_transform_tool->add_component(c_element);
			c_overlayer->transform_tool_element = c_element;

			auto c_event_receiver = cEventReceiver::create();
			struct Capture
			{
				cSceneEditorPrivate* e;
				cEventReceiver* er;
				uint off;
			}capture;
			capture.e = c_editor;
			capture.er = c_event_receiver;
			capture.off = element_pos_offset;
			c_event_receiver->mouse_listeners.add([](void* c, KeyState action, MouseKey key, const Vec2i& pos) {
				auto& capture = *(Capture*)c;
				if (capture.er->active && is_mouse_move(action, key) && capture.e->selected)
				{
					auto e = (cElement*)capture.e->selected->find_component(cH("Element"));
					if (e)
					{
						e->pos += pos;
						capture.e->inspector->update_data_tracker(cH("Element"), capture.off);
					}
				}
			}, new_mail(&capture));
			e_transform_tool->add_component(c_event_receiver);

			e_transform_tool->add_component(cStyleColor::create(Vec4c(100, 100, 100, 128), Vec4c(50, 50, 50, 190), Vec4c(80, 80, 80, 255)));

			auto e_h_wing = Entity::create();
			e_transform_tool->add_child(e_h_wing);
			{
				auto c_element = cElement::create();
				c_element->pos.x() = 25.f;
				c_element->pos.y() = 5.f;
				c_element->size.x() = 20.f;
				c_element->size.y() = 10.f;
				c_element->frame_thickness = 2.f;
				e_h_wing->add_component(c_element);

				auto c_event_receiver = cEventReceiver::create();
				struct Capture
				{
					cSceneEditorPrivate* e;
					cEventReceiver* er;
					uint off;
				}capture;
				capture.e = c_editor;
				capture.er = c_event_receiver;
				capture.off = element_pos_offset;
				c_event_receiver->mouse_listeners.add([](void* c, KeyState action, MouseKey key, const Vec2i& pos) {
					auto& capture = *(Capture*)c;
					if (capture.er->active && is_mouse_move(action, key) && capture.e->selected)
					{
						auto e = (cElement*)capture.e->selected->find_component(cH("Element"));
						if (e)
						{
							e->pos.x() += pos.x();
							capture.e->inspector->update_data_tracker(cH("Element"), capture.off);
						}
					}
				}, new_mail(&capture));
				e_h_wing->add_component(c_event_receiver);

				e_h_wing->add_component(cStyleColor::create(Vec4c(100, 100, 100, 128), Vec4c(50, 50, 50, 190), Vec4c(80, 80, 80, 255)));
			}

			auto e_v_wing = Entity::create();
			e_transform_tool->add_child(e_v_wing);
			{
				auto c_element = cElement::create();
				c_element->pos.x() = 5.f;
				c_element->pos.y() = 25.f;
				c_element->size.x() = 10.f;
				c_element->size.y() = 20.f;
				c_element->frame_thickness = 2.f;
				e_v_wing->add_component(c_element);

				auto c_event_receiver = cEventReceiver::create();
				struct Capture
				{
					cSceneEditorPrivate* e;
					cEventReceiver* er;
					uint off;
				}capture;
				capture.e = c_editor;
				capture.er = c_event_receiver;
				capture.off = element_pos_offset;
				c_event_receiver->mouse_listeners.add([](void* c, KeyState action, MouseKey key, const Vec2i& pos) {
					auto& capture = *(Capture*)c;
					if (capture.er->active && is_mouse_move(action, key) && capture.e->selected)
					{
						auto e = (cElement*)capture.e->selected->find_component(cH("Element"));
						if (e)
						{
							e->pos.y() += pos.y();
							capture.e->inspector->update_data_tracker(cH("Element"), capture.off);
						}
					}
				}, new_mail(&capture));
				e_v_wing->add_component(c_event_receiver);

				e_v_wing->add_component(cStyleColor::create(Vec4c(100, 100, 100, 128), Vec4c(50, 50, 50, 190), Vec4c(80, 80, 80, 255)));
			}
		}

		{
			auto combobox = (cCombobox*)e_tool->find_component(cH("Combobox"));
			combobox->set_index(0, false);
			combobox->changed_listeners.add([](void* c, int idx) {
				(*(cSceneOverlayer**)c)->tool_type = idx;
			}, new_mail_p(c_overlayer));
		}
	}

	open_hierachy(c_editor, Vec2f(20.f));
	open_inspector(c_editor, Vec2f(1480, 20.f));
}
