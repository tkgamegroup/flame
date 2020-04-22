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
			e_layout->get_component(cElement)->size = 300.f;
			{
				auto cl = e_layout->get_component(cLayout);
				cl->width_fit_children = false;
				cl->height_fit_children = false;
				cl->fence = -1;
			}

			struct cReflector : Component
			{
				bool recording;
				cText* txt_mouse;
				cText* txt_record;
				cText* txt_under_mouse;
				cText* txt_hovering;
				cText* txt_focusing;
				cText* txt_drag_overing;
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
						{
							std::wstring str = L"Hovering: ";
							auto hovering = s_event_dispatcher->hovering;
							auto e = hovering ? hovering->entity : nullptr;
							if (e->is_child_of(e_window))
								e = nullptr;
							str += wfmt(L"0x%016I64X", (ulonglong)e);
							txt_hovering->set_text(str.c_str());
						}
						{
							auto color = style_4c(TextColorNormal);
							std::wstring str = L"Focusing: ";
							auto focusing = s_event_dispatcher->focusing;
							auto e = focusing ? focusing->entity : nullptr;
							if (e->is_child_of(e_window))
								e = nullptr;
							str += wfmt(L"0x%016I64X", (ulonglong)e);
							if (e)
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
							txt_focusing->set_color(color);
							txt_focusing->set_text(str.c_str());
						}
						{
							std::wstring str = L"Drag Overing: ";
							auto drag_overing = s_event_dispatcher->drag_overing;
							auto e = drag_overing ? drag_overing->entity : nullptr;
							if (e->is_child_of(e_window))
								e = nullptr;
							str += wfmt(L"0x%016I64X", (ulonglong)e);
							txt_drag_overing->set_text(str.c_str());
						}
					}
				}

				void processing_2d_renderer_event()
				{
					if (s_2d_renderer->pending_update)
					{
						auto highlight = [&](Entity* e) {
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
						auto hovering = s_event_dispatcher->hovering;
						if (hovering)
							highlight(hovering->entity);
						if (c_tree->selected)
							highlight(c_tree->selected->child(0));
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
			};
			auto c_reflector = new_object<cReflector>();

			c_reflector->recording = true;
			c_reflector->txt_mouse = e_text(nullptr)->get_component(cText);
			c_reflector->txt_record = e_text(L"Stop Recording (Esc)")->get_component(cText);
			c_reflector->txt_hovering = e_text(nullptr)->get_component(cText);
			c_reflector->txt_focusing = e_text(nullptr)->get_component(cText);
			c_reflector->txt_drag_overing = e_text(nullptr)->get_component(cText);
			c_reflector->e_window = e_window;
			c_reflector->e_tree = Entity::create();

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
			}, Mail::from_p(c_reflector));

			next_element_padding = 4.f;
			next_element_frame_thickness = 2.f;
			next_element_frame_color = style_4c(ForegroundColor);
			e_begin_scrollbar(ScrollbarVertical, true);
				next_entity = c_reflector->e_tree;
				e_begin_tree(true);
				e_end_tree();
			e_end_scrollbar();
			c_reflector->c_tree = c_reflector->e_tree->get_component(cTree);

			push_parent(c_reflector->e_tree);
			c_reflector->add_node(current_root());
			pop_parent();

			e_size_dragger();

			current_parent()->add_component(c_reflector);

			e_end_window();

			pop_parent();
			return e_window;
		}
	}
}
