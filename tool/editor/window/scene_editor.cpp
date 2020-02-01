#include <flame/serialize.h>
#include <flame/universe/ui/utils.h>

#include "../renderpath/canvas/canvas.h"

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
			looper().add_event([](void* c) {
				(*(cDockerTab**)c)->take_away(true);
			}, new_mail_p(hierarchy->tab));
		}
		if (inspector)
		{
			looper().add_event([](void* c) {
				(*(cDockerTab**)c)->take_away(true);
			}, new_mail_p(inspector->tab));
		}
	}

	void load(const std::wstring& _filename)
	{
		filename = _filename;
		if (prefab)
			e_scene->remove_child(prefab);
		prefab = Entity::create_from_file(e_scene->world_, filename.c_str());
		e_scene->add_child(prefab);
	}

	void search_hover(Entity* e)
	{
		if (e->child_count() > 0)
		{
			for (auto i = (int)e->child_count() - 1; i >= 0; i--)
			{
				auto c = e->child(i);
				if (c->global_visibility_)
					search_hover(c);
			}
		}
		if (selected)
			return;

		auto element = e->get_component(cElement);
		if (element && rect_contains(element->cliped_rect, mpos))
			selected = e;
	}
};

struct cSceneOverlayer : Component
{
	cElement* element;

	cSceneEditorPrivate* editor;
	int tool_type;
	cElement* transform_tool_element;

	cSceneOverlayer() :
		Component("cSceneOverlayer")
	{
		tool_type = 0;
	}

	virtual void on_component_added(Component* c) override
	{
		if (c->name_hash == FLAME_CHASH("cElement"))
		{
			element = (cElement*)c;
			element->cmds.add([](void* c, graphics::Canvas* canvas) {
				(*(cSceneOverlayer**)c)->draw(canvas);
			}, new_mail_p(this));
		}
	}

	void draw(graphics::Canvas* canvas)
	{
		if (!element->cliped && editor->selected)
		{
			auto se = editor->selected->get_component(cElement);
			if (se)
			{
				std::vector<Vec2f> points;
				path_rect(points, se->global_pos, se->global_size);
				points.push_back(points[0]);
				canvas->stroke(points.size(), points.data(), Vec4c(255, 255, 255, 255), 6.f);

				if (tool_type > 0)
					transform_tool_element->set_pos((se->global_pos + se->global_size * 0.5f) - element->global_pos - transform_tool_element->size_ * 0.5f);
			}
		}
		else
			transform_tool_element->set_pos(Vec2f(-200.f));
	}
};

