#include "../../foundation/window.h"
#include "../world_private.h"
#include "../components/element_private.h"
#include "../components/receiver_private.h"
#include "imgui_private.h"
#include "dispatcher_private.h"

namespace flame
{
	sDispatcherPrivate::sDispatcherPrivate()
	{
		for (auto i = 0; i < _countof(kbtns_temp); i++)
			kbtns_temp[i].second = false;
		for (auto i = 0; i < _countof(mbtns_temp); i++)
			mbtns_temp[i].second = false;
	}

	void sDispatcherPrivate::on_added()
	{
		imgui = world->get_system_t<sImguiPrivate>();
	}

	void sDispatcherPrivate::setup(NativeWindow* _window)
	{
		assert(!window);
		window = _window;

		window->add_key_down_listener([this](KeyboardKey key) {
			if (kbtns_temp[key].first)
				return;
			kbtns_temp[key] = std::make_pair(true, true);
			key_down_inputs.push_back(key);

			dirty = true;
		});

		window->add_key_up_listener([this](KeyboardKey key) {
			kbtns_temp[key] = std::make_pair(false, true);
			key_up_inputs.push_back(key);

			dirty = true;
		});

		window->add_char_listener([this](wchar_t ch) {
			char_inputs.push_back(ch);

			dirty = true;
		});

		window->add_mouse_left_down_listener([this](const ivec2& pos) {
			mbtns_temp[Mouse_Left] = std::make_pair(true, true);
			mdisp_temp += pos - mpos;
			mpos = pos;

			dirty = true;
		});

		window->add_mouse_left_up_listener([this](const ivec2& pos) {
			mbtns_temp[Mouse_Left] = std::make_pair(false, true);
			mdisp_temp += pos - mpos;
			mpos = pos;

			dirty = true;
		});

		window->add_mouse_right_down_listener([this](const ivec2& pos) {
			mbtns_temp[Mouse_Right] = std::make_pair(true, true);
			mdisp_temp += pos - mpos;
			mpos = pos;

			dirty = true;
		});

		window->add_mouse_right_up_listener([this](const ivec2& pos) {
			mbtns_temp[Mouse_Right] = std::make_pair(false, true);
			mdisp_temp += pos - mpos;
			mpos = pos;

			dirty = true;
		});

		window->add_mouse_middle_down_listener([this](const ivec2& pos) {
			mbtns_temp[Mouse_Middle] = std::make_pair(true, true);
			mdisp_temp += pos - mpos;
			mpos = pos;

			dirty = true;
		});

		window->add_mouse_middle_up_listener([this](const ivec2& pos) {
			mbtns_temp[Mouse_Middle] = std::make_pair(false, true);
			mdisp_temp += pos - mpos;
			mpos = pos;

			dirty = true;
		});

		window->add_mouse_move_listener([this](const ivec2& pos) {
			mdisp_temp += pos - mpos;
			mpos = pos;

			dirty = true;
		});

		window->add_mouse_scroll_listener([this](int scroll) {
			mscrl_temp = scroll;

			dirty = true;
		});

		auto root = world->root.get();
		cReceiverPrivate* cer;
		cer = root->get_component_t<cReceiverPrivate>();
		if (!cer)
		{
			root->add_component(new cElementPrivate);

			cer = new cReceiverPrivate;
			cer->set_floating(true);
			root->add_component(cer);
		}
		set_next_focusing(cer);
	}

	void sDispatcherPrivate::dispatch_mouse_single(cReceiverPrivate* er, bool force)
	{
		auto frame = get_frames();
		if (er->mute || er->frame >= frame)
			return;

		auto mouse_contained = er->element->contains((vec2)mpos) && er->element->parent_scissor.contains((vec2)mpos);

		if (!hovering && mouse_contained)
		{
			hovering = er;

			if (mbtns[Mouse_Left] == std::make_pair(true, true))
			{
				focusing = nullptr;
				if (hovering)
				{
					focusing = hovering;
					active = focusing;
					active_pos = mpos;
				}
			}
		}
		if (hovering == er || force || (er->floating && mouse_contained))
			mouse_targets.push_back(er);

//		if (!drag_overing && mouse_contained && focusing && focusing_state == FocusingAndDragging && er != focusing)
//		{
//			auto hash = focusing->drag_hash;
//			for (auto h : er->acceptable_drops)
//			{
//				if (h == hash)
//				{
//					drag_overing = er;
//					break;
//				}
//			}
//		}

		er->frame = frame;
	}

