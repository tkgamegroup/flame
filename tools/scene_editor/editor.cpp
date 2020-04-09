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
			utils::e_begin_combobox();
				utils::e_combobox_item(L"2D");
				utils::e_combobox_item(L"3D");
			utils::e_end_combobox(0);
			{
				auto c_combobox = utils::e_begin_combobox()->get_component(cCombobox);
					utils::e_combobox_item(L"Select");
					utils::e_combobox_item(L"Move");
					utils::e_combobox_item(L"Scale");
				utils::e_end_combobox(0);
				c_combobox->data_changed_listeners.add([](void* c, uint hash, void*) {
					if (hash == FLAME_CHASH("index"))
						app.editor->tool_type = (*(cCombobox**)c)->index;
					return true;
				}, Mail::from_p(c_combobox));
			}
		utils::e_end_layout();

		utils::e_begin_layout();
		utils::c_aligner(SizeFitParent, SizeFitParent);
		{
			auto c_element = utils::current_entity()->get_component(cElement);
			c_element->clip_flags = ClipSelf | ClipChildren;
			c_element->cmds.add([](void* c, graphics::Canvas* canvas) {
				auto element = *(cElement**)c;
				auto base_element = app.editor->c_base_element;

				if (element->clipped)
					return true;

				if (app.editor->selecting)
				{
					std::vector<Vec2f> points;
					auto p0 = app.editor->c_base_element->global_pos;
					auto s = app.editor->scale * 0.1f;
					auto p1 = app.editor->select_anchor_begin * s + p0;
					auto p2 = app.editor->select_anchor_end * s + p0;
					path_rect(points, p1, p2 - p1);
					points.push_back(points[0]);
					canvas->stroke(points.size(), points.data(), Vec4c(17, 193, 101, 255), 2.f);
				}

				auto scale = base_element->global_scale;
				auto line_width = 3.f * scale;

				{
					const auto grid_size = 50.f * scale;
					auto pos = base_element->global_pos;
					auto size = element->global_size + grid_size * 2.f;
					auto grid_number = Vec2i(size / grid_size) + 2;
					auto grid_offset = element->global_pos + (fract(pos / grid_size) - 1.f) * grid_size;
					for (auto x = 0; x < grid_number.x(); x++)
					{
						std::vector<Vec2f> points;
						points.push_back(grid_offset + Vec2f(x * grid_size, 0.f));
						points.push_back(grid_offset + Vec2f(x * grid_size, size.y()));
						canvas->stroke(points.size(), points.data(), Vec4c(200, 200, 200, 255), 1.f);
					}
					for (auto y = 0; y < grid_number.y(); y++)
					{
						std::vector<Vec2f> points;
						points.push_back(grid_offset + Vec2f(0.f, y * grid_size));
						points.push_back(grid_offset + Vec2f(size.x(), y * grid_size));
						canvas->stroke(points.size(), points.data(), Vec4c(200, 200, 200, 255), 1.f);
					}
				}

				return true;
			}, Mail::from_p(c_element));
			{
				auto c_event_receiver = utils::c_event_receiver();
				c_event_receiver->focus_type = FocusByLeftOrRightButton;
				c_event_receiver->mouse_listeners.add([](void* c, KeyStateFlags action, MouseKey key, const Vec2i& pos) {
					if (is_mouse_scroll(action, key))
						app.editor->base_scale((pos.x() > 0.f ? 1 : -1));
					else if (is_mouse_down(action, key, true) && key == Mouse_Left)
					{
						app.s_2d_renderer->pending_update = true;

						//app.deselect();

						app.editor->selecting = true;
						app.editor->select_anchor_begin = (Vec2f(pos) - app.editor->c_base_element->global_pos) / (app.editor->scale * 0.1f);
						app.editor->select_anchor_end = app.editor->select_anchor_begin;
					}
					else if (is_mouse_down(action, key, true) && key == Mouse_Right)
						app.editor->base_moved = false;
					else if (is_mouse_up(action, key, true) && key == Mouse_Right)
					{
						if (!app.editor->base_moved)
							/*app.editor->show_add_node_menu(Vec2f(pos))*/;
					}
					else if (is_mouse_move(action, key))
					{
						if (app.editor->selecting)
						{
							app.s_2d_renderer->pending_update = true;

							app.editor->select_anchor_end = (Vec2f(app.s_event_dispatcher->mouse_pos) - app.editor->c_base_element->global_pos) / (app.editor->scale * 0.1f);
						}

						if (app.s_event_dispatcher->mouse_buttons[Mouse_Right] & KeyStateDown)
							app.editor->base_move(Vec2f(pos));
					}
					return true;
				}, Mail());
				c_event_receiver->state_listeners.add([](void* c, EventReceiverStateFlags state) {
					if (!(state & EventReceiverActive) && app.editor->selecting)
					{
						app.s_2d_renderer->pending_update = true;

						app.editor->selecting = false;
						if (app.editor->select_anchor_begin == app.editor->select_anchor_end)
							return true;

						auto p0 = app.editor->c_base_element->global_pos;
						auto s = app.editor->scale * 0.1f;
						auto r = rect_from_points(app.editor->select_anchor_begin * s + p0, app.editor->select_anchor_end * s + p0);

					}
					return true;
				}, Mail::from_p(c_element));
			}
		}

			e_base = utils::e_element();
			c_base_element = e_base->get_component(cElement);

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
			}
			utils::c_aligner(SizeFitParent, SizeFitParent);
			utils::push_parent(e_overlayer);
				utils::e_empty();
				c_transform_tool_element = utils::c_element();
				c_transform_tool_element->size_ = 20.f;
				c_transform_tool_element->frame_thickness_ = 2.f;
				{
					auto c_event_receiver = utils::c_event_receiver();
					c_event_receiver->mouse_listeners.add([](void* c, KeyStateFlags action, MouseKey key, const Vec2i& pos) {
						auto er = *(cEventReceiver**)c;
						if (utils::is_active(er) && is_mouse_move(action, key) && app.selected)
						{
							auto e = app.selected->get_component(cElement);
							if (e)
								e->set_pos(Vec2f(pos), true);
						}
						return true;
					}, Mail::from_t(c_event_receiver));
				}
				{
					auto c_style = utils::c_style_color();
					c_style->color_normal = Vec4c(100, 100, 100, 128);
					c_style->color_hovering = Vec4c(50, 50, 50, 190);
					c_style->color_active = Vec4c(80, 80, 80, 255);
					c_style->style();
				}

				utils::push_parent(utils::current_entity());
					utils::e_empty();
					{
						auto c_element = utils::c_element();
						c_element->pos_ = Vec2f(25.f, 5.f);
						c_element->size_ = Vec2f(20.f, 10.f);
						c_element->frame_thickness_ = 2.f;

						auto c_event_receiver = utils::c_event_receiver();
						c_event_receiver->mouse_listeners.add([](void* c, KeyStateFlags action, MouseKey key, const Vec2i& pos) {
							auto er = *(cEventReceiver**)c;
							if (utils::is_active(er) && is_mouse_move(action, key) && app.selected)
							{
								auto e = app.selected->get_component(cElement);
								if (e)
									e->set_x(pos.x(), true);
							}
							return true;
						}, Mail::from_t(c_event_receiver));

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
						c_element->pos_ = Vec2f(5.f, 25.f);
						c_element->size_ = Vec2f(10.f, 20.f);
						c_element->frame_thickness_ = 2.f;

						auto c_event_receiver = utils::c_event_receiver();
						c_event_receiver->mouse_listeners.add([](void* c, KeyStateFlags action, MouseKey key, const Vec2i& pos) {
							auto er = *(cEventReceiver**)c;
							if (utils::is_active(er) && is_mouse_move(action, key) && app.selected)
							{
								auto e = app.selected->get_component(cElement);
								if (e)
									e->set_y(pos.y(), true);
							}
							return true;
						}, Mail::from_t(c_event_receiver));

						{
							auto c_style = utils::c_style_color();
							c_style->color_normal = Vec4c(100, 100, 100, 128);
							c_style->color_hovering = Vec4c(50, 50, 50, 190);
							c_style->color_active = Vec4c(80, 80, 80, 255);
							c_style->style();
						}
					}
				utils::pop_parent();
			utils::pop_parent();

			scale = 10;
			c_scale_text = utils::e_text(L"100%")->get_component(cText);
			utils::c_aligner(AlignxLeft, AlignyBottom);

		utils::e_end_layout();

	utils::e_end_docker_page();

	selecting = false;

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

void cEditor::base_scale(int v)
{
	scale += v;
	if (scale < 1 || scale > 10)
		scale -= v;
	else
	{
		auto p = (Vec2f(app.s_event_dispatcher->mouse_pos) - c_base_element->global_pos) / ((scale - v) * 0.1f);
		c_base_element->set_pos(float(v) * p * -0.1f, true);
		c_base_element->set_scale(scale * 0.1f);
		c_scale_text->set_text((std::to_wstring(scale * 10) + L"%").c_str());
	}
}

void cEditor::base_move(const Vec2f& p)
{
	c_base_element->set_pos(p, true);
	base_moved = true;
}
