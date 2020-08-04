#include <flame/serialize.h>
#include "../world_private.h"
#include "../components/element_private.h"
#include "../components/event_receiver_private.h"
#include "event_dispatcher_private.h"

namespace flame
{
	sEventDispatcherPrivate::sEventDispatcherPrivate()
	{
		for (auto i = 0; i < size(kbtns_temp); i++)
			kbtns_temp[i].second = false;
		for (auto i = 0; i < size(mbtns_temp); i++)
			mbtns_temp[i].second = false;
	}

	void sEventDispatcherPrivate::on_added()
	{
		window = (Window*)((WorldPrivate*)world)->find_object("flame::Window");
		if (window)
		{
			key_down_listener = window->add_key_down_listener([](Capture& c, KeyboardKey key) {
				auto thiz = c.thiz<sEventDispatcherPrivate>();

				thiz->kbtns_temp[key] = std::make_pair(true, true);
				thiz->key_down_inputs.push_back(key);

				thiz->dirty = true;
			}, Capture().set_thiz(this));
			key_up_listener = window->add_key_up_listener([](Capture& c, KeyboardKey key) {
				auto thiz = c.thiz<sEventDispatcherPrivate>();

				thiz->kbtns_temp[key] = std::make_pair(false, true);
				thiz->key_up_inputs.push_back(key);

				thiz->dirty = true;
			}, Capture().set_thiz(this));
			char_listener = window->add_char_listener([](Capture& c, char ch) {
				auto thiz = c.thiz<sEventDispatcherPrivate>();

				if (!thiz->char_input_compelete && !thiz->char_inputs.empty())
				{
					std::string ansi;
					ansi += thiz->char_inputs.back();
					ansi += ch;
					auto wstr = a2w(ansi);
					thiz->char_inputs.back() = wstr[0];
					thiz->char_input_compelete = true;
				}
				else
				{
					thiz->char_inputs.push_back(ch);
					if (ch >= 0x80)
						thiz->char_input_compelete = false;
				}

				thiz->dirty = true;
			}, Capture().set_thiz(this));

			mouse_left_down_listener = window->add_mouse_left_down_listener([](Capture& c, const Vec2i& pos) {
				auto thiz = c.thiz<sEventDispatcherPrivate>();

				thiz->mbtns_temp[Mouse_Left] = std::make_pair(true, true);
				thiz->mdisp_temp += pos - thiz->mpos;
				thiz->mpos = pos;

				thiz->dirty = true;
			}, Capture().set_thiz(this));
			mouse_left_up_listener = window->add_mouse_left_up_listener([](Capture& c, const Vec2i& pos) {
				auto thiz = c.thiz<sEventDispatcherPrivate>();

				thiz->mbtns_temp[Mouse_Left] = std::make_pair(false, true);
				thiz->mdisp_temp += pos - thiz->mpos;
				thiz->mpos = pos;

				thiz->dirty = true;
			}, Capture().set_thiz(this));
			mouse_right_down_listener = window->add_mouse_right_down_listener([](Capture& c, const Vec2i& pos) {
				auto thiz = c.thiz<sEventDispatcherPrivate>();

				thiz->mbtns_temp[Mouse_Right] = std::make_pair(true, true);
				thiz->mdisp_temp += pos - thiz->mpos;
				thiz->mpos = pos;

				thiz->dirty = true;
			}, Capture().set_thiz(this));
			mouse_right_up_listener = window->add_mouse_right_up_listener([](Capture& c, const Vec2i& pos) {
				auto thiz = c.thiz<sEventDispatcherPrivate>();

				thiz->mbtns_temp[Mouse_Right] = std::make_pair(false, true);
				thiz->mdisp_temp += pos - thiz->mpos;
				thiz->mpos = pos;

				thiz->dirty = true;
			}, Capture().set_thiz(this));
			mouse_middle_down_listener = window->add_mouse_middle_down_listener([](Capture& c, const Vec2i& pos) {
				auto thiz = c.thiz<sEventDispatcherPrivate>();

				thiz->mbtns_temp[Mouse_Middle] = std::make_pair(true, true);
				thiz->mdisp_temp += pos - thiz->mpos;
				thiz->mpos = pos;

				thiz->dirty = true;
			}, Capture().set_thiz(this));
			mouse_middle_up_listener = window->add_mouse_middle_up_listener([](Capture& c, const Vec2i& pos) {
				auto thiz = c.thiz<sEventDispatcherPrivate>();

				thiz->mbtns_temp[Mouse_Middle] = std::make_pair(false, true);
				thiz->mdisp_temp += pos - thiz->mpos;
				thiz->mpos = pos;

				thiz->dirty = true;
			}, Capture().set_thiz(this));
			mouse_move_listener = window->add_mouse_move_listener([](Capture& c, const Vec2i& pos) {
				auto thiz = c.thiz<sEventDispatcherPrivate>();

				thiz->mdisp_temp += pos - thiz->mpos;
				thiz->mpos = pos;

				thiz->dirty = true;
			}, Capture().set_thiz(this));
			mouse_scroll_listener = window->add_mouse_scroll_listener([](Capture& c, int scroll) {
				auto thiz = c.thiz<sEventDispatcherPrivate>();

				thiz->mscrl_temp = scroll;

				thiz->dirty = true;
			}, Capture().set_thiz(this));

			destroy_listener = window->add_destroy_listener([](Capture& c) {
				c.thiz<sEventDispatcherPrivate>()->window = nullptr;
			}, Capture().set_thiz(this));
		}
	}