	void sDispatcherPrivate::dispatch_mouse_recursively(EntityPrivate* e)
	{
		for (auto it = e->children.rbegin(); it != e->children.rend(); it++)
		{
			auto c = it->get();
			if (c->global_visibility && c->get_component_t<cElement>())
				dispatch_mouse_recursively(c);
		}

		auto er = (cReceiverPrivate*)e->get_component_t<cReceiver>();
		if (er)
			dispatch_mouse_single(er, false);
	}

	void sDispatcherPrivate::update()
	{
		if (dbclick_timer > 0.f)
			dbclick_timer -= get_delta_time();

		if (!dirty)
			return;

		if (!imgui || !imgui->mouse_consumed)
		{
			for (int i = 0; i < _countof(mbtns); i++)
			{
				if (mbtns_temp[i].second)
				{
					mbtns[i] = std::make_pair(mbtns_temp[i].first, true);
					mbtns_temp[i].second = false;
				}
				else
					mbtns[i].second = false;
			}

			mdisp = mdisp_temp;
			mdisp_temp = ivec2(0.f);
			mscrl = mscrl_temp;
			mscrl_temp = 0;

			auto prev_hovering = hovering;
			auto prev_focusing = focusing;
			auto prev_active = active;
			//		auto prev_drag_overing = drag_overing;
			//		auto prev_dragging = (!focusing || focusing_state != FocusingAndDragging) ? nullptr : focusing;
			hovering = nullptr;
			//		drag_overing = nullptr;

			if (next_focusing != INVALID_POINTER)
			{
				focusing = next_focusing;
				next_focusing = (cReceiverPrivate*)INVALID_POINTER;
			}

			if (focusing)
			{
				if (!focusing->entity->global_visibility)
					focusing = nullptr;
				else if (active || dragging)
				{
					if (!mbtns[Mouse_Left].first)
						active = dragging = nullptr;
				}

			}

			//		if (focusing && focusing_state == FocusingAndActive)
			//		{
			//			if (focusing->drag_hash)
			//			{
			//				if (mouse_disp != 0 && (abs(mouse_pos.x - active_pos.x) > 4.f || abs(mouse_pos.y - active_pos.y) > 4.f))
			//					focusing_state = FocusingAndDragging;
			//			}
			//		}

			mouse_targets.clear();
			if (active)
				dispatch_mouse_single(active, true);
			dispatch_mouse_recursively(world->root.get());

			auto set_state = [&](cReceiverPrivate* er) {
				auto e = er->entity;
				auto s = (e->state & (~StateHovering) & (~StateFocusing) & (~StateActive));
				if (er == hovering)
					s |= StateHovering;
				if (er == focusing)
					s |= StateFocusing;
				if (er == active)
					s |= StateActive;
				e->set_state((StateFlags)s);
			};
			if (prev_hovering)
				set_state(prev_hovering);
			if (hovering)
				set_state(hovering);
			if (prev_focusing)
				set_state(prev_focusing);
			if (focusing)
				set_state(focusing);

			if (prev_focusing != focusing)
			{
				if (focusing)
				{
					keyboard_target = nullptr;
					auto e = focusing->entity;
					while (e)
					{
						auto er = e->get_component_t<cReceiverPrivate>();
						if (er && !(er->key_down_listeners.list.empty() && er->key_up_listeners.list.empty() && er->char_listeners.list.empty()))
						{
							keyboard_target = er;
							break;
						}
						e = e->parent;
					}
				}

				dbclick_timer = -1.f;
			}

			for (auto er : mouse_targets)
			{
				er->mouse_left_down_listeners.begin_staging();
				er->mouse_left_up_listeners.begin_staging();
				er->mouse_right_down_listeners.begin_staging();
				er->mouse_right_up_listeners.begin_staging();
				er->mouse_middle_down_listeners.begin_staging();
				er->mouse_middle_up_listeners.begin_staging();
				er->mouse_move_listeners.begin_staging();
				er->mouse_scroll_listeners.begin_staging();
				er->mouse_click_listeners.begin_staging();
				er->mouse_dbclick_listeners.begin_staging();
			}
			for (auto er : mouse_targets)
			{
				if (mdisp.x != 0 || mdisp.y != 0)
				{
					for (auto& l : er->mouse_move_listeners.list)
					{
						l->c._current = er;
						l->call(mdisp, mpos);
					}
				}
				if (mscrl != 0)
				{
					for (auto& l : er->mouse_scroll_listeners.list)
					{
						l->c._current = er;
						l->call(mscrl);
					}
				}
				if (mbtns[Mouse_Left].second)
				{
					if (mbtns[Mouse_Left].first)
					{
						for (auto& l : er->mouse_left_down_listeners.list)
						{
							l->c._current = er;
							l->call(mpos);
						}
					}
					else
					{
						for (auto& l : er->mouse_left_up_listeners.list)
						{
							l->c._current = er;
							l->call(mpos);
						}
					}
				}
				if (mbtns[Mouse_Right].second)
				{
					if (mbtns[Mouse_Right].first)
					{
						for (auto& l : er->mouse_right_down_listeners.list)
						{
							l->c._current = er;
							l->call(mpos);
						}
					}
					else
					{
						for (auto& l : er->mouse_right_up_listeners.list)
						{
							l->c._current = er;
							l->call(mpos);
						}
					}
				}
				if (mbtns[Mouse_Middle].second)
				{
					if (mbtns[Mouse_Middle].first)
					{
						for (auto& l : er->mouse_middle_down_listeners.list)
						{
							l->c._current = er;
							l->call(mpos);
						}
					}
					else
					{
						for (auto& l : er->mouse_middle_up_listeners.list)
						{
							l->c._current = er;
							l->call(mpos);
						}
					}
				}
			}
			for (auto er : mouse_targets)
			{
				er->mouse_left_down_listeners.end_staging();
				er->mouse_left_up_listeners.end_staging();
				er->mouse_right_down_listeners.end_staging();
				er->mouse_right_up_listeners.end_staging();
				er->mouse_middle_down_listeners.end_staging();
				er->mouse_middle_up_listeners.end_staging();
				er->mouse_move_listeners.end_staging();
				er->mouse_scroll_listeners.end_staging();
				er->mouse_click_listeners.end_staging();
				er->mouse_dbclick_listeners.end_staging();
			}

			//if (focusing && (mbtns[Mouse_Left] == (KeyStateUp | KeyStateJust)) && rect_contains(focusing->element->clipped_rect, vec2(mpos)))
			if (focusing && mbtns[Mouse_Left] == std::make_pair(false, true) && focusing->element->contains(vec2(mpos)))
			{
				auto disp = mpos - active_pos;
				auto db = dbclick_timer > 0.f;
				for (auto& l : focusing->mouse_click_listeners.list)
				{
					l->c._current = focusing;
					l->call();
				}
				if (db)
				{
					for (auto& l : focusing->mouse_dbclick_listeners.list)
					{
						l->c._current = focusing;
						l->call();
					}
					dbclick_timer = -1.f;
				}
				else if (disp == ivec2(0))
					dbclick_timer = 0.5f;
			}

			//		if (!prev_dragging && focusing && focusing_state == FocusingAndDragging)
			//			((cReceiverPrivate*)focusing)->send_drag_and_drop_event(DragStart, nullptr, mouse_pos);
			//		else if (prev_dragging && (!focusing || focusing_state != FocusingAndDragging))
			//		{
			//			if (prev_drag_overing)
			//				((cReceiverPrivate*)prev_drag_overing)->send_drag_and_drop_event(BeenDropped, prev_dragging, mouse_pos);
			//			((cReceiverPrivate*)prev_dragging)->send_drag_and_drop_event(DragEnd, prev_drag_overing, mouse_pos);
			//		}
			//		if (prev_drag_overing != drag_overing)
			//		{
			//			if (prev_drag_overing)
			//				((cReceiverPrivate*)prev_drag_overing)->send_drag_and_drop_event(BeingOverEnd, focusing, mouse_pos);
			//			if (drag_overing)
			//				((cReceiverPrivate*)drag_overing)->send_drag_and_drop_event(BeingOverStart, focusing, mouse_pos);
			//		}
			//		if (focusing && focusing_state == FocusingAndDragging)
			//		{
			//			((cReceiverPrivate*)focusing)->send_drag_and_drop_event(DragOvering, drag_overing, mouse_pos);
			//			if (drag_overing)
			//				((cReceiverPrivate*)drag_overing)->send_drag_and_drop_event(BeingOvering, focusing, mouse_pos);
			//		}
		}

		if (!imgui || !imgui->keyboard_consumed)
		{
			for (int i = 0; i < _countof(kbtns); i++)
			{
				if (kbtns_temp[i].second)
				{
					kbtns[i] = std::make_pair(kbtns_temp[i].first, true);
					kbtns_temp[i].second = false;
				}
				else
					kbtns[i].second = false;
			}

			if (keyboard_target)
			{
				for (auto& key : key_down_inputs)
					keyboard_target->on_key_event(key, true);
				for (auto& key : key_up_inputs)
					keyboard_target->on_key_event(key, false);
				for (auto& ch : char_inputs)
				{
					for (auto& l : keyboard_target->char_listeners.list)
					{
						l->c._current = keyboard_target;
						l->call(ch);
					}
				}
			}
		}

		key_down_inputs.clear();
		key_up_inputs.clear();
		char_inputs.clear();

		dirty = false;
	}

	sDispatcher* sDispatcher::create(void* parms)
	{
		return new sDispatcherPrivate();
	}
}
