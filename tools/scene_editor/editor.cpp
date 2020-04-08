#include "app.h"

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
			auto e_tool = utils::e_begin_combobox();
				utils::e_combobox_item(L"Null");
				utils::e_combobox_item(L"Move");
				utils::e_combobox_item(L"Scale");
			utils::e_end_combobox(0);
		utils::e_end_layout();

		utils::e_begin_layout()->get_component(cElement)->clip_flags = ClipChildren;
		utils::c_aligner(SizeFitParent, SizeFitParent);
		e_scene = utils::current_entity();
		if (app.prefab)
			e_scene->add_child(app.prefab);

		auto e_overlayer = utils::e_element();
		{
			auto c_element = e_overlayer->get_component(cElement);
			c_element->cmds.add([](void* c, graphics::Canvas* canvas) {
				auto element = *(cElement**)c;
				if (!element->clipped && app.selected)
				{
					auto se = app.selected->get_component(cElement);
					if (se)
					{
						std::vector<Vec2f> points;
						path_rect(points, se->global_pos, se->global_size);
						points.push_back(points[0]);
						canvas->stroke(points.size(), points.data(), Vec4c(255, 255, 255, 255), 6.f);

						if (app.editor->tool_type > 0)
							app.editor->c_transform_tool_element->set_pos(se->center() - element->global_pos - app.editor->c_transform_tool_element->size_ * 0.5f);
					}
				}
				else
					app.editor->c_transform_tool_element->set_pos(Vec2f(-200.f));
				return true;
			}, Mail::from_p(c_element));
		}
		auto c_event_receiver = utils::c_event_receiver();
		c_event_receiver->pass_checkers.add([](void*, cEventReceiver*, bool* pass) {
			*pass = true;
			return true;
		}, Mail());
		c_event_receiver->mouse_listeners.add([](void* c, KeyStateFlags action, MouseKey key, const Vec2i& pos) {
			if (is_mouse_down(action, key, true) && key == Mouse_Left)
			{
				looper().add_event([](void* c, bool*) {
					auto prev_selected = app.selected;
					app.selected = nullptr;
					if (app.prefab)
						app.editor->search_hover(app.prefab);
					if (prev_selected != app.selected)
					{
						if (app.hierarchy)
							app.hierarchy->refresh_selected();
						if (app.inspector)
							app.inspector->refresh();
					}
				}, Mail());
			}
			return true;
		}, Mail());
		utils::c_aligner(SizeFitParent, SizeFitParent);
		utils::push_parent(e_overlayer);
		{
			auto e_transform_tool = utils::e_empty();
			c_transform_tool_element = utils::c_element();
			c_transform_tool_element->size_ = 20.f;
			c_transform_tool_element->frame_thickness_ = 2.f;
			{
				auto c_event_receiver = utils::c_event_receiver();
				struct Capture
				{
					cEventReceiver* er;
				}capture;
				capture.er = c_event_receiver;
				c_event_receiver->mouse_listeners.add([](void* c, KeyStateFlags action, MouseKey key, const Vec2i& pos) {
					auto& capture = *(Capture*)c;
					if (utils::is_active(capture.er) && is_mouse_move(action, key) && app.selected)
					{
						auto e = app.selected->get_component(cElement);
						if (e)
							e->set_pos(Vec2f(pos), true);
					}
					return true;
				}, Mail::from_t(&capture));
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
					}capture;
					capture.er = c_event_receiver;
					c_event_receiver->mouse_listeners.add([](void* c, KeyStateFlags action, MouseKey key, const Vec2i& pos) {
						auto& capture = *(Capture*)c;
						if (utils::is_active(capture.er) && is_mouse_move(action, key) && app.selected)
						{
							auto e = app.selected->get_component(cElement);
							if (e)
								e->set_x(pos.x(), true);
						}
						return true;
					}, Mail::from_t(&capture));

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
					}capture;
					capture.er = c_event_receiver;
					c_event_receiver->mouse_listeners.add([](void* c, KeyStateFlags action, MouseKey key, const Vec2i& pos) {
						auto& capture = *(Capture*)c;
						if (utils::is_active(capture.er) && is_mouse_move(action, key) && app.selected)
						{
							auto e = app.selected->get_component(cElement);
							if (e)
								e->set_y(pos.y(), true);
						}
						return true;
					}, Mail::from_t(&capture));

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

			{
				auto c_combobox = e_tool->get_component(cCombobox);
				c_combobox->data_changed_listeners.add([](void* c, uint hash, void*) {
					if (hash == FLAME_CHASH("index"))
						app.editor->tool_type = (*(cCombobox**)c)->index;
					return true;
				}, Mail::from_p(c_combobox));
			}
		}
		utils::pop_parent();

		utils::e_end_layout();

	utils::e_end_docker_page();

	tool_type = 0;
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
			if (c->global_visibility)
				search_hover(c);
		}
	}
	if (app.selected)
		return;

	auto element = e->get_component(cElement);
	if (element && rect_contains(element->clipped_rect, Vec2f(app.s_event_dispatcher->mouse_pos)))
		app.selected = e;
}