void open_scene_editor(const std::wstring& filename, const Vec2f& pos)
{
	ui::push_parent(app.root);
	ui::next_element_pos = pos;
	ui::next_element_size = Vec2f(1000.f, 900.f);
	ui::e_begin_docker_floating_container();
	ui::e_begin_docker();
	ui::e_begin_docker_page(L"Scene Editor");
	{
		auto c_layout = ui::c_layout(LayoutVertical);
		c_layout->item_padding = 4.f;
		c_layout->width_fit_children = false;
		c_layout->height_fit_children = false;
	}
	auto c_editor = new_u_object<cSceneEditorPrivate>();
	ui::current_entity()->add_component(c_editor);

	ui::e_begin_menu_bar();
	ui::e_begin_menubar_menu(L"Scene");
	ui::e_menu_item(L"New Entity", [](void* c) {
		auto editor = *(cSceneEditor**)c;
		looper().add_event([](void* c) {
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
	}, new_mail_p(c_editor));
	ui::e_menu_item(L"Save", [](void* c) {
		auto editor = *(cSceneEditor**)c;
	}, new_mail_p(c_editor));
	ui::e_end_menubar_menu();
	ui::e_begin_menubar_menu(L"Edit");
	ui::e_menu_item(L"Delete", [](void* c) {
		auto editor = *(cSceneEditor**)c;
		looper().add_event([](void* c) {
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
	}, new_mail_p(c_editor));
	ui::e_menu_item(L"Duplicate", [](void* c) {
		auto editor = *(cSceneEditor**)c;
	}, new_mail_p(c_editor));
	ui::e_end_menubar_menu();
	ui::e_end_menu_bar();

	ui::e_begin_layout(LayoutHorizontal, 4.f);
	ui::e_text(L"Tool");
	auto e_tool = ui::e_begin_combobox(50.f, 0);
	ui::e_combobox_item(L"Null");
	ui::e_combobox_item(L"Move");
	ui::e_combobox_item(L"Scale");
	ui::e_end_combobox();
	ui::e_end_layout();
	
	ui::e_begin_layout()->get_component(cElement)->clip_children = true;
	ui::c_aligner(SizeFitParent, SizeFitParent);
	c_editor->e_scene = ui::current_entity();
	c_editor->load(filename);

	auto e_overlayer = ui::e_empty();
	ui::c_element();
	auto c_event_receiver = ui::c_event_receiver();
	c_event_receiver->pass = (Entity*)INVALID_POINTER;
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
			looper().add_event([](void* c) {
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
	ui::c_aligner(SizeFitParent, SizeFitParent);
	ui::push_parent(e_overlayer);
	{
		auto c_overlayer = new_u_object<cSceneOverlayer>();
		c_overlayer->editor = c_editor;
		e_overlayer->add_component(c_overlayer);

		auto udt_element = find_udt(FLAME_CHASH("Serializer_cElement"));
		assert(udt_element);
		auto element_pos_offset = udt_element->find_variable("pos")->offset();

		auto e_transform_tool = ui::e_empty();
		c_overlayer->transform_tool_element = ui::c_element();
		c_overlayer->transform_tool_element->size_ = 20.f;
		c_overlayer->transform_tool_element->frame_thickness_ = 2.f;
		{
			auto c_event_receiver = ui::c_event_receiver();
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
					auto e = capture.e->selected->get_component(cElement);
					if (e)
					{
						e->set_pos(Vec2f(pos), true);
						capture.e->inspector->update_data_tracker(FLAME_CHASH("cElement"), capture.off);
					}
				}
			}, new_mail(&capture));
			{
				auto c_style = ui::c_style_color();
				c_style->color_normal = Vec4c(100, 100, 100, 128);
				c_style->color_hovering = Vec4c(50, 50, 50, 190);
				c_style->color_active = Vec4c(80, 80, 80, 255);
				c_style->style();
			}

			ui::push_parent(e_transform_tool);
			ui::e_empty();
			{
				auto c_element = ui::c_element();
				c_element->pos_.x() = 25.f;
				c_element->pos_.y() = 5.f;
				c_element->size_.x() = 20.f;
				c_element->size_.y() = 10.f;
				c_element->frame_thickness_ = 2.f;

				auto c_event_receiver = ui::c_event_receiver();
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
						auto e = capture.e->selected->get_component(cElement);
						if (e)
						{
							e->set_x(pos.x(), true);
							capture.e->inspector->update_data_tracker(FLAME_CHASH("cElement"), capture.off);
						}
					}
				}, new_mail(&capture));

				{
					auto c_style = ui::c_style_color();
					c_style->color_normal = Vec4c(100, 100, 100, 128);
					c_style->color_hovering = Vec4c(50, 50, 50, 190);
					c_style->color_active = Vec4c(80, 80, 80, 255);
					c_style->style();
				}
			}
			ui::e_empty();
			{
				auto c_element = ui::c_element();
				c_element->pos_.x() = 5.f;
				c_element->pos_.y() = 25.f;
				c_element->size_.x() = 10.f;
				c_element->size_.y() = 20.f;
				c_element->frame_thickness_ = 2.f;

				auto c_event_receiver = ui::c_event_receiver();
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
						auto e = capture.e->selected->get_component(cElement);
						if (e)
						{
							e->set_y(pos.y(), true);
							capture.e->inspector->update_data_tracker(FLAME_CHASH("cElement"), capture.off);
						}
					}
				}, new_mail(&capture));

				{
					auto c_style = ui::c_style_color();
					c_style->color_normal = Vec4c(100, 100, 100, 128);
					c_style->color_hovering = Vec4c(50, 50, 50, 190);
					c_style->color_active = Vec4c(80, 80, 80, 255);
					c_style->style();
				}
			}
			ui::pop_parent();
		}

		e_tool->get_component(cCombobox)->data_changed_listeners.add([](void* c, Component* cb, uint hash, void*) {
			if (hash == FLAME_CHASH("index"))
				(*(cSceneOverlayer**)c)->tool_type = ((cCombobox*)cb)->idx;
		}, new_mail_p(c_overlayer));
	}
	ui::pop_parent();

	ui::e_end_layout();

	ui::e_end_docker_page();
	ui::e_end_docker();
	ui::e_end_docker_floating_container();
	ui::pop_parent();

	open_hierachy(c_editor, Vec2f(20.f));
	open_inspector(c_editor, Vec2f(1480, 20.f));
}
