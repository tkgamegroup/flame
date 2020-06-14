#pragma once

#include <flame/foundation/typeinfo.h>
#include <flame/graphics/canvas.h>
#include <flame/universe/ui/ui.h>

namespace flame
{
	inline Entity* create_ui_reflector(UI& ui)
	{
		ui.parents.push(ui.current_root);
		auto e_window = ui.e_begin_window(L"UI Reflector");
		auto e_layout = ui.parents.top();
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

			UI* ui;

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
				auto pt = &t;

				ui->e_begin_layout(LayoutHorizontal, 4.f);
				ui->e_text(title);
				ui->push_style(FontSize, common(Vec1u(11)));
				ui->e_button(Icon_HAND_POINTER_O, [](Capture& c) {
					auto pt = c.data<Target*>();
					auto thiz = c.thiz<cReflector>();
					Entity* ret = nullptr;
					thiz->find_target_in_tree(thiz->e_tree, pt->target, ret);
					if (ret)
						thiz->c_tree->set_selected(ret);
					thiz->c_tree->expand_to_selected();
				}, Capture().set_data(&pt).set_thiz(this));
				ui->e_toggle(Icon_SQUARE_O, highlight)->get_component(cCheckbox)->data_changed_listeners.add([](Capture& c, uint hash, void*) {
					if (hash == FLAME_CHASH("checked"))
						c.data<Target*>()->highlight = c.current<cCheckbox>()->checked;
					return true;
				}, Capture().set_data(&pt));
				ui->pop_style(FontSize);
				t.text = ui->e_text(nullptr)->get_component(cText);
				ui->e_end_layout();
			}

