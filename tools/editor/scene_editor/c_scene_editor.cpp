#include "scene_editor.h"

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
	auto& ui = scene_editor.window->ui;

	auto create_block = [&]() {
		auto b = ui.e_element()->get_component(cElement);
		b->size = vec2(8.f);
		b->pivot = 0.5f;
		b->frame_thickness = 2.f;
		b->color = cvec4(255, 255, 255, 255);
		b->frame_color = cvec4(0, 0, 0, 255);
		b->entity->set_visible(false);
		return b;
	};

	block_c = create_block();
	{
		auto er = ui.c_receiver();
		er->mouse_listeners.add([](Capture& c, KeyStateFlags action, MouseKey key, const ivec2& pos) {
			if (c.current<cReceiver>()->is_active() && is_mouse_move(action, key))
			{
				auto thiz = c.thiz<_2DGizmo>();
				thiz->target->add_pos(vec2(pos) / thiz->base->scale);
			}
			return true;
		}, Capture().set_thiz(this));
		er->state_listeners.add([](Capture& c, EventReceiverState s) {
			c.current<cReceiver>()->dispatcher->window->set_cursor(s ? CursorSizeAll : CursorArrow);
			return true;
		}, Capture());
	}

	block_l = create_block();
	{
		auto er = ui.c_receiver();
		er->mouse_listeners.add([](Capture& c, KeyStateFlags action, MouseKey key, const ivec2& pos) {
			if (c.current<cReceiver>()->is_active() && is_mouse_move(action, key))
			{
				auto thiz = c.thiz<_2DGizmo>();
				auto x = pos.x / thiz->base->scale;
				thiz->target->add_x(x);
				thiz->target->add_width(-x);
			}
			return true;
		}, Capture().set_thiz(this));
		er->state_listeners.add([](Capture& c, EventReceiverState s) {
			c.current<cReceiver>()->dispatcher->window->set_cursor(s ? CursorSizeWE : CursorArrow);
			return true;
		}, Capture());
	}
	block_t = create_block();
	{
		auto er = ui.c_receiver();
		er->mouse_listeners.add([](Capture& c, KeyStateFlags action, MouseKey key, const ivec2& pos) {
			if (c.current<cReceiver>()->is_active() && is_mouse_move(action, key))
			{
				auto thiz = c.thiz<_2DGizmo>();
				auto y = pos.y / thiz->base->scale;
				thiz->target->add_y(y);
				thiz->target->add_height(-y);
			}
			return true;
		}, Capture().set_thiz(this));
		er->state_listeners.add([](Capture& c, EventReceiverState s) {
			c.current<cReceiver>()->dispatcher->window->set_cursor(s ? CursorSizeNS : CursorArrow);
			return true;
		}, Capture());
	}
	block_r = create_block();
	{
		auto er = ui.c_receiver();
		er->mouse_listeners.add([](Capture& c, KeyStateFlags action, MouseKey key, const ivec2& pos) {
			if (c.current<cReceiver>()->is_active() && is_mouse_move(action, key))
			{
				auto thiz = c.thiz<_2DGizmo>();
				auto x = pos.x / thiz->base->scale;
				thiz->target->add_width(x);
			}
			return true;
		}, Capture().set_thiz(this));
		er->state_listeners.add([](Capture& c, EventReceiverState s) {
			c.current<cReceiver>()->dispatcher->window->set_cursor(s ? CursorSizeWE : CursorArrow);
			return true;
		}, Capture());
	}
	block_b = create_block();
	{
		auto er = ui.c_receiver();
		er->mouse_listeners.add([](Capture& c, KeyStateFlags action, MouseKey key, const ivec2& pos) {
			if (c.current<cReceiver>()->is_active() && is_mouse_move(action, key))
			{
				auto thiz = c.thiz<_2DGizmo>();
				auto y = pos.y / thiz->base->scale;
				thiz->target->add_height(y);
			}
			return true;
		}, Capture().set_thiz(this));
		er->state_listeners.add([](Capture& c, EventReceiverState s) {
			c.current<cReceiver>()->dispatcher->window->set_cursor(s ? CursorSizeNS : CursorArrow);
			return true;
		}, Capture());
	}
	block_lt = create_block();
	{
		auto er = ui.c_receiver();
		er->mouse_listeners.add([](Capture& c, KeyStateFlags action, MouseKey key, const ivec2& pos) {
			if (c.current<cReceiver>()->is_active() && is_mouse_move(action, key))
			{
				auto thiz = c.thiz<_2DGizmo>();
				auto p = vec2(pos) / thiz->base->scale;
				thiz->target->add_pos(p);
				thiz->target->add_size(-p);
			}
			return true;
		}, Capture().set_thiz(this));
		er->state_listeners.add([](Capture& c, EventReceiverState s) {
			c.current<cReceiver>()->dispatcher->window->set_cursor(s ? CursorSizeNWSE : CursorArrow);
			return true;
		}, Capture());
	}
	block_rt = create_block();
	{
		auto er = ui.c_receiver();
		er->mouse_listeners.add([](Capture& c, KeyStateFlags action, MouseKey key, const ivec2& pos) {
			if (c.current<cReceiver>()->is_active() && is_mouse_move(action, key))
			{
				auto thiz = c.thiz<_2DGizmo>();
				auto p = vec2(pos) / thiz->base->scale;
				thiz->target->add_y(p.y);
				thiz->target->add_size(vec2(p.x, -p.y));
			}
			return true;
		}, Capture().set_thiz(this));
		er->state_listeners.add([](Capture& c, EventReceiverState s) {
			c.current<cReceiver>()->dispatcher->window->set_cursor(s ? CursorSizeNESW : CursorArrow);
			return true;
		}, Capture());
	}
	block_lb = create_block();
	{
		auto er = ui.c_receiver();
		er->mouse_listeners.add([](Capture& c, KeyStateFlags action, MouseKey key, const ivec2& pos) {
			if (c.current<cReceiver>()->is_active() && is_mouse_move(action, key))
			{
				auto thiz = c.thiz<_2DGizmo>();
				auto p = vec2(pos) / thiz->base->scale;
				thiz->target->add_x(p.x);
				thiz->target->add_size(vec2(-p.x, p.y));
			}
			return true;
		}, Capture().set_thiz(this));
		er->state_listeners.add([](Capture& c, EventReceiverState s) {
			c.current<cReceiver>()->dispatcher->window->set_cursor(s ? CursorSizeNESW : CursorArrow);
			return true;
		}, Capture());
	}
	block_rb = create_block();
	{
		auto er = ui.c_receiver();
		er->mouse_listeners.add([](Capture& c, KeyStateFlags action, MouseKey key, const ivec2& pos) {
			if (c.current<cReceiver>()->is_active() && is_mouse_move(action, key))
			{
				auto thiz = c.thiz<_2DGizmo>();
				auto p = vec2(pos) / thiz->base->scale;
				thiz->target->add_size(p);
			}
			return true;
		}, Capture().set_thiz(this));
		er->state_listeners.add([](Capture& c, EventReceiverState s) {
			c.current<cReceiver>()->dispatcher->window->set_cursor(s ? CursorSizeNWSE : CursorArrow);
			return true;
		}, Capture());
	}
}

