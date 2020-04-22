#pragma once

#include <flame/universe/utils/ui.h>

namespace flame
{
	namespace utils
	{
		inline Entity* e_reflector_window(sEventDispatcher* event_dispatcher)
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

			struct Capture
			{
				sEventDispatcher* event_dispatcher;
				cText* txt_mouse;
				bool recording;
				Entity* e_window;
				Entity* e_tree;
				cText* txt_record;
				cText* txt_hovering;
				cText* txt_focusing;
				cText* txt_drag_overing;

				void add_node(Entity* src)
				{
					if (src == e_window)
						return;
					auto cs = src->child_count();
					auto e = src->get_component(cElement);
					auto str = wfmt(L"%I64X (%.2f, %.2f)-(%.2f, %.2f)", (ulonglong)src,
						e->global_pos.x(), e->global_pos.y(),
						e->global_size.x(), e->global_size.y());
					if (cs == 0)
						e_tree_leaf(str.c_str());
					else
					{
						e_begin_tree_node(str.c_str(), true);
						for (auto i = 0; i < cs; i++)
							add_node(src->child(i));
						e_end_tree_node();
					}
				}
			}capture;
			capture.event_dispatcher = event_dispatcher;
			capture.txt_mouse = e_text(nullptr)->get_component(cText);
			capture.recording = true;
			capture.e_window = e_window;
			capture.e_tree = Entity::create();
			capture.txt_record = e_text(L"Recording (Esc)")->get_component(cText);
			capture.txt_hovering = e_text(nullptr)->get_component(cText);
			capture.txt_focusing = e_text(nullptr)->get_component(cText);
			capture.txt_drag_overing = e_text(nullptr)->get_component(cText);

			e_button(Icon_REFRESH, [](void* c) {
				looper().add_event([](void* c, bool*) {
					auto& capture = **(Capture**)c;
					capture.e_tree->get_component(cTree)->set_selected(nullptr);
					capture.e_tree->remove_children(0, -1);
					push_parent(capture.e_tree);
					capture.add_node(current_root());
					pop_parent();
				}, Mail::from_p(c));
			}, Mail::from_t(&capture));

			next_entity = capture.e_tree;
			next_element_padding = 4.f;
			e_begin_tree(true);
			{
				auto ce = capture.e_tree->get_component(cElement);
				ce->frame_thickness = 2.f;
				ce->frame_color = style_4c(ForegroundColor);
			}
			e_end_tree();

			push_parent(capture.e_tree);
			capture.add_node(current_root());
			pop_parent();

			e_size_dragger();

			e_end_window();

			e_window->on_destroyed_listeners.add([](void* c) {
				looper().remove_event(*(void**)c);
				return true;
			}, Mail::from_p(looper().add_event([](void* c, bool* go_on) {
				auto& capture = *(Capture*)c;

				if (capture.event_dispatcher->key_states[Key_Esc] == (KeyStateDown | KeyStateJust))
				{
					capture.recording = !capture.recording;
					capture.txt_record->set_text(capture.recording ? L"Recording (Esc)" : L"Start Record (Esc)");
				}

				{
					std::wstring str = L"Mouse: ";
					str += to_wstring(capture.event_dispatcher->mouse_pos);
					capture.txt_mouse->set_text(str.c_str());
				}

				if (capture.recording)
				{
					{
						std::wstring str = L"Hovering: ";
						auto hovering = capture.event_dispatcher->hovering;
						auto e = hovering ? hovering->entity : nullptr;
						if (e->is_child_of(capture.e_window))
							e = nullptr;
						str += wfmt(L"0x%016I64X", (ulonglong)e);
						capture.txt_hovering->set_text(str.c_str());
					}
					{
						auto color = style_4c(TextColorNormal);
						std::wstring str = L"Focusing: ";
						auto focusing = capture.event_dispatcher->focusing;
						auto e = focusing ? focusing->entity : nullptr;
						if (e->is_child_of(capture.e_window))
							e = nullptr;
						str += wfmt(L"0x%016I64X", (ulonglong)e);
						if (e)
						{
							if (focusing == capture.event_dispatcher->hovering)
								color = Vec4c(0, 255, 0, 255);
							switch (capture.event_dispatcher->focusing_state)
							{
							case FocusingAndActive:
								str += L" Active";
								break;
							case FocusingAndDragging:
								str += L" Dragging";
								break;
							}
						}
						capture.txt_focusing->set_color(color);
						capture.txt_focusing->set_text(str.c_str());
					}
					{
						std::wstring str = L"Drag Overing: ";
						auto drag_overing = capture.event_dispatcher->drag_overing;
						auto e = drag_overing ? drag_overing->entity : nullptr;
						if (e->is_child_of(capture.e_window))
							e = nullptr;
						str += wfmt(L"0x%016I64X", (ulonglong)e);
						capture.txt_drag_overing->set_text(str.c_str());
					}
				}
				*go_on = true;
			}, Mail::from_t(&capture))));
			pop_parent();
			return e_window;
		}
	}
}
