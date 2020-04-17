#include "app.h"

_2DGizmo::_2DGizmo() :
	base(nullptr),
	block_c(nullptr),
	block_l(nullptr),
	block_t(nullptr),
	block_r(nullptr),
	block_b(nullptr),
	block_lt(nullptr),
	block_rt(nullptr),
	block_lb(nullptr),
	block_rb(nullptr),
	target(nullptr),
	listener(nullptr)
{
}

void _2DGizmo::create()
{
	auto create_block = []() {
		auto b = utils::e_element()->get_component(cElement);
		b->size = Vec2f(8.f);
		b->pivot = 0.5f;
		b->frame_thickness = 2.f;
		b->color = Vec4c(255, 255, 255, 255);
		b->frame_color = Vec4c(0, 0, 0, 255);
		b->entity->set_visible(false);
		return b;
	};

	block_c = create_block();
	{
		auto er = utils::c_event_receiver();
		er->mouse_listeners.add([](void* c, KeyStateFlags action, MouseKey key, const Vec2i& pos) {
			if (utils::is_active(cEventReceiver::current()) && is_mouse_move(action, key))
			{
				auto thiz = *(_2DGizmo**)c;
				thiz->target->add_pos(Vec2f(pos) / thiz->base->scale);
			}
			return true;
		}, Mail::from_p(this));
		er->state_listeners.add([](void*, EventReceiverState s) {
			cEventReceiver::current()->dispatcher->window->set_cursor(s ? CursorSizeAll : CursorArrow);
			return true;
		}, Mail());
	}

	block_l = create_block();
	{
		auto er = utils::c_event_receiver();
		er->mouse_listeners.add([](void* c, KeyStateFlags action, MouseKey key, const Vec2i& pos) {
			if (utils::is_active(cEventReceiver::current()) && is_mouse_move(action, key))
			{
				auto thiz = *(_2DGizmo**)c;
				auto x = pos.x() / thiz->base->scale;
				thiz->target->set_x(x, true);
				thiz->target->set_width(-x, true);
			}
			return true;
		}, Mail::from_p(this));
		er->state_listeners.add([](void*, EventReceiverState s) {
			cEventReceiver::current()->dispatcher->window->set_cursor(s ? CursorSizeWE : CursorArrow);
			return true;
		}, Mail());
	}
	block_t = create_block();
	{
		auto er = utils::c_event_receiver();
		er->mouse_listeners.add([](void* c, KeyStateFlags action, MouseKey key, const Vec2i& pos) {
			if (utils::is_active(cEventReceiver::current()) && is_mouse_move(action, key))
			{
				auto thiz = *(_2DGizmo**)c;
				auto y = pos.y() / thiz->base->scale;
				thiz->target->set_y(y, true);
				thiz->target->set_height(-y, true);
			}
			return true;
		}, Mail::from_p(this));
		er->state_listeners.add([](void*, EventReceiverState s) {
			cEventReceiver::current()->dispatcher->window->set_cursor(s ? CursorSizeNS : CursorArrow);
			return true;
		}, Mail());
	}
	block_r = create_block();
	{
		auto er = utils::c_event_receiver();
		er->mouse_listeners.add([](void* c, KeyStateFlags action, MouseKey key, const Vec2i& pos) {
			if (utils::is_active(cEventReceiver::current()) && is_mouse_move(action, key))
			{
				auto thiz = *(_2DGizmo**)c;
				auto x = pos.x() / thiz->base->scale;
				thiz->target->set_width(x, true);
			}
			return true;
		}, Mail::from_p(this));
		er->state_listeners.add([](void*, EventReceiverState s) {
			cEventReceiver::current()->dispatcher->window->set_cursor(s ? CursorSizeWE : CursorArrow);
			return true;
		}, Mail());
	}
	block_b = create_block();
	{
		auto er = utils::c_event_receiver();
		er->mouse_listeners.add([](void* c, KeyStateFlags action, MouseKey key, const Vec2i& pos) {
			if (utils::is_active(cEventReceiver::current()) && is_mouse_move(action, key))
			{
				auto thiz = *(_2DGizmo**)c;
				auto y = pos.y() / thiz->base->scale;
				thiz->target->set_height(y, true);
			}
			return true;
		}, Mail::from_p(this));
		er->state_listeners.add([](void*, EventReceiverState s) {
			cEventReceiver::current()->dispatcher->window->set_cursor(s ? CursorSizeNS : CursorArrow);
			return true;
		}, Mail());
	}
	block_lt = create_block();
	{
		auto er = utils::c_event_receiver();
		er->mouse_listeners.add([](void* c, KeyStateFlags action, MouseKey key, const Vec2i& pos) {
			if (utils::is_active(cEventReceiver::current()) && is_mouse_move(action, key))
			{
				auto thiz = *(_2DGizmo**)c;
				auto p = Vec2f(pos) / thiz->base->scale;
				thiz->target->add_pos(p);
				thiz->target->add_size(-p);
			}
			return true;
		}, Mail::from_p(this));
		er->state_listeners.add([](void*, EventReceiverState s) {
			cEventReceiver::current()->dispatcher->window->set_cursor(s ? CursorSizeNWSE : CursorArrow);
			return true;
		}, Mail());
	}
	block_rt = create_block();
	{
		auto er = utils::c_event_receiver();
		er->mouse_listeners.add([](void* c, KeyStateFlags action, MouseKey key, const Vec2i& pos) {
			if (utils::is_active(cEventReceiver::current()) && is_mouse_move(action, key))
			{
				auto thiz = *(_2DGizmo**)c;
				auto p = Vec2f(pos) / thiz->base->scale;
				thiz->target->set_y(p.y(), true);
				thiz->target->add_size(Vec2f(p.x(), -p.y()));
			}
			return true;
		}, Mail::from_p(this));
		er->state_listeners.add([](void*, EventReceiverState s) {
			cEventReceiver::current()->dispatcher->window->set_cursor(s ? CursorSizeNESW : CursorArrow);
			return true;
		}, Mail());
	}
	block_lb = create_block();
	{
		auto er = utils::c_event_receiver();
		er->mouse_listeners.add([](void* c, KeyStateFlags action, MouseKey key, const Vec2i& pos) {
			if (utils::is_active(cEventReceiver::current()) && is_mouse_move(action, key))
			{
				auto thiz = *(_2DGizmo**)c;
				auto p = Vec2f(pos) / thiz->base->scale;
				thiz->target->set_x(p.x(), true);
				thiz->target->add_size(Vec2f(-p.x(), p.y()));
			}
			return true;
		}, Mail::from_p(this));
		er->state_listeners.add([](void*, EventReceiverState s) {
			cEventReceiver::current()->dispatcher->window->set_cursor(s ? CursorSizeNESW : CursorArrow);
			return true;
		}, Mail());
	}
	block_rb = create_block();
	{
		auto er = utils::c_event_receiver();
		er->mouse_listeners.add([](void* c, KeyStateFlags action, MouseKey key, const Vec2i& pos) {
			if (utils::is_active(cEventReceiver::current()) && is_mouse_move(action, key))
			{
				auto thiz = *(_2DGizmo**)c;
				auto p = Vec2f(pos) / thiz->base->scale;
				thiz->target->add_size(p);
			}
			return true;
		}, Mail::from_p(this));
		er->state_listeners.add([](void*, EventReceiverState s) {
			cEventReceiver::current()->dispatcher->window->set_cursor(s ? CursorSizeNWSE : CursorArrow);
			return true;
		}, Mail());
	}
}