			void find_undering(Entity* e, const Vec2f& pos)
			{
				for (auto i = (int)e->children.s - 1; i >= 0; i--)
				{
					auto c = e->children[i];
					if (c->get_component(cElement))
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

				for (auto c : e->children)
					find_target_in_tree(c, t, ret);
			}

			void add_node(Entity* src)
			{
				if (src == e_window)
					return;
				auto element = src->get_component(cElement);
				ui->next_entity = f_new<Entity>();
				auto geometry = Vec4f(element->global_pos, element->global_size);
				auto dp = cDataKeeper::create();
				dp->set_common_item(FLAME_CHASH("entity"), common(src));
				dp->set_common_item(FLAME_CHASH("geometry"), common(geometry));
				std::string desc = sfmt("name: %s\n", src->name.v);
				for (auto c : src->components.get_all())
				{
					auto name = std::string(c->name);
					desc += sfmt("[%s]\n", c->name);
					if (name == "cElement")
					{
						auto ce = (cElement*)c;
						desc += sfmt("pos: %.0f, %0.f\n", ce->pos.x(), ce->pos.y());
						desc += sfmt("size: %.0f, %0.f\n", ce->size.x(), ce->size.y());
						desc += sfmt("scale: %f\n", ce->scale);
						desc += sfmt("padding: %.0f, %.0f, %.0f, %.0f\n", ce->padding.x(), ce->padding.y(), ce->padding.z(), ce->padding.w());
						desc += sfmt("global pos: %.0f, %0.f\n", ce->global_pos.x(), ce->global_pos.y());
						desc += sfmt("global size: %.0f, %0.f\n", ce->global_size.x(), ce->global_size.y());
						desc += sfmt("global scale: %f\n", ce->global_scale);
					}
					else if (name == "cAligner")
					{
						auto ca = (cAligner*)c;
						auto align_flag_info = find_enum(FLAME_CHASH("flame::AlignFlag"));
						auto get_enum_str = [&](EnumInfo* info, int v) {
							std::string ret;
							for (auto item : info->items)
							{
								if (v & item->value)
								{
									ret += item->name.str();
									ret += " ";
								}								
							}
							return ret;
						};
						desc += sfmt("x flags: %s\n", get_enum_str(align_flag_info, ca->x_align_flags).c_str());
						desc += sfmt("y flags: %s\n", get_enum_str(align_flag_info, ca->y_align_flags).c_str());
						desc += sfmt("margin: %.0f, %.0f, %.0f, %.0f\n", ca->margin.x(), ca->margin.y(), ca->margin.z(), ca->margin.w());
					}
					else if (name == "cLayout")
					{
						auto cl = (cLayout*)c;
						auto layout_type_info = find_enum(FLAME_CHASH("flame::LayoutType"));
						desc += sfmt("type: %s\n", layout_type_info->find_item(cl->type)->name.v);
					}
				}
				dp->set_string_item(FLAME_CHASH("desc"), desc.c_str());
				ui->next_entity->add_component(dp);
				auto title = wfmt(L"%I64X ", (ulonglong)src);
				if (src->children.s == 0)
					ui->e_tree_leaf(title.c_str())->name = "ui_reflector_item_leaf";
				else
				{
					ui->e_begin_tree_node(title.c_str(), true)->children[0]->name = "ui_reflector_item_title";
					for (auto c : src->children)
						add_node(c);
					ui->e_end_tree_node();
				}
			}

			void refresh_detail()
			{
				e_detail->remove_children(0, -1);
				auto selected = c_tree->selected;
				if (selected)
				{
					auto dp = selected->get_component(cDataKeeper);
					ui->parents.push(e_detail);
					ui->e_text(s2w(dp->get_string_item(FLAME_CHASH("desc"))).c_str());

					struct Capturing
					{
						cReflector* thiz;
						Entity* e;
					}capture;
					capture.thiz = this;
					capture.e = (Entity*)dp->get_common_item(FLAME_CHASH("entity")).p;
					ui->e_button(L"See Create Stack", [](Capture& c) {
						auto& capture = c.data<Capturing>();
						Entity* ret = nullptr;
						capture.thiz->find_target_in_tree(capture.thiz->e_tree, capture.e, ret);
						if (ret)
						{
							capture.thiz->recording = false;

							auto frames = capture.e->created_stack_;
							auto infos = get_stack_frame_infos(frames.s, frames.v);

							auto ui = capture.thiz->ui;
							auto dialog = ui->e_begin_dialog();
							dialog->get_component(cElement)->size = Vec2f(500.f, 300.f);
							auto cl = dialog->get_component(cLayout);
							cl->width_fit_children = false;
							cl->height_fit_children = false;
							cl->fence = -1;
								auto e_list = f_new<Entity>();

								ui->e_begin_scrollbar(ScrollbarVertical, true);
								ui->next_entity = e_list;
								ui->e_begin_list(true);
								for (auto i = 0; i < infos.s; i++)
								{
									auto& info = infos[i];
									ui->e_list_item(s2w(sfmt("%s\n%s: %d", info.function.v, info.file.v, info.line)).c_str(), AlignMinMax | AlignGreedy);
									auto dk = ui->c_data_keeper();
									dk->set_string_item(FLAME_CHASH("file"), info.file.v);
									dk->set_common_item(FLAME_CHASH("line"), common(Vec1u(info.line)));
								}
								ui->e_end_list();
								ui->e_end_scrollbar();

								ui->e_begin_layout(LayoutHorizontal, 4.f)->get_component(cLayout)->fence = -1;
								ui->c_aligner(AlignMinMax, 0);
								ui->e_button(L"Open In Visual Studio", [](Capture& c) {
									auto selected = c.thiz<Entity>()->get_component(cList)->selected;
									if (selected)
									{
										auto dk = selected->get_component(cDataKeeper);
										auto fn = dk->get_string_item(FLAME_CHASH("file"));
										if (std::filesystem::exists(fn))
										{
											auto cmd = wfmt(L"/edit \"%s\"", s2w(fn).c_str());
											exec(L"devenv", (wchar_t*)cmd.c_str(), true, true);
											set_clipboard(std::to_wstring(dk->get_common_item(FLAME_CHASH("line")).u[0]).c_str());
											looper().add_event([](Capture& c) {
												send_global_key_event(KeyStateDown, Key_Ctrl);
												send_global_key_event(KeyStateDown, Key_G);
												send_global_key_event(KeyStateUp, Key_G);
												send_global_key_event(KeyStateDown, Key_V);
												send_global_key_event(KeyStateUp, Key_V);
												send_global_key_event(KeyStateDown, Key_Enter);
												send_global_key_event(KeyStateUp, Key_Enter);
												send_global_key_event(KeyStateUp, Key_Ctrl);
											}, Capture(), 2.f);
										}
									}
								}, Capture().set_thiz(e_list));
								ui->e_button(L"OK", [](Capture& c) {
									remove_layer(c.thiz<Entity>());
								}, Capture().set_thiz(dialog->parent));
								ui->c_aligner(AlignMax, 0);
								ui->e_end_layout();

								ui->e_size_dragger();
							ui->e_end_dialog();
						}
					}, Capture().set_data(&capture));
					ui->e_button(L"Debug Drawing", [](Capture& c) {
						auto& capture = c.data<Capturing>();
						Entity* ret = nullptr;
						capture.thiz->find_target_in_tree(capture.thiz->e_tree, capture.e, ret);
						if (ret)
						{
							auto ce = capture.e->get_component(cElement);
							if (ce)
							{
								ce->debug_level = 1;
								ce->mark_dirty();
							}
						}
					}, Capture().set_data(&capture));
					ui->e_button(L"Debug Layout", [](Capture& c) {
						auto& capture = c.data<Capturing>();
						Entity* ret = nullptr;
						capture.thiz->find_target_in_tree(capture.thiz->e_tree, capture.e, ret);
						if (ret)
						{
							auto cl = capture.e->get_component(cLayout);
							if (cl)
							{
								cl->debug_level = 1;
								cl->mark_dirty();
							}
						}
					}, Capture().set_data(&capture));
					ui->parents.pop();
				}
			}

			void create(Entity* _window)
			{
				recording = true;
				txt_mouse = ui->e_text(nullptr)->get_component(cText);
				txt_record = ui->e_text(L"Recording (Esc To Stop)")->get_component(cText);
				txt_record->set_color(Vec4c(200, 120, 0, 255));
				make_target(L"Undering", t_undering, true);
				make_target(L"Hovering", t_hovering);
				make_target(L"Focusing", t_focusing);
				make_target(L"Drag Overing", t_drag_overing);
				e_window = _window;
				e_tree = f_new<Entity>();

				ui->e_button(Icon_REFRESH, [](Capture& c) {
					looper().add_event([](Capture& c) {
						auto reflector = c.thiz<cReflector>();
						reflector->c_tree->set_selected(nullptr);
						reflector->e_tree->remove_children(0, -1);
						auto ui = reflector->ui;
						ui->parents.push(reflector->e_tree);
						reflector->add_node(ui->current_root);
						ui->parents.pop();
					}, Capture().set_thiz(c._thiz));
				}, Capture().set_thiz(this));

				ui->e_begin_splitter(SplitterHorizontal);
				ui->next_element_padding = 4.f;
				ui->next_element_frame_thickness = 2.f;
				ui->next_element_frame_color = ui->style(ForegroundColor).c;
				ui->e_begin_scrollbar(ScrollbarVertical, true);
				ui->next_entity = e_tree;
				ui->e_begin_tree(true);
				ui->e_end_tree();
				ui->e_end_scrollbar();
				c_tree = e_tree->get_component(cTree);
				c_tree->data_changed_listeners.add([](Capture& c, uint hash, void*) {
					if (hash == FLAME_CHASH("selected"))
					{
						looper().add_event([](Capture& c) {
							c.thiz<cReflector>()->refresh_detail();
						}, Capture().set_thiz(c._thiz));
					}
					return true;
				}, Capture().set_thiz(this));

				ui->next_element_padding = 4.f;
				ui->next_element_frame_thickness = 2.f;
				ui->next_element_frame_color = ui->style(ForegroundColor).c;
				ui->e_begin_scrollbar(ScrollbarVertical, true);
				e_detail = ui->e_begin_layout(LayoutVertical, 4.f, false, false);
				ui->c_aligner(AlignMinMax, AlignMinMax);
				ui->e_end_layout();
				ui->e_end_scrollbar();
				ui->e_end_splitter();

				ui->parents.push(e_tree);
				add_node(ui->current_root);
				ui->parents.pop();
			}

			void processing_event_dispatcher_event()
			{
				if (s_event_dispatcher->key_states[Key_Esc] == (KeyStateDown | KeyStateJust))
				{
					recording = !recording;
					txt_record->set_text(recording ? L"Recording (Esc To Stop)" : L"Stopped (Esc To Start)");
					txt_record->set_color(recording ? Vec4c(200, 120, 0, 255) : ui->style(TextColorNormal).c);
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
						find_undering(ui->current_root, Vec2f(s_event_dispatcher->mouse_pos));
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
						auto color = ui->style(TextColorNormal).c;
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
						if (e->name.h == FLAME_CHASH("ui_reflector_item_leaf"))
							dp = e->get_component(cDataKeeper);
						else if (e->name.h == FLAME_CHASH("ui_reflector_item_title"))
							dp = e->parent->get_component(cDataKeeper);
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

			void on_event(EntityEvent e, void* t) override
			{
				if (e == EntityEnteredWorld)
				{
					s_event_dispatcher = entity->world->get_system(sEventDispatcher);
					s_event_dispatcher_listener = s_event_dispatcher->after_update_listeners.add([](Capture& c) {
						c.thiz<cReflector>()->processing_event_dispatcher_event();
						return true;
					}, Capture().set_thiz(this));
					s_2d_renderer = entity->world->get_system(s2DRenderer);
					s_2d_renderer_listener = s_2d_renderer->after_update_listeners.add([](Capture& c) {
						c.thiz<cReflector>()->processing_2d_renderer_event();
						return true;
					}, Capture().set_thiz(this));
				}
			}
		};
		auto c_reflector = f_new<cReflector>();
		c_reflector->ui = &ui;
		c_reflector->create(e_window);
		ui.parents.top()->add_component(c_reflector);

		ui.e_size_dragger();

		ui.e_end_window();

		ui.parents.pop();
		return e_window;
	}
}