	void sEventDispatcherPrivate::on_removed()
	{
		if (window)
		{
			window->remove_key_down_listener(key_down_listener);
			window->remove_key_up_listener(key_up_listener);
			window->remove_char_listener(char_listener);
			window->remove_mouse_left_down_listener(mouse_left_down_listener);
			window->remove_mouse_left_up_listener(mouse_left_up_listener);
			window->remove_mouse_right_down_listener(mouse_right_down_listener);
			window->remove_mouse_right_up_listener(mouse_right_up_listener);
			window->remove_mouse_middle_down_listener(mouse_middle_down_listener);
			window->remove_mouse_middle_up_listener(mouse_middle_up_listener);
			window->remove_mouse_move_listener(mouse_move_listener);
			window->remove_mouse_scroll_listener(mouse_scroll_listener);
			window->remove_destroy_listener(destroy_listener);
		}
	}

	void sEventDispatcherPrivate::dispatch_mouse_single(cEventReceiverPrivate* er, bool force)
	{
		auto mouse_contained = er->element->contains((Vec2f)mpos);
		//auto mouse_contained = !er->element->clipped && rect_contains(er->element->clipped_rect, Vec2f(mouse_pos));

		if ([&]() {
				//if (!mouse_event_checker)
				//	return true;
				//if (mouse_event_checker == INVALID_POINTER)
				//	return false;
				//auto pass = false;
				//mouse_event_checker->pass_checkers.call(er, &pass);
				//return pass;
				return true;
			}() && (force || mouse_contained))
		{
//			mouse_event_checker = er->pass_checkers.impl->count() ? er : (cEventReceiverPrivate*)INVALID_POINTER;
//
			//if (mouse_event_checker == INVALID_POINTER && mouse_contained)
			if (mouse_contained)
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

			if (mdisp != 0)
			{
				for (auto& l : er->mouse_move_listeners)
					l->call(mdisp, mpos);
			}
			if (mscrl != 0)
			{
				for (auto& l : er->mouse_scroll_listeners)
					l->call(mscrl);
			}
			if (mbtns[Mouse_Left].second)
			{
				if (mbtns[Mouse_Left].first)
				{
					for (auto& l : er->mouse_left_down_listeners)
						l->call(mpos);
				}
				else
				{
					for (auto& l : er->mouse_left_up_listeners)
						l->call(mpos);
				}
			}
			if (mbtns[Mouse_Right].second)
			{
				if (mbtns[Mouse_Right].first)
				{
					for (auto& l : er->mouse_right_down_listeners)
						l->call(mpos);
				}
				else
				{
					for (auto& l : er->mouse_right_up_listeners)
						l->call(mpos);
				}
			}
			if (mbtns[Mouse_Middle].second)
			{
				if (mbtns[Mouse_Middle].first)
				{
					for (auto& l : er->mouse_middle_down_listeners)
						l->call(mpos);
				}
				else
				{
					for (auto& l : er->mouse_middle_up_listeners)
						l->call(mpos);
				}
			}
		}

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

		er->frame = looper().get_frame();
	}

	void sEventDispatcherPrivate::dispatch_mouse_recursively(EntityPrivate* e)
	{
		for (auto it = e->children.rbegin(); it != e->children.rend(); it++)
		{
			auto c = it->get();
			if (c->global_visibility && e->get_component(cElement::type_hash))
				dispatch_mouse_recursively(c);
		}

		auto er = (cEventReceiverPrivate*)e->get_component(cEventReceiver::type_hash);
		if (!er || er->frame >= (int)looper().get_frame())
			return;

		dispatch_mouse_single(er, false);
	}