void _2DGizmo::on_select()
{
	if (target)
		target->data_changed_listeners.remove(listener);
	if (!app.selected)
	{
		block_c->entity->set_visible(false);
		block_l->entity->set_visible(false);
		block_t->entity->set_visible(false);
		block_r->entity->set_visible(false);
		block_b->entity->set_visible(false);
		block_lt->entity->set_visible(false);
		block_rt->entity->set_visible(false);
		block_lb->entity->set_visible(false);
		block_rb->entity->set_visible(false);
		target = nullptr;
		listener = nullptr;
	}
	else
	{
		target = app.selected->get_component(cElement);
		if (target)
		{
			block_c->entity->set_visible(true);
			block_l->entity->set_visible(true);
			block_t->entity->set_visible(true);
			block_r->entity->set_visible(true);
			block_b->entity->set_visible(true);
			block_lt->entity->set_visible(true);
			block_rt->entity->set_visible(true);
			block_lb->entity->set_visible(true);
			block_rb->entity->set_visible(true);
			update_blocks();
			listener = target->data_changed_listeners.add([](void* c, uint hash, void*) {
				switch (hash)
				{
				case FLAME_CHASH("global_pos"):
				case FLAME_CHASH("global_scale"):
				case FLAME_CHASH("global_size"):
					(*(_2DGizmo**)c)->update_blocks();
					break;
				}
				return true;
			}, Mail::from_p(this));
		}
	}
}

void _2DGizmo::update_blocks()
{
	auto p = target->pos * base->scale + base->pos;
	auto s = target->size * base->scale;

	block_c->set_pos(p + s * 0.5f);
	block_l->set_pos(p + Vec2f(0.f, s.y() * 0.5f));
	block_t->set_pos(p + Vec2f(s.x() * 0.5f, 0.f));
	block_r->set_pos(p + Vec2f(s.x(), s.y() * 0.5f));
	block_b->set_pos(p + Vec2f(s.x() * 0.5f, s.y()));
	block_lt->set_pos(p);
	block_rt->set_pos(p + Vec2f(s.x(), 0.f));
	block_lb->set_pos(p + Vec2f(0.f, s.y()));
	block_rb->set_pos(p + s);
}

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
		edt.scale_level_max = 20;

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
					gizmo.create();
					gizmo.base = edt.base;
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

void cEditor::on_select()
{
	gizmo.on_select();
}
