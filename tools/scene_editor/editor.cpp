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
					utils::e_combobox_item(L"Gizmo");
				utils::e_end_combobox(1);
				c_combobox->data_changed_listeners.add([](void* c, uint hash, void*) {
					if (hash == FLAME_CHASH("index"))
						app.editor->tool_type = (*(cCombobox**)c)->index;
					return true;
				}, Mail::from_p(c_combobox));
			}
		utils::e_end_layout();

		edt.create([](void*, const Vec4f& r) {
			if (r.x() == r.z() && r.y() == r.z())
			{
				if (app.selected)
				{
					app.selected = nullptr;
					if (app.hierarchy)
						app.hierarchy->refresh_selected();
					if (app.inspector)
						app.inspector->refresh();
				}
			}
			else
			{
				//std::vector<BP::Node*> nodes;
				//for (auto i = 0; i < app.bp->node_count(); i++)
				//{
				//	auto n = app.bp->node(i);
				//	auto e = ((Entity*)n->user_data)->get_component(cElement);
				//	if (rect_overlapping(r, rect(e->global_pos, e->global_size)))
				//		nodes.push_back(n);
				//}
				//if (!nodes.empty())
				//{
				//	app.select(nodes);
				//	return;
				//}
			}
		}, Mail());

			auto e_overlay = edt.overlay->entity;
			{
				edt.overlay->cmds.add([](void* c, graphics::Canvas* canvas) {
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
								app.editor->c_transform_tool_element->set_pos(se->center() - element->global_pos - app.editor->c_transform_tool_element->size * 0.5f);
						}
					}
					else
						app.editor->c_transform_tool_element->set_pos(Vec2f(-200.f));
					return true;
				}, Mail::from_p(edt.overlay));
				utils::set_current_entity(e_overlay);
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
			utils::push_parent(e_overlay);
				utils::e_empty();
				c_transform_tool_element = utils::c_element();
				c_transform_tool_element->size = 20.f;
				c_transform_tool_element->frame_thickness = 2.f;
				{
					auto c_event_receiver = utils::c_event_receiver();
					c_event_receiver->mouse_listeners.add([](void* c, KeyStateFlags action, MouseKey key, const Vec2i& pos) {
						auto er = *(cEventReceiver**)c;
						if (utils::is_active(er) && is_mouse_move(action, key) && app.selected)
						{
							auto e = app.selected->get_component(cElement);
							if (e)
								e->add_pos(Vec2f(pos));
						}
						return true;
					}, Mail::from_t(c_event_receiver));
				}

				utils::push_parent(utils::current_entity());
					utils::e_empty();
					{
						auto c_element = utils::c_element();
						c_element->pos = Vec2f(25.f, 5.f);
						c_element->size = Vec2f(20.f, 10.f);
						c_element->frame_thickness = 2.f;

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
					}
					utils::e_empty();
					{
						auto c_element = utils::c_element();
						c_element->pos = Vec2f(5.f, 25.f);
						c_element->size = Vec2f(10.f, 20.f);
						c_element->frame_thickness = 2.f;

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
					}
				utils::pop_parent();
			utils::pop_parent();

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
