//#include <flame/serialize.h>

#include "../world_private.h"
#include "../components/element_private.h"
#include "../components/event_receiver_private.h"
#include "event_dispatcher_private.h"
//#include <flame/universe/components/element.h>

namespace flame
{
//	sEventDispatcherPrivate::sEventDispatcherPrivate()
//	{
//		next_focusing = (cEventReceiver*)INVALID_POINTER;
//
//		char_input_compelete = true;
//		for (auto i = 0; i < size(key_states); i++)
//			key_states[i] = KeyStateUp;
//
//		mouse_scroll = 0;
//		for (auto i = 0; i < size(mouse_buttons); i++)
//			mouse_buttons[i] = KeyStateUp;
//		dbclick_timer = -1.f;
//	}

	void sEventDispatcherPrivate::on_added()
	{
		window = (Window*)((WorldPrivate*)world)->find_object("Window");
		if (window)
		{
			key_listener = window->add_key_listener([](Capture& c, KeyStateFlags action, int value) {
//				auto thiz = c.thiz<sEventDispatcherPrivate>();
//
//				if (action == KeyStateNull)
//				{
//					if (!thiz->char_input_compelete && !thiz->char_inputs.empty())
//					{
//						std::string ansi;
//						ansi += thiz->char_inputs.back();
//						ansi += value;
//						auto wstr = a2w(ansi);
//						thiz->char_inputs.back() = wstr[0];
//						thiz->char_input_compelete = true;
//					}
//					else
//					{
//						thiz->char_inputs.push_back(value);
//						if (value >= 0x80)
//							thiz->char_input_compelete = false;
//					}
//				}
//				else
//				{
//					thiz->key_states[value] = action | KeyStateJust;
//					if (action == KeyStateDown)
//						thiz->keydown_inputs.push_back((Key)value);
//					else if (action == KeyStateUp)
//						thiz->keyup_inputs.push_back((Key)value);
//				}
//
//				thiz->dirty = true;
			}, Capture().set_thiz(this));

			mouse_listener = window->add_mouse_listener([](Capture& c, KeyStateFlags action, MouseKey key, const Vec2i& pos) {
				auto thiz = c.thiz<sEventDispatcherPrivate>();

				if (action == KeyStateNull)
				{
					if (key == Mouse_Middle)
						;//thiz->mouse_scroll = pos.x();
					else if (key == Mouse_Null)
					{
						thiz->mdisp += pos - thiz->mpos;
						thiz->mpos = pos;
					}
				}
				else
				{
					thiz->mbtns[key] = action | KeyStateJust;
					thiz->mdisp += pos - thiz->mpos;
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
		auto mouse_contained = er->element->contains(mpos);
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
//							focusing_state = FocusingAndActive;
//							active_pos = mouse_pos;
						}
					}
				}
			}
//
//			if (mouse_disp != 0)
//				((cEventReceiverPrivate*)er)->on_mouse(KeyStateNull, Mouse_Null, mouse_disp);
//			if (mouse_scroll != 0)
//				((cEventReceiverPrivate*)er)->on_mouse(KeyStateNull, Mouse_Middle, Vec2i(mouse_scroll, 0));
//			for (auto i = 0; i < size(mouse_buttons); i++)
//			{
//				auto s = mouse_buttons[i];
//				if (s & KeyStateJust)
//					((cEventReceiverPrivate*)er)->on_mouse(s, (MouseKey)i, mouse_pos);
//			}
		}
//
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
//
//		er->frame = get_looper()->frame;
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
		if (!er /*|| er->frame >= (int)get_looper()->frame*/)
			return;

