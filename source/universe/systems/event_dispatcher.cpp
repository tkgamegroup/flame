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
			kbtns_temp[i] = KeyStateNull;
		for (auto i = 0; i < size(mbtns_temp); i++)
			mbtns_temp[i] = KeyStateNull;
	}

	void sEventDispatcherPrivate::on_added()
	{
		window = (Window*)((WorldPrivate*)world)->find_object("flame::Window");
		if (window)
		{
			key_listener = window->add_key_listener([](Capture& c, KeyStateFlags action, int value) {
				auto thiz = c.thiz<sEventDispatcherPrivate>();

				if (action == KeyStateNull)
				{
					if (!thiz->char_input_compelete && !thiz->char_inputs.empty())
					{
						std::string ansi;
						ansi += thiz->char_inputs.back();
						ansi += value;
						auto wstr = a2w(ansi);
						thiz->char_inputs.back() = wstr[0];
						thiz->char_input_compelete = true;
					}
					else
					{
						thiz->char_inputs.push_back(value);
						if (value >= 0x80)
							thiz->char_input_compelete = false;
					}
				}
				else
				{
					thiz->kbtns_temp[value] = action;
					if (action == KeyStateDown)
						thiz->keydown_inputs.push_back((Key)value);
					else if (action == KeyStateUp)
						thiz->keyup_inputs.push_back((Key)value);
				}

				thiz->dirty = true;
			}, Capture().set_thiz(this));

			mouse_listener = window->add_mouse_listener([](Capture& c, KeyStateFlags action, MouseKey key, const Vec2i& pos) {
				auto thiz = c.thiz<sEventDispatcherPrivate>();

				if (action == KeyStateNull)
				{
					if (key == Mouse_Middle)
						thiz->mscrl_temp = pos.x();
					else if (key == Mouse_Null)
					{
						thiz->mdisp_temp += pos - thiz->mpos;
						thiz->mpos = pos;
					}
				}
				else
				{
					thiz->mbtns_temp[key] = action;
					thiz->mdisp_temp += pos - thiz->mpos;
					thiz->mpos = pos;
				}

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
			window->remove_key_listener(key_listener);
			window->remove_mouse_listener(mouse_listener);
			window->remove_mouse_listener(destroy_listener);
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

				auto left_just_down = mbtns[Mouse_Left] == (KeyStateDown | KeyStateJust);
				auto right_just_down = mbtns[Mouse_Right] == (KeyStateDown | KeyStateJust);
				if (left_just_down || right_just_down)
				{
					if (left_just_down)
						focusing = nullptr;
					if (hovering)
					{
						auto do_focus = false;
//						switch (hovering->focus_type)
//						{
//						case FocusByLeftButton:
//							if (left_just_down)
//								do_focus = true;
//							break;
//						case FocusByRightButton:
//							if (right_just_down)
//								do_focus = true;
//							break;
//						case FocusByLeftOrRightButton:
//							if (left_just_down || right_just_down)
//								do_focus = true;
//							break;
//						}
						{ // TODO test
							if (left_just_down)
								do_focus = true;
						}
						if (do_focus)
						{
							focusing = hovering;
							active = focusing;
							active_pos = mpos;
						}
					}
				}
			}

			if (mdisp != 0)
				er->send_mouse_event(KeyStateNull, Mouse_Null, mdisp);
			if (mscrl != 0)
				er->send_mouse_event(KeyStateNull, Mouse_Middle, Vec2i(mscrl, 0));
			for (auto i = 0; i < size(mbtns); i++)
			{
				auto s = mbtns[i];
				if (s & KeyStateJust)
					er->send_mouse_event(s, (MouseKey)i, mpos);
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

		er->frame = get_looper()->get_frame();
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
		if (!er || er->frame >= (int)get_looper()->get_frame())
			return;

		dispatch_mouse_single(er, false);
	}

	void sEventDispatcherPrivate::update()
	{
		if (dbclick_timer > 0.f)
			dbclick_timer -= get_looper()->get_delta_time();

		if (!dirty)
			return;
		dirty = false;

		for (int i = 0; i < size(kbtns); i++)
		{
			if (kbtns_temp[i] != KeyStateNull)
				kbtns[i] = (KeyStateFlags)((int)kbtns_temp[i] | KeyStateJust);
			else
				kbtns[i] = (KeyStateFlags)((int)kbtns[i] & ~KeyStateJust);
			kbtns_temp[i] = KeyStateNull;
		}
		for (int i = 0; i < size(mbtns); i++)
		{
			if (mbtns_temp[i] != KeyStateNull)
				mbtns[i] = (KeyStateFlags)((int)mbtns_temp[i] | KeyStateJust);
			else
				mbtns[i] = (KeyStateFlags)((int)mbtns[i] & ~KeyStateJust);
			mbtns_temp[i] = KeyStateNull;
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
				if (mbtns[Mouse_Left] & KeyStateUp)
					active = dragging = nullptr;
				//switch (focusing->focus_type)
				//{
				//case FocusByLeftButton:
				//	if (mouse_buttons[Mouse_Left] & KeyStateUp)
				//		focusing_state = FocusingNormal;
				//	break;
				//case FocusByRightButton:
				//	if (mouse_buttons[Mouse_Right] & KeyStateUp)
				//		focusing_state = FocusingNormal;
				//	break;
				//case FocusByLeftOrRightButton:
				//	if ((mouse_buttons[Mouse_Left] & KeyStateUp) && (mouse_buttons[Mouse_Right] & KeyStateUp))
				//		focusing_state = FocusingNormal;
				//	break;
				//}
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
		if (focusing && (mbtns[Mouse_Left] == (KeyStateUp | KeyStateJust)) && focusing->element->contains(Vec2f(mpos)))
		{
			//auto disp = mouse_pos - active_pos;
			//auto db = dbclick_timer > 0.f;
			focusing->send_mouse_event(KeyStateDown | KeyStateUp, Mouse_Null, Vec2i(0));
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
				key_target = nullptr;
				auto e = (EntityPrivate*)focusing->entity;
				while (e)
				{
					auto er = (cEventReceiverPrivate*)e->get_component(cEventReceiver::type_hash);
					if (er && !er->key_listeners.empty())
					{
						key_target = er;
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

		if (key_target)
		{
			for (auto& code : keydown_inputs)
				key_target->send_key_event(KeyStateDown, code);
			for (auto& code : keyup_inputs)
				key_target->send_key_event(KeyStateUp, code);
			for (auto& ch : char_inputs)
				key_target->send_key_event(KeyStateNull, ch);
		}
		keydown_inputs.clear();
		keyup_inputs.clear();
		char_inputs.clear();
	}

	sEventDispatcher* sEventDispatcher::create()
	{
		return f_new<sEventDispatcherPrivate>();
	}
}