void _2DGizmo::on_select()
{
	if (target)
		target->data_changed_listeners.remove(listener);
	if (!scene_editor.selected)
	{
		show_blocks(false);
		target = nullptr;
		listener = nullptr;
	}
	else
	{
		target = scene_editor.selected->get_component(cElement);
		if (target)
		{
			looper().add_event([](Capture& c) {
				auto thiz = c.thiz<_2DGizmo>();
				thiz->show_blocks(true);
				if (thiz->target)
					thiz->update_blocks();
			}, Capture().set_thiz(this));
			listener = target->data_changed_listeners.add([](Capture& c, uint hash, void*) {
				switch (hash)
				{
				case FLAME_CHASH("global_pos"):
				case FLAME_CHASH("global_scale"):
				case FLAME_CHASH("global_size"):
					c.thiz<_2DGizmo>()->update_blocks();
					break;
				}
				return true;
			}, Capture().set_thiz(this));
		}
	}
}

void _2DGizmo::show_blocks(bool v)
{
	block_c->entity->set_visible(v);
	block_l->entity->set_visible(v);
	block_t->entity->set_visible(v);
	block_r->entity->set_visible(v);
	block_b->entity->set_visible(v);
	block_lt->entity->set_visible(v);
	block_rt->entity->set_visible(v);
	block_lb->entity->set_visible(v);
	block_rb->entity->set_visible(v);
}