	void sEventDispatcherPrivate::update()
	{
		if (dbclick_timer > 0.f)
			dbclick_timer -= looper().get_delta_time();

		if (!dirty)
			return;
		dirty = false;

		for (int i = 0; i < size(kbtns); i++)
		{
			if (kbtns_temp[i].second)
			{
				kbtns[i] = std::make_pair(kbtns_temp[i].first, true);
				kbtns_temp[i].second = false;
			}
			else
				kbtns[i].second = false;
		}
		for (int i = 0; i < size(mbtns); i++)
		{
			if (mbtns_temp[i].second)
			{
				mbtns[i] = std::make_pair(mbtns_temp[i].first, true);
				mbtns_temp[i].second = false;
			}
			else
				mbtns[i].second;
		}
		mdisp = mdisp_temp;
		mdisp_temp = 0.f;
		mscrl = mscrl_temp;
		mscrl_temp = 0;

		auto prev_hovering = hovering;
		auto prev_focusing = focusing;
		auto prev_active = active;
//		auto prev_drag_overing = drag_overing;
//		auto prev_dragging = (!focusing || focusing_state != FocusingAndDragging) ? nullptr : focusing;
		hovering = nullptr;
//		drag_overing = nullptr;

//		if (next_focusing != INVALID_POINTER)
//		{
//			focusing = next_focusing;
//			next_focusing = (cEventReceiver*)INVALID_POINTER;
//		}

		if (focusing)
		{
			if (!((EntityPrivate*)focusing->entity)->global_visibility)
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
//				if (mouse_disp != 0 && (abs(mouse_pos.x() - active_pos.x()) > 4.f || abs(mouse_pos.y() - active_pos.y()) > 4.f))
//					focusing_state = FocusingAndDragging;
//			}
//		}

//		mouse_event_checker = nullptr;
		if (active)
			dispatch_mouse_single(active, true);
		dispatch_mouse_recursively(((WorldPrivate*)world)->root.get());

		//if (focusing && (mbtns[Mouse_Left] == (KeyStateUp | KeyStateJust)) && rect_contains(focusing->element->clipped_rect, Vec2f(mpos)))
		if (focusing && mbtns[Mouse_Left] == std::make_pair(false, true) && focusing->element->contains(Vec2f(mpos)))
		{
			//auto disp = mouse_pos - active_pos;
			//auto db = dbclick_timer > 0.f;
			for (auto& l : focusing->mouse_click_listeners)
				l->call();
			//((cEventReceiverPrivate*)focusing)->send_mouse_event(KeyStateDown | KeyStateUp | (db ? KeyStateDouble : 0), Mouse_Null, disp);
			//if (db)
			//	dbclick_timer = -1.f;
			//else if (disp == 0)
			//	dbclick_timer = 0.5f;
		}

		auto set_state = [&](cEventReceiver* er) {
			auto e = (EntityPrivate*)er->entity;
			auto s = (e->state & (~StateHovering) & (~StateActive));
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
				auto e = (EntityPrivate*)focusing->entity;
				while (e)
				{
					auto er = (cEventReceiverPrivate*)e->get_component(cEventReceiver::type_hash);
					if (er && !(er->key_down_listeners.empty() && er->key_up_listeners.empty() && er->char_listeners.empty()))
					{
						keyboard_target = er;
						break;
					}
					e = e->parent;
				}
			}

			dbclick_timer = -1.f;
		}

//		if (!prev_dragging && focusing && focusing_state == FocusingAndDragging)
//			((cEventReceiverPrivate*)focusing)->send_drag_and_drop_event(DragStart, nullptr, mouse_pos);
//		else if (prev_dragging && (!focusing || focusing_state != FocusingAndDragging))
//		{
//			if (prev_drag_overing)
//				((cEventReceiverPrivate*)prev_drag_overing)->send_drag_and_drop_event(BeenDropped, prev_dragging, mouse_pos);
//			((cEventReceiverPrivate*)prev_dragging)->send_drag_and_drop_event(DragEnd, prev_drag_overing, mouse_pos);
//		}
//		if (prev_drag_overing != drag_overing)
//		{
//			if (prev_drag_overing)
//				((cEventReceiverPrivate*)prev_drag_overing)->send_drag_and_drop_event(BeingOverEnd, focusing, mouse_pos);
//			if (drag_overing)
//				((cEventReceiverPrivate*)drag_overing)->send_drag_and_drop_event(BeingOverStart, focusing, mouse_pos);
//		}
//		if (focusing && focusing_state == FocusingAndDragging)
//		{
//			((cEventReceiverPrivate*)focusing)->send_drag_and_drop_event(DragOvering, drag_overing, mouse_pos);
//			if (drag_overing)
//				((cEventReceiverPrivate*)drag_overing)->send_drag_and_drop_event(BeingOvering, focusing, mouse_pos);
//		}

		if (keyboard_target)
		{
			for (auto& code : key_down_inputs)
			{
				for (auto& l : keyboard_target->key_down_listeners)
					l->call(code);
			}
			for (auto& code : key_up_inputs)
			{
				for (auto& l : keyboard_target->key_up_listeners)
					l->call(code);
			}
			for (auto& ch : char_inputs)
			{
				for (auto& l : keyboard_target->char_listeners)
					l->call(ch);
			}
		}
		key_down_inputs.clear();
		key_up_inputs.clear();
		char_inputs.clear();
	}

	sEventDispatcher* sEventDispatcher::create()
	{
		return f_new<sEventDispatcherPrivate>();
	}
}
