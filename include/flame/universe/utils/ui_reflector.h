#pragma once

#include <flame/foundation/typeinfo.h>
#include <flame/graphics/canvas.h>
#include <flame/universe/utils/ui.h>

namespace flame
{
	namespace utils
	{
		inline Entity* e_ui_reflector_window()
		{
			push_parent(current_root());
			auto e_window = e_begin_window(L"UI Reflector");
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
					Entity* target;
					Vec4f rect;

					cText* text;
					bool highlight;

					void get_rect()
					{
						if (!target)
						{
							rect = 0.f;
							return;
						}
						auto element = target->get_component(cElement);
						if (!element)
						{
							rect = 0.f;
							return;
						}
						rect = Vec4f(element->global_pos, element->global_size);
					}
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
				Entity* e_detail;

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

				void make_target(const wchar_t* title, Target& t, bool highlight = false)
				{
					t.target = nullptr;
					t.highlight = highlight;

					e_begin_layout(LayoutHorizontal, 4.f);
						e_text(title);
						push_style(FontSize, common(Vec1u(11)));
						struct Capture
						{
							cReflector* thiz;
							Entity** t;
						}capture;
						capture.thiz = this;
						capture.t = &t.target;
						e_button(Icon_HAND_POINTER_O, [](void* c) {
							auto capture = *(Capture*)c;
							Entity* ret = nullptr;
							capture.thiz->find_target_in_tree(capture.thiz->e_tree, *capture.t, ret);
							if (ret)
								capture.thiz->c_tree->set_selected(ret);
							capture.thiz->c_tree->expand_to_selected();
						}, Mail::from_t(&capture));
						e_toggle(Icon_SQUARE_O, highlight)->get_component(cCheckbox)->data_changed_listeners.add([](void* c, uint hash, void*) {
							if (hash == FLAME_CHASH("checked"))
								**(bool**)c = ((cCheckbox*)Component::current())->checked;
							return true;
						}, Mail::from_p(&t.highlight));
						pop_style(FontSize);
						t.text = e_text(nullptr)->get_component(cText);
					e_end_layout();
				}

				void find_undering(Entity* e, const Vec2f& pos)
				{
					for (auto i = (int)e->child_count() - 1; i >= 0; i--)
					{
						auto c = e->child(i);
						if (c && c->get_component(cElement))
							find_undering(c, pos);
					}

					if (t_undering.target)
						return;
					auto element = e->get_component(cElement);
					if (!element)
						return;
					auto event_receiver = e->get_component(cEventReceiver);
					if (event_receiver && event_receiver->pass_checkers.impl->count() > 0)
						return;
					if (!element->clipped && rect_contains(element->clipped_rect, pos))
						t_undering.target = e;
				}

				void find_target_in_tree(Entity* e, Entity* t, Entity*& ret)
				{
					auto dp = e->get_component(cDataKeeper);
					if (dp && dp->get_common_item(FLAME_CHASH("entity")).p == t)
					{
						ret = e;
						return;
					}

					for (auto i = 0; i < e->child_count(); i++)
						find_target_in_tree(e->child(i), t, ret);
				}

				void add_node(Entity* src)
				{
					if (src == e_window)
						return;
					auto element = src->get_component(cElement);
					next_entity = Entity::create();
					auto geometry = Vec4f(element->global_pos, element->global_size);
					auto dp = cDataKeeper::create();
					dp->set_common_item(FLAME_CHASH("entity"), common(src));
					dp->set_common_item(FLAME_CHASH("geometry"), common(geometry));
					std::string desc = sfmt("name: %s\n", src->name());
					auto components = src->get_components();
					for (auto i = 0; i < components.s; i++)
					{
						auto c = components[i];
						auto name = std::string(c->name);
						desc += sfmt("[%s]\n", c->name);
						if (name == "cElement")
						{
							auto ce = (cElement*)c;
							desc += sfmt("pos: %.0f, %0.f\n", ce->pos.x(), ce->pos.y());
							desc += sfmt("size: %.0f, %0.f\n", ce->size.x(), ce->size.y());
							desc += sfmt("scale: %f\n", ce->scale);
							desc += sfmt("global pos: %.0f, %0.f\n", ce->global_pos.x(), ce->global_pos.y());
							desc += sfmt("global size: %.0f, %0.f\n", ce->global_size.x(), ce->global_size.y());
							desc += sfmt("global scale: %f\n", ce->global_scale);
						}
						else if (name == "cAligner")
						{
							auto ca = (cAligner*)c;

						}
					}
					dp->set_string_item(FLAME_CHASH("desc"), desc.c_str());
					next_entity->add_component(dp);
					auto cs = src->child_count();
					auto title = wfmt(L"%I64X ", (ulonglong)src);
					if (cs == 0)
						e_tree_leaf(title.c_str())->set_name("ui_reflector_item_leaf");
					else
					{
						e_begin_tree_node(title.c_str(), true)->child(0)->set_name("ui_reflector_item_title");
						for (auto i = 0; i < cs; i++)
							add_node(src->child(i));
						e_end_tree_node();
					}
				}

				void refresh_detail()
				{
					e_detail->remove_children(0, -1);
					auto selected = c_tree->selected;
					if (selected)
					{
						auto dp = selected->get_component(cDataKeeper);
						push_parent(e_detail);
						e_text(s2w(dp->get_string_item(FLAME_CHASH("desc"))).c_str());
						struct Capture
						{
							cReflector* thiz;
							Entity* e;
						}capture;
						capture.thiz = this;
						capture.e = (Entity*)dp->get_common_item(FLAME_CHASH("entity")).p;
						e_button(L"Debug cElelemnt In Renderer", [](void* c) {
							auto& capture = *(Capture*)c;
							Entity* ret = nullptr;
							capture.thiz->find_target_in_tree(capture.thiz->e_tree, capture.e, ret);
							if (ret)
							{
								auto ce = capture.e->get_component(cElement);
								if (ce)
									ce->debug_level = 1;
							}
						}, Mail::from_t(&capture));
						e_button(L"Debug cLayout In Management", [](void* c) {
							auto& capture = *(Capture*)c;
							Entity* ret = nullptr;
							capture.thiz->find_target_in_tree(capture.thiz->e_tree, capture.e, ret);
							if (ret)
							{
								auto cl = capture.e->get_component(cLayout);
								if (cl)
									cl->debug_level = 1;
							}
						}, Mail::from_t(&capture));
						pop_parent();
					}
				}

				void create(Entity* _window)
				{
					recording = true;
					txt_mouse = e_text(nullptr)->get_component(cText);
					txt_record = e_text(L"Recording (Esc To Stop)")->get_component(cText);
					txt_record->set_color(Vec4c(200, 120, 0, 255));
					make_target(L"Undering", t_undering, true);
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

					e_begin_splitter(SplitterHorizontal);
						next_element_padding = 4.f;
						next_element_frame_thickness = 2.f;
						next_element_frame_color = style(ForegroundColor).c;
						e_begin_scrollbar(ScrollbarVertical, true);
							next_entity = e_tree;
							e_begin_tree(true);
							e_end_tree();
						e_end_scrollbar();
						c_tree = e_tree->get_component(cTree);
						c_tree->data_changed_listeners.add([](void* c, uint hash, void*) {
							if (hash == FLAME_CHASH("selected"))
							{
								auto thiz = *(void**)c;
								looper().add_event([](void* c, bool*) {
									(*(cReflector**)c)->refresh_detail();
								}, Mail::from_p(thiz));
							}
							return true;
						}, Mail::from_p(this));

						next_element_padding = 4.f;
						next_element_frame_thickness = 2.f;
						next_element_frame_color = style(ForegroundColor).c;
						e_begin_scrollbar(ScrollbarVertical, true);
							e_detail = e_begin_layout(LayoutVertical, 4.f, false, false);
							c_aligner(AlignMinMax, AlignMinMax);
							e_end_layout();
						e_end_scrollbar();
					e_end_splitter();

					push_parent(e_tree);
					add_node(current_root());
					pop_parent();
				}

				void processing_event_dispatcher_event()
				{
					if (s_event_dispatcher->key_states[Key_Esc] == (KeyStateDown | KeyStateJust))
					{
						recording = !recording;
						txt_record->set_text(recording ? L"Recording (Esc To Stop)" : L"Stopped (Esc To Start)");
						txt_record->set_color(recording ? Vec4c(200, 120, 0, 255) : style(TextColorNormal).c);
					}

					{
						std::wstring str = L"Mouse: ";
						str += to_wstring(s_event_dispatcher->mouse_pos);
						txt_mouse->set_text(str.c_str());
					}

					if (recording)
					{
						if (s_event_dispatcher->mouse_disp != 0)
						{
							t_undering.target = nullptr;
							find_undering(current_root(), Vec2f(s_event_dispatcher->mouse_pos));
							if (t_undering.target->is_child_of(e_window))
								t_undering.target = nullptr;
							t_undering.get_rect();
							t_undering.text->set_text(wfmt(L"0x%016I64X", (ulonglong)t_undering.target).c_str());
						}

						{
							auto hovering = s_event_dispatcher->hovering;
							t_hovering.target = hovering ? hovering->entity : nullptr;
							if (t_hovering.target->is_child_of(e_window))
								t_hovering.target = nullptr;
							t_hovering.get_rect();
							t_hovering.text->set_text(wfmt(L"0x%016I64X", (ulonglong)t_hovering.target).c_str());
						}
						{
							auto color = style(TextColorNormal).c;
							auto focusing = s_event_dispatcher->focusing;
							t_focusing.target = focusing ? focusing->entity : nullptr;
							if (t_focusing.target->is_child_of(e_window))
								t_focusing.target = nullptr;
							t_focusing.get_rect();
							auto str = wfmt(L"0x%016I64X", (ulonglong)t_focusing.target);
							if (t_focusing.target)
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
							t_drag_overing.target = drag_overing ? drag_overing->entity : nullptr;
							if (t_drag_overing.target->is_child_of(e_window))
								t_drag_overing.target = nullptr;
							t_drag_overing.get_rect();
							t_drag_overing.text->set_text(wfmt(L"0x%016I64X", (ulonglong)t_drag_overing.target).c_str());
						}
					}
				}

				void processing_2d_renderer_event()
				{
					if (s_2d_renderer->pending_update)
					{
						auto highlight = [&](Entity* e, const Vec4c& col) {
							if (!e)
								return;
							cDataKeeper* dp = nullptr;
							if (e->name_hash() == FLAME_CHASH("ui_reflector_item_leaf"))
								dp = e->get_component(cDataKeeper);
							else if (e->name_hash() == FLAME_CHASH("ui_reflector_item_title"))
								dp = e->parent()->get_component(cDataKeeper);
							if (dp)
							{
								auto rect = dp->get_common_item(FLAME_CHASH("geometry")).f;
								std::vector<Vec2f> points;
								path_rect(points, rect.xy(), rect.zw());
								points.push_back(points[0]);
								s_2d_renderer->canvas->stroke(points.size(), points.data(), col, 3.f);
							}
						};
						if (c_tree->selected)
							highlight(c_tree->selected, Vec4c(200, 0, 30, 255));
						auto hovering = s_event_dispatcher->hovering;
						if (hovering)
							highlight(hovering->entity, Vec4c(255, 255, 0, 255));
						if (recording)
						{
							auto highlight = [&](const Vec4f& rect) {
								std::vector<Vec2f> points;
								path_rect(points, rect.xy(), rect.zw());
								points.push_back(points[0]);
								s_2d_renderer->canvas->stroke(points.size(), points.data(), Vec4c(255, 255, 0, 255), 3.f);
							};
							if (t_undering.highlight)
								highlight(t_undering.rect);
							if (t_hovering.highlight)
								highlight(t_hovering.rect);
							if (t_focusing.highlight)
								highlight(t_focusing.rect);
							if (t_drag_overing.highlight)
								highlight(t_drag_overing.rect);
						}
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