		dispatch_mouse_single(er, false);
	}

	void sEventDispatcherPrivate::update()
	{
//		if (dbclick_timer > 0.f)
//			dbclick_timer -= get_looper()->delta_time;

		if (!dirty)
			return;
		dirty = false;

//		mouse_disp = mouse_pos - mouse_pos_prev;
//
		auto prev_hovering = hovering;
		auto prev_focusing = focusing;
//		auto prev_focusing_state = focusing_state;
//		auto prev_drag_overing = drag_overing;
//		auto prev_dragging = (!focusing || focusing_state != FocusingAndDragging) ? nullptr : focusing;
		hovering = nullptr;
//		drag_overing = nullptr;
//
//		if (next_focusing != INVALID_POINTER)
//		{
//			focusing = next_focusing;
//			next_focusing = (cEventReceiver*)INVALID_POINTER;
//		}
//
//		if (focusing)
//		{
//			if (!focusing->entity->global_visibility)
//				focusing = nullptr;
//			else if (focusing_state != FocusingNormal)
//			{
//				switch (focusing->focus_type)
//				{
//				case FocusByLeftButton:
//					if (mouse_buttons[Mouse_Left] & KeyStateUp)
//						focusing_state = FocusingNormal;
//					break;
//				case FocusByRightButton:
//					if (mouse_buttons[Mouse_Right] & KeyStateUp)
//						focusing_state = FocusingNormal;
//					break;
//				case FocusByLeftOrRightButton:
//					if ((mouse_buttons[Mouse_Left] & KeyStateUp) && (mouse_buttons[Mouse_Right] & KeyStateUp))
//						focusing_state = FocusingNormal;
//					break;
//				}
//			}
//
//		}
//
//		if (focusing && focusing_state == FocusingAndActive)
//		{
//			if (focusing->drag_hash)
//			{
//				if (mouse_disp != 0 && (abs(mouse_pos.x() - active_pos.x()) > 4.f || abs(mouse_pos.y() - active_pos.y()) > 4.f))
//					focusing_state = FocusingAndDragging;
//			}
//		}
//
//		mouse_event_checker = nullptr;
//		if (focusing && focusing_state != FocusingNormal)
//			dispatch_mouse_single((cEventReceiverPrivate*)focusing, true);
		dispatch_mouse_recursively(((WorldPrivate*)world)->root.get());

		//if (focusing && (mbtns[Mouse_Left] == (KeyStateUp | KeyStateJust)) && rect_contains(focusing->element->clipped_rect, Vec2f(mpos)))
		if (focusing && (mbtns[Mouse_Left] == (KeyStateUp | KeyStateJust)) && focusing->element->contains(Vec2f(mpos)))
		{
			//auto disp = mouse_pos - active_pos;
			//auto db = dbclick_timer > 0.f;
			focusing->on_mouse(KeyStateDown | KeyStateUp, Mouse_Null, Vec2i(0));
			//((cEventReceiverPrivate*)focusing)->on_mouse(KeyStateDown | KeyStateUp | (db ? KeyStateDouble : 0), Mouse_Null, disp);
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

//		if (prev_hovering != hovering)
//		{
//			if (prev_hovering)
//				((cEventReceiverPrivate*)prev_hovering)->on_hovering(false);
//			if (hovering)
//				((cEventReceiverPrivate*)hovering)->on_hovering(true);
//		}
//
//		if (prev_focusing != focusing)
//		{
//			if (prev_focusing)
//				((cEventReceiverPrivate*)prev_focusing)->on_focusing(false);
//			if (focusing)
//			{
//				((cEventReceiverPrivate*)focusing)->on_focusing(true);
//
//				key_receiving = nullptr;
//				auto e = focusing->entity;
//				while (e)
//				{
//					auto er = e->get_component(cEventReceiver);
//					if (er && er->key_listeners.impl->count() > 0)
//					{
//						key_receiving = er;
//						break;
//					}
//					e = e->parent;
//				}
//			}
//
//			dbclick_timer = -1.f;
//		}
//
//		if (!prev_dragging && focusing && focusing_state == FocusingAndDragging)
//			((cEventReceiverPrivate*)focusing)->on_drag_and_drop(DragStart, nullptr, mouse_pos);
//		else if (prev_dragging && (!focusing || focusing_state != FocusingAndDragging))
//		{
//			if (prev_drag_overing)
//				((cEventReceiverPrivate*)prev_drag_overing)->on_drag_and_drop(BeenDropped, prev_dragging, mouse_pos);
//			((cEventReceiverPrivate*)prev_dragging)->on_drag_and_drop(DragEnd, prev_drag_overing, mouse_pos);
//		}
//		if (prev_drag_overing != drag_overing)
//		{
//			if (prev_drag_overing)
//				((cEventReceiverPrivate*)prev_drag_overing)->on_drag_and_drop(BeingOverEnd, focusing, mouse_pos);
//			if (drag_overing)
//				((cEventReceiverPrivate*)drag_overing)->on_drag_and_drop(BeingOverStart, focusing, mouse_pos);
//		}
//		if (focusing && focusing_state == FocusingAndDragging)
//		{
//			((cEventReceiverPrivate*)focusing)->on_drag_and_drop(DragOvering, drag_overing, mouse_pos);
//			if (drag_overing)
//				((cEventReceiverPrivate*)drag_overing)->on_drag_and_drop(BeingOvering, focusing, mouse_pos);
//		}
//
//		if (key_receiving)
//		{
//			for (auto& code : keydown_inputs)
//				((cEventReceiverPrivate*)key_receiving)->on_key(KeyStateDown, code);
//			for (auto& code : keyup_inputs)
//				((cEventReceiverPrivate*)key_receiving)->on_key(KeyStateUp, code);
//			for (auto& ch : char_inputs)
//				((cEventReceiverPrivate*)key_receiving)->on_key(KeyStateNull, ch);
//		}

		{ // TODO test
			for (int i = 0; i < size(mbtns); i++)
				mbtns[i] = (KeyStateFlags)((int)mbtns[i] & ~KeyStateJust);
		}
	}
//
//	void sEventDispatcherPrivate::after_update()
//	{
//		keydown_inputs.clear();
//		keyup_inputs.clear();
//		char_inputs.clear();
//		for (int i = 0; i < size(key_states); i++)
//			key_states[i] &= ~KeyStateJust;
//
//		mouse_pos_prev = mouse_pos;
//		mouse_scroll = 0;
//		for (auto i = 0; i < size(mouse_buttons); i++)
//			mouse_buttons[i] &= ~KeyStateJust;
//	}

	sEventDispatcherPrivate* sEventDispatcherPrivate::create()
	{
		auto ret = _allocate(sizeof(sEventDispatcherPrivate));
		new (ret) sEventDispatcherPrivate;
		return (sEventDispatcherPrivate*)ret;
	}

	sEventDispatcher* sEventDispatcher::create() { return sEventDispatcherPrivate::create(); }
}