void _2DGizmo::update_blocks()
{
	auto p = target->pos * base->scale + base->pos;
	auto s = target->size * base->scale;

	block_c->set_pos(p + s * 0.5f);
	block_l->set_pos(p + vec2(0.f, s.y * 0.5f));
	block_t->set_pos(p + vec2(s.x * 0.5f, 0.f));
	block_r->set_pos(p + vec2(s.x, s.y * 0.5f));
	block_b->set_pos(p + vec2(s.x * 0.5f, s.y));
	block_lt->set_pos(p);
	block_rt->set_pos(p + vec2(s.x, 0.f));
	block_lb->set_pos(p + vec2(0.f, s.y));
	block_rb->set_pos(p + s);
}

cSceneEditor::cSceneEditor() :
	Component("cSceneEditor")
{
	auto& ui = scene_editor.window->ui;

	auto e_page = ui.e_begin_docker_page(L"Editor").second;
	{
		auto c_layout = ui.c_layout(LayoutVertical);
		c_layout->item_padding = 4.f;
		c_layout->width_fit_children = false;
		c_layout->height_fit_children = false;

		e_page->add_component(this);
	}
	
		ui.e_begin_layout(LayoutHorizontal, 4.f);
			ui.e_begin_combobox();
				ui.e_combobox_item(L"2D");
				ui.e_combobox_item(L"3D");
			ui.e_end_combobox(0);
			tool_type = 1;
			ui.e_begin_combobox();
				ui.e_combobox_item(L"Select");
				ui.e_combobox_item(L"Gizmo");
			ui.e_end_combobox(tool_type)->get_component(cCombobox)->data_changed_listeners.add([](Capture& c, uint hash, void*) {
				if (hash == FLAME_CHASH("index"))
					scene_editor.editor->tool_type = c.current<cCombobox>()->index;
				return true;
			}, Capture());
		ui.e_end_layout();

		edt.create(ui, [](Capture&, const vec4& r) {
			if (r.x == r.z && r.y == r.z)
				scene_editor.select(nullptr);
			else
				scene_editor.select(scene_editor.editor->search_hovering(r));
		}, Capture());
		edt.scale_level_max = 20;

			{
				auto e_overlay = edt.overlay->entity;

				edt.overlay->cmds.add([](Capture& c, graphics::Canvas* canvas) {
					auto element = c.thiz<cElement>();
					if (!element->clipped && scene_editor.selected)
					{
						auto se = scene_editor.selected->get_component(cElement);
						if (se)
						{
							std::vector<vec2> points;
							path_rect(points, se->global_pos, se->global_size);
							points.push_back(points[0]);
							canvas->stroke(points.size(), points.data(), cvec4(0, 0, 0, 255), 2.f);
						}
					}
					return true;
				}, Capture().set_thiz(edt.overlay));
				ui.current_entity = e_overlay;
				auto c_receiver = e_overlay->get_component(cReceiver);
				c_receiver->pass_checkers.add([](Capture&, cReceiver*, bool* pass) {
					*pass = true;
					return true;
				}, Capture());
				c_receiver->mouse_listeners.add([](Capture& c, KeyStateFlags action, MouseKey key, const ivec2& pos) {
					if (is_mouse_down(action, key, true) && key == Mouse_Left)
						scene_editor.select(scene_editor.editor->search_hovering(vec4(vec2(pos), vec2(pos))));
					return true;
				}, Capture());

				ui.parents.push(e_overlay);
					gizmo.create();
					gizmo.base = edt.base;
				ui.parents.pop();
			}

	ui.e_end_docker_page();
}

cSceneEditor::~cSceneEditor()
{
	scene_editor.editor = nullptr;
}

Entity* cSceneEditor::search_hovering(const vec4& r)
{
	Entity* s = nullptr;
	if (scene_editor.prefab)
		search_hovering_r(scene_editor.prefab, s, r);
	return s;
}

void cSceneEditor::search_hovering_r(Entity* e, Entity*& s, const vec4& r)
{
	if (e->children.s > 0)
	{
		for (auto i = (int)e->children.s - 1; i >= 0; i--)
		{
			auto c = e->children[i];
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

void cSceneEditor::on_select()
{
	gizmo.on_select();
}
