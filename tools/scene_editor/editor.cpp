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
				tool_type = 1;
				auto c_combobox = utils::e_begin_combobox()->get_component(cCombobox);
					utils::e_combobox_item(L"Select");
					utils::e_combobox_item(L"Gizmo");
				utils::e_end_combobox(tool_type);
				c_combobox->data_changed_listeners.add([](void* c, uint hash, void*) {
					if (hash == FLAME_CHASH("index"))
						app.editor->tool_type = (*(cCombobox**)c)->index;
					return true;
				}, Mail::from_p(c_combobox));
			}
		utils::e_end_layout();

		edt.create([](void*, const Vec4f& r) {
			if (r.x() == r.z() && r.y() == r.z())
				app.select(nullptr);
			else
				app.select(app.editor->search_hovering(r));
		}, Mail());

			{
				auto e_overlay = edt.overlay->entity;

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
							canvas->stroke(points.size(), points.data(), Vec4c(0, 0, 0, 255), 2.f);
						}
					}
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
						app.select(app.editor->search_hovering(Vec4f(Vec2f(pos), Vec2f(pos))));
					return true;
				}, Mail());

				utils::push_parent(e_overlay);
					gizmo = utils::e_element()->get_component(cElement);
					gizmo->size = Vec2f(10.f);
					gizmo->frame_thickness = 1.f;
					gizmo->color = Vec4c(255);
					gizmo->frame_color = Vec4c(0, 0, 0, 255);
					gizmo->entity->set_visible(false);
					{
						auto c_event_receiver = utils::c_event_receiver();
						c_event_receiver->mouse_listeners.add([](void* c, KeyStateFlags action, MouseKey key, const Vec2i& pos) {
							if (utils::is_active(*(cEventReceiver**)c) && is_mouse_move(action, key))
								app.editor->gizmo_target->add_pos(Vec2f(pos));
							return true;
						}, Mail::from_p(c_event_receiver));
						c_event_receiver->state_listeners.add([](void*, EventReceiverStateFlags s) {
							app.main_window->w->set_cursor(s ? CursorSizeAll : CursorArrow);
							return true;
						}, Mail());
					}
					gizmo_target = nullptr;
					gizmo_listener = nullptr;
				utils::pop_parent();
			}

	utils::e_end_docker_page();
}

cEditor::~cEditor()
{
	app.editor = nullptr;
}

Entity* cEditor::search_hovering(const Vec4f& r)
{
	Entity* s = nullptr;
	if (app.prefab)
		search_hovering_r(app.prefab, s, r);
	return s;
}

void cEditor::search_hovering_r(Entity* e, Entity*& s, const Vec4f& r)
{
	if (e->child_count() > 0)
	{
		for (auto i = (int)e->child_count() - 1; i >= 0; i--)
		{
			auto c = e->child(i);
			if (c->global_visibility)
				search_hovering_r(c, s, r);
		}
	}
	if (s)
		return;

	auto element = e->get_component(cElement);
	if (element && rect_overlapping(element->clipped_rect, r))
		s = e;
}

void cEditor::update_gizmo()
{
	gizmo->set_pos((gizmo_target->pos + gizmo_target->size * gizmo_target->scale * 0.5f) * edt.base->scale + edt.base->pos - 5.f);
}

void cEditor::on_select()
{
	if (gizmo_target)
		gizmo_target->data_changed_listeners.remove(gizmo_listener);
	if (!app.selected)
	{
		gizmo->entity->set_visible(false);
		gizmo_target = nullptr;
		gizmo_listener = nullptr;
	}
	else
	{
		gizmo_target = app.selected->get_component(cElement);
		if (gizmo_target)
		{
			gizmo->entity->set_visible(true);
			update_gizmo();
			gizmo_listener = gizmo_target->data_changed_listeners.add([](void* c, uint hash, void*) {
				switch (hash)
				{
				case FLAME_CHASH("global_pos"):
				case FLAME_CHASH("global_scale"):
				case FLAME_CHASH("global_size"):
					app.editor->update_gizmo();
					break;
				}
				return true;
			}, Mail());
		}
	}
}
