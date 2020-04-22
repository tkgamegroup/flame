#pragma once

#include <flame/universe/utils/ui.h>

namespace flame
{
	namespace utils
	{
		inline Entity* e_reflector_window()
		{
			push_parent(current_root());
			auto e_window = e_begin_window(L"Reflector");
			auto e_layout = current_parent();
			e_layout->get_component(cElement)->size = 400.f;
			{
				auto cl = e_layout->get_component(cLayout);
				cl->width_fit_children = false;
				cl->height_fit_children = false;
				cl->fence = -1;
			}

			struct cReflector : Component
			{
				struct Target
				{
					Entity* t;

					cText* text;
					bool highlight;
				};

				bool recording;
				cText* txt_mouse;
				cText* txt_record;
				Target t_undering;
				Target t_hovering;
				Target t_focusing;
				Target t_drag_overing;
				Entity* e_window;
				Entity* e_tree;
				cTree* c_tree;

				sEventDispatcher* s_event_dispatcher;
				s2DRenderer* s_2d_renderer;
				void* s_event_dispatcher_listener;
				void* s_2d_renderer_listener;

				cReflector() :
					Component("cReflector")
				{
				}

				~cReflector() override
				{
					s_event_dispatcher->after_update_listeners.remove(s_event_dispatcher_listener);
					s_2d_renderer->after_update_listeners.remove(s_2d_renderer_listener);
				}

				void make_target(const wchar_t* title, Target& t)
				{
					t.highlight = false;

					e_begin_layout(LayoutHorizontal, 4.f);
						e_text(title);
						struct Capture
						{
							cReflector* thiz;
							Entity** t;
						}capture;
						capture.thiz = this;
						capture.t = &t.t;
						e_button(Icon_SHARE, [](void* c) {
							auto capture = *(Capture*)c;
							capture.thiz->find_target_in_tree(capture.thiz->e_tree, *capture.t);
						}, Mail::from_t(&capture));
						e_toggle(Icon_SQUARE_O)->get_component(cCheckbox)->data_changed_listeners.add([](void* c, uint hash, void*) {
							if (hash == FLAME_CHASH("checked"))
								**(bool**)c = ((cCheckbox*)Component::current())->checked;
							return true;
						}, Mail::from_p(&t.highlight));
						t.text = e_text(nullptr)->get_component(cText);
					e_end_layout();
				}

				void find_undering_mouse(Entity* e, const Vec2f& pos)
				{
					for (auto i = (int)e->child_count() - 1; i >= 0; i--)
						find_undering_mouse(e->child(i), pos);

					if (t_undering.t)
						return;
					auto element = e->get_component(cElement);
					if (!element)
						return;
					if (rect_contains(rect(element->global_pos, element->global_size), pos))
						t_undering.t = e;
				}

				void find_target_in_tree(Entity* e, Entity* t)
				{
					auto dp = e->get_component(cDataKeeper);
					if (dp && dp->get_voidp_item(FLAME_CHASH("entity")) == t)
					{
						c_tree->set_selected(e->parent());
						return;
					}

					for (auto i = 0; i < e->child_count(); i++)
						find_target_in_tree(e->child(i), t);
				}

				void add_node(Entity* src)
				{
					if (src == e_window)
						return;
					auto element = src->get_component(cElement);
					auto geometry = Vec4f(element->global_pos, element->global_size);
					auto item = e_begin_tree_node(wfmt(L"%I64X ", (ulonglong)src).c_str(), true)->child(0);
					{
						item->set_name("reflector_item");
						auto dp = cDataKeeper::create();
						dp->set_voidp_item(FLAME_CHASH("entity"), src);
						dp->set_vec4f_item(FLAME_CHASH("geometry"), geometry);
						item->add_component(dp);
					}
					e_text((L"name: " + s2w(src->name())).c_str());
					e_text(wfmt(L"geometry: (%.0f, %.0f)-(%.0f, %.0f)", geometry.x(), geometry.y(), geometry.x() + geometry.z(), geometry.y() + geometry.w()).c_str());
					auto cs = src->child_count();
					for (auto i = 0; i < cs; i++)
						add_node(src->child(i));
					e_end_tree_node();
				}

				void create(Entity* _window)
				{
					recording = true;
					txt_mouse = e_text(nullptr)->get_component(cText);
					txt_record = e_text(L"Stop Recording (Esc)")->get_component(cText);
					make_target(L"Undering", t_undering);
					make_target(L"Hovering", t_hovering);
					make_target(L"Focusing", t_focusing);
					make_target(L"Drag Overing", t_drag_overing);
					e_window = _window;
					e_tree = Entity::create();

					e_button(Icon_REFRESH, [](void* c) {
						auto thiz = *(void**)c;
						looper().add_event([](void* c, bool*) {
							auto reflector = *(cReflector**)c;
							reflector->c_tree->set_selected(nullptr);
							reflector->e_tree->remove_children(0, -1);
							push_parent(reflector->e_tree);
							reflector->add_node(current_root());
							pop_parent();
						}, Mail::from_p(thiz));
					}, Mail::from_p(this));

					next_element_padding = 4.f;
					next_element_frame_thickness = 2.f;
					next_element_frame_color = style_4c(ForegroundColor);
					e_begin_scrollbar(ScrollbarVertical, true);
						next_entity = e_tree;
						e_begin_tree(true);
						e_end_tree();
					e_end_scrollbar();
					c_tree = e_tree->get_component(cTree);

					push_parent(e_tree);
					add_node(current_root());
					pop_parent();
				}

				void processing_event_dispatcher_event()
				{
					if (s_event_dispatcher->key_states[Key_Esc] == (KeyStateDown | KeyStateJust))
					{
						recording = !recording;
						txt_record->set_text(recording ? L"Stop Recording (Esc)" : L"Start Recording (Esc)");
					}

					{
						std::wstring str = L"Mouse: ";
						str += to_wstring(s_event_dispatcher->mouse_pos);
						txt_mouse->set_text(str.c_str());
					}

					if (recording)
					{
						t_undering.t = nullptr;
						find_undering_mouse(current_root(), Vec2f(s_event_dispatcher->mouse_pos));
						if (t_undering.t->is_child_of(e_window))
							t_undering.t = nullptr;
						t_undering.text->set_text(wfmt(L"0x%016I64X", (ulonglong)t_undering.t).c_str());

						{
							auto hovering = s_event_dispatcher->hovering;
							t_hovering.t = hovering ? hovering->entity : nullptr;
							if (t_hovering.t->is_child_of(e_window))
								t_hovering.t = nullptr;
							t_hovering.text->set_text(wfmt(L"0x%016I64X", (ulonglong)t_hovering.t).c_str());
						}
						{
							auto color = style_4c(TextColorNormal);
							auto focusing = s_event_dispatcher->focusing;
							t_focusing.t = focusing ? focusing->entity : nullptr;
							if (t_focusing.t->is_child_of(e_window))
								t_focusing.t = nullptr;
							auto str = wfmt(L"0x%016I64X", (ulonglong)t_focusing.t);
							if (t_focusing.t)
							{
								if (focusing == s_event_dispatcher->hovering)
									color = Vec4c(0, 255, 0, 255);
								switch (s_event_dispatcher->focusing_state)
								{
								case FocusingAndActive:
									str += L" Active";
									break;
								case FocusingAndDragging:
									str += L" Dragging";
									break;
								}
							}
							t_focusing.text->set_color(color);
							t_focusing.text->set_text(str.c_str());
						}
						{
							auto drag_overing = s_event_dispatcher->drag_overing;
							t_drag_overing.t = drag_overing ? drag_overing->entity : nullptr;
							if (t_drag_overing.t->is_child_of(e_window))
								t_drag_overing.t = nullptr;
							t_drag_overing.text->set_text(wfmt(L"0x%016I64X", (ulonglong)t_drag_overing.t).c_str());
						}
					}
				}

				void processing_2d_renderer_event()
				{
					if (s_2d_renderer->pending_update)
					{
						auto highlight1 = [&](Entity* e) {
							if (e && e->name_hash() == FLAME_CHASH("reflector_item"))
							{
								auto dp = e->get_component(cDataKeeper);
								auto rect = dp->get_vec4f_item(FLAME_CHASH("geometry"));
								std::vector<Vec2f> points;
								path_rect(points, rect.xy(), rect.zw());
								points.push_back(points[0]);
								s_2d_renderer->canvas->stroke(points.size(), points.data(), Vec4c(255, 255, 0, 255), 3.f);
							}
						};
						auto highlight2 = [&](Entity* e) {
							if (!e)
								return;
							auto element = e->get_component(cElement);
							if (!element)
								return;
							std::vector<Vec2f> points;
							path_rect(points, element->global_pos, element->global_size);
							points.push_back(points[0]);
							s_2d_renderer->canvas->stroke(points.size(), points.data(), Vec4c(255, 255, 0, 255), 3.f);
						};
						auto hovering = s_event_dispatcher->hovering;
						if (hovering)
							highlight1(hovering->entity);
						if (c_tree->selected)
							highlight1(c_tree->selected->child(0));
						if (t_undering.highlight)
							highlight2(t_undering.t);
						if (t_hovering.highlight)
							highlight2(t_hovering.t);
						if (t_focusing.highlight)
							highlight2(t_focusing.t);
						if (t_drag_overing.highlight)
							highlight2(t_drag_overing.t);
					}
				}

				void on_entered_world() override
				{
					s_event_dispatcher = entity->world()->get_system(sEventDispatcher);
					s_event_dispatcher_listener = s_event_dispatcher->after_update_listeners.add([](void* c) {
						(*(cReflector**)c)->processing_event_dispatcher_event();
						return true;
					}, Mail::from_p(this));
					s_2d_renderer = entity->world()->get_system(s2DRenderer);
					s_2d_renderer_listener = s_2d_renderer->after_update_listeners.add([](void* c) {
						(*(cReflector**)c)->processing_2d_renderer_event();
						return true;
					}, Mail::from_p(this));
				}
			};
			auto c_reflector = new_object<cReflector>();
			c_reflector->create(e_window);
			current_parent()->add_component(c_reflector);

			e_size_dragger();

			e_end_window();

			pop_parent();
			return e_window;
		}
	}
}
