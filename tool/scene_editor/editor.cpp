#include "app.h"

struct cOverlayer : Component
{
	cElement* element;

	int tool_type;
	cElement* transform_tool_element;

	cOverlayer() :
		Component("cOverlayer")
	{
		tool_type = 0;
	}

	virtual void on_component_added(Component* c) override
	{
		if (c->name_hash == FLAME_CHASH("cElement"))
		{
			element = (cElement*)c;
			element->cmds.add([](void* c, graphics::Canvas* canvas) {
				(*(cOverlayer**)c)->draw(canvas);
				return true;
			}, new_mail_p(this));
		}
	}

	void draw(graphics::Canvas* canvas)
	{
		if (!element->cliped && app.selected)
		{
			auto se = app.selected->get_component(cElement);
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

cEditor::cEditor() :
	Component("cEditor")
{
	auto e_page = utils::e_begin_docker_page(L"Editor").second;
	{
		auto c_layout = utils::c_layout(LayoutVertical);
		c_layout->item_padding = 4.f;
		c_layout->width_fit_children = false;
		c_layout->height_fit_children = false;

		e_page->add_component(this);
	}

		utils::e_begin_layout(LayoutHorizontal, 4.f);
			utils::e_text(L"Tool");
			auto e_tool = utils::e_begin_combobox(50.f);
				utils::e_combobox_item(L"Null");
				utils::e_combobox_item(L"Move");
				utils::e_combobox_item(L"Scale");
			utils::e_end_combobox(0);
		utils::e_end_layout();

		utils::e_begin_layout()->get_component(cElement)->clip_children = true;
		utils::c_aligner(SizeFitParent, SizeFitParent);
		e_scene = utils::current_entity();
		if (app.prefab)
			e_scene->add_child(app.prefab);

		auto e_overlayer = utils::e_element();
		auto c_event_receiver = utils::c_event_receiver();
		c_event_receiver->pass = (Entity*)INVALID_POINTER;
		c_event_receiver->mouse_listeners.add([](void* c, KeyStateFlags action, MouseKey key, const Vec2i& pos) {
			if (is_mouse_down(action, key, true) && key == Mouse_Left)
			{
				struct Capture
				{
					Vec2f pos;
				}capture;
				capture.pos = pos;
				looper().add_event([](void* c, bool*) {
					auto& capture = *(Capture*)c;

					auto prev_selected = app.selected;
					app.selected = nullptr;
					app.editor->mpos = capture.pos;
					app.editor->search_hover(app.prefab);
					if (prev_selected != app.selected)
					{
						if (app.hierarchy)
							app.hierarchy->refresh_selected();
						if (app.inspector)
							app.inspector->refresh();
					}
				}, new_mail(&capture));
			}
		}, Mail<>());
		utils::c_aligner(SizeFitParent, SizeFitParent);
		utils::push_parent(e_overlayer);
		{
			auto c_overlayer = new_u_object<cOverlayer>();
			e_overlayer->add_component(c_overlayer);

			auto udt_element = find_udt(FLAME_CHASH("D#flame::Serializer_cElement"));
			assert(udt_element);
			auto element_pos_offset = udt_element->find_variable("pos")->offset();

			auto e_transform_tool = utils::e_empty();
			c_overlayer->transform_tool_element = utils::c_element();
			c_overlayer->transform_tool_element->size_ = 20.f;
			c_overlayer->transform_tool_element->frame_thickness_ = 2.f;
			{
				auto c_event_receiver = utils::c_event_receiver();
				struct Capture
				{
					cEventReceiver* er;
					uint off;
				}capture;
				capture.er = c_event_receiver;
				capture.off = element_pos_offset;
				c_event_receiver->mouse_listeners.add([](void* c, KeyStateFlags action, MouseKey key, const Vec2i& pos) {
					auto& capture = *(Capture*)c;
					if (capture.er->active && is_mouse_move(action, key) && app.selected)
					{
						auto e = app.selected->get_component(cElement);
						if (e)
						{
							e->set_pos(Vec2f(pos), true);
							if (app.inspector)
								app.inspector->update_data_tracker(FLAME_CHASH("cElement"), capture.off);
						}
					}
				}, new_mail(&capture));
				{
					auto c_style = utils::c_style_color();
					c_style->color_normal = Vec4c(100, 100, 100, 128);
					c_style->color_hovering = Vec4c(50, 50, 50, 190);
					c_style->color_active = Vec4c(80, 80, 80, 255);
					c_style->style();
				}

				utils::push_parent(e_transform_tool);
				utils::e_empty();
				{
					auto c_element = utils::c_element();
					c_element->pos_.x() = 25.f;
					c_element->pos_.y() = 5.f;
					c_element->size_.x() = 20.f;
					c_element->size_.y() = 10.f;
					c_element->frame_thickness_ = 2.f;

					auto c_event_receiver = utils::c_event_receiver();
					struct Capture
					{
						cEventReceiver* er;
						uint off;
					}capture;
					capture.er = c_event_receiver;
					capture.off = element_pos_offset;
					c_event_receiver->mouse_listeners.add([](void* c, KeyStateFlags action, MouseKey key, const Vec2i& pos) {
						auto& capture = *(Capture*)c;
						if (capture.er->active && is_mouse_move(action, key) && app.selected)
						{
							auto e = app.selected->get_component(cElement);
							if (e)
							{
								e->set_x(pos.x(), true);
								if (app.inspector)
									app.inspector->update_data_tracker(FLAME_CHASH("cElement"), capture.off);
							}
						}
					}, new_mail(&capture));

					{
						auto c_style = utils::c_style_color();
						c_style->color_normal = Vec4c(100, 100, 100, 128);
						c_style->color_hovering = Vec4c(50, 50, 50, 190);
						c_style->color_active = Vec4c(80, 80, 80, 255);
						c_style->style();
					}
				}
				utils::e_empty();
				{
					auto c_element = utils::c_element();
					c_element->pos_.x() = 5.f;
					c_element->pos_.y() = 25.f;
					c_element->size_.x() = 10.f;
					c_element->size_.y() = 20.f;
					c_element->frame_thickness_ = 2.f;

					auto c_event_receiver = utils::c_event_receiver();
					struct Capture
					{
						cEventReceiver* er;
						uint off;
					}capture;
					capture.er = c_event_receiver;
					capture.off = element_pos_offset;
					c_event_receiver->mouse_listeners.add([](void* c, KeyStateFlags action, MouseKey key, const Vec2i& pos) {
						auto& capture = *(Capture*)c;
						if (capture.er->active && is_mouse_move(action, key) && app.selected)
						{
							auto e = app.selected->get_component(cElement);
							if (e)
							{
								e->set_y(pos.y(), true);
								if (app.inspector)
									app.inspector->update_data_tracker(FLAME_CHASH("cElement"), capture.off);
							}
						}
					}, new_mail(&capture));

					{
						auto c_style = utils::c_style_color();
						c_style->color_normal = Vec4c(100, 100, 100, 128);
						c_style->color_hovering = Vec4c(50, 50, 50, 190);
						c_style->color_active = Vec4c(80, 80, 80, 255);
						c_style->style();
					}
				}
				utils::pop_parent();
			}

			e_tool->get_component(cCombobox)->data_changed_listeners.add([](void* c, Component* cb, uint hash, void*) {
				if (hash == FLAME_CHASH("index"))
					(*(cOverlayer**)c)->tool_type = ((cCombobox*)cb)->idx;
			}, new_mail_p(c_overlayer));
		}
		utils::pop_parent();

		utils::e_end_layout();

	utils::e_end_docker_page();
}

cEditor::~cEditor()
{
	app.editor = nullptr;
}

void cEditor::search_hover(Entity* e)
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
	if (app.selected)
		return;

	auto element = e->get_component(cElement);
	if (element && rect_contains(element->cliped_rect, mpos))
		app.selected = e;
}
