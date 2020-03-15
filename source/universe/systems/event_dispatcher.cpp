#include <flame/serialize.h>
#include <flame/universe/world.h>
#include "event_dispatcher_private.h"
#include <flame/universe/components/element.h>

namespace flame
{
	sEventDispatcherPrivate::sEventDispatcherPrivate()
	{
		window = nullptr;
		key_listener = nullptr;
		mouse_listener = nullptr;

		hovering = nullptr;
		focusing = nullptr;
		focusing_state = FocusingNormal;
		key_receiving = nullptr;
		drag_overing = nullptr;

		next_focusing = (cEventReceiver*)INVALID_POINTER;

		char_input_compelete = true;
		for (auto i = 0; i < array_size(key_states); i++)
			key_states[i] = KeyStateUp;

		mouse_pos = Vec2i(0);
		mouse_pos_prev = Vec2i(0);
		mouse_disp = Vec2i(0);
		mouse_scroll = 0;
		for (auto i = 0; i < array_size(mouse_buttons); i++)
			mouse_buttons[i] = KeyStateUp;
		dbclick_timer = -1.f;

		active_pos = Vec2i(0);

		pending_update = false;
	}

	sEventDispatcherPrivate::~sEventDispatcherPrivate()
	{
		if (window)
		{
			window->key_listeners.remove(key_listener);
			window->mouse_listeners.remove(mouse_listener);
		}
	}

	void sEventDispatcherPrivate::on_receiver_removed(cEventReceiver* er)
	{
		if (er == focusing)
			focusing = nullptr;
		if (er == hovering)
			hovering = nullptr;
		if (er == key_receiving)
			key_receiving = nullptr;
		if (er == drag_overing)
			drag_overing = nullptr;
		er->state = EventReceiverNormal;
		er->state_listeners.call(EventReceiverNormal);
	}

	void sEventDispatcherPrivate::on_added()
	{
		window = (SysWindow*)world_->find_object(FLAME_CHASH("SysWindow"), 0);
		if (window)
		{

			key_listener = window->key_listeners.add([](void* c, KeyStateFlags action, int value) {
				auto thiz = *(sEventDispatcherPrivate**)c;

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
					thiz->key_states[value] = action | KeyStateJust;
					if (action == KeyStateDown)
						thiz->keydown_inputs.push_back((Key)value);
					else if (action == KeyStateUp)
						thiz->keyup_inputs.push_back((Key)value);
				}

				thiz->pending_update = true;

				return true;
			}, new_mail_p(this));

			mouse_listener = window->mouse_listeners.add([](void* c, KeyStateFlags action, MouseKey key, const Vec2i& pos) {
				auto thiz = *(sEventDispatcherPrivate**)c;

				if (action == KeyStateNull)
				{
					if (key == Mouse_Middle)
						thiz->mouse_scroll = pos.x();
					else if (key == Mouse_Null)
						thiz->mouse_pos = pos;
				}
				else
				{
					thiz->mouse_buttons[key] = action | KeyStateJust;
					thiz->mouse_pos = pos;
				}

				thiz->pending_update = true;

				return true;
			}, new_mail_p(this));
		}
	}

	void sEventDispatcherPrivate::dispatch_mouse_single(cEventReceiverPrivate* er, bool force)
	{
		auto mouse_contained = !er->element->cliped && rect_contains(er->element->cliped_rect, Vec2f(mouse_pos));

		if ([&]() {
				if (!mouse_event_checker)
					return true;
				if (mouse_event_checker == INVALID_POINTER)
					return false;
				auto pass = false;
				mouse_event_checker->pass_checkers.call(er, &pass);
				return pass;
			}() && (force || mouse_contained))
		{
			mouse_event_checker = er->pass_checkers.impl->count() ? er : (cEventReceiverPrivate*)INVALID_POINTER;

			if (mouse_event_checker == INVALID_POINTER && mouse_contained)
			{
				hovering = er;

				if (mouse_buttons[Mouse_Left] == (KeyStateDown | KeyStateJust))
				{
					focusing = nullptr;

					if (hovering)
					{
						focusing = hovering;
						focusing_state = FocusingAndActive;
						active_pos = mouse_pos;
					}
				}
			}

			if (mouse_disp != 0)
				((cEventReceiverPrivate*)er)->on_mouse(KeyStateNull, Mouse_Null, mouse_disp);
			if (mouse_scroll != 0)
				((cEventReceiverPrivate*)er)->on_mouse(KeyStateNull, Mouse_Middle, Vec2i(mouse_scroll, 0));
			for (auto i = 0; i < array_size(mouse_buttons); i++)
			{
				auto s = mouse_buttons[i];
				if (s & KeyStateJust)
					((cEventReceiverPrivate*)er)->on_mouse(s, (MouseKey)i, mouse_pos);
			}
		}

		if (!drag_overing && mouse_contained && focusing && focusing_state == FocusingAndDragging && er != focusing)
		{
			auto hash = focusing->drag_hash;
			for (auto h : er->acceptable_drops)
			{
				if (h == hash)
				{
					drag_overing = er;
					break;
				}
			}
		}

		er->frame = looper().frame;
	}

	void sEventDispatcherPrivate::dispatch_mouse_recursively(EntityPrivate* e)
	{
		for (auto i = (int)e->children.size() - 1; i >= 0; i--)
		{
			auto c = e->children[i].get();
			if (c->global_visibility_ && e->get_component(cElement))
				dispatch_mouse_recursively(c);
		}

		auto er = (cEventReceiverPrivate*)e->get_component(cEventReceiver);
		if (!er || er->frame >= (int)looper().frame)
			return;

		dispatch_mouse_single(er, false);
	}

	void sEventDispatcherPrivate::update()
	{
		if (!pending_update)
			return;
		pending_update = false;

		mouse_disp = mouse_pos - mouse_pos_prev;

		auto prev_hovering = hovering;
		auto prev_focusing = focusing;
		auto prev_focusing_state = focusing_state;
		auto prev_drag_overing = drag_overing;
		auto prev_dragging = (!focusing || focusing_state != FocusingAndDragging) ? nullptr : focusing;
		hovering = nullptr;
		drag_overing = nullptr;

		if (next_focusing != INVALID_POINTER)
		{
			focusing = next_focusing;
			next_focusing = (cEventReceiver*)INVALID_POINTER;
		}

		if (focusing)
		{
			if (!focusing->entity->global_visibility_)
				focusing = nullptr;
			else if (focusing_state != FocusingNormal && (mouse_buttons[Mouse_Left] & KeyStateUp))
				focusing_state = FocusingNormal;

			if (dbclick_timer > 0.f)
			{
				dbclick_timer -= looper().delta_time;
				if (dbclick_timer <= 0.f)
					dbclick_timer = -1.f;
			}
		}
		else
			dbclick_timer = -1.f;

		if (focusing && focusing_state == FocusingAndActive)
		{
			if (focusing->drag_hash)
			{
				if (mouse_disp != 0 && (abs(mouse_pos.x() - active_pos.x()) > 4.f || abs(mouse_pos.y() - active_pos.y()) > 4.f))
					focusing_state = FocusingAndDragging;
			}
		}

		mouse_event_checker = nullptr;
		if (focusing && focusing_state != FocusingNormal)
			dispatch_mouse_single((cEventReceiverPrivate*)focusing, true);
		dispatch_mouse_recursively((EntityPrivate*)world_->root());

		if (focusing && (mouse_buttons[Mouse_Left] == (KeyStateUp | KeyStateJust)) && rect_contains(focusing->element->cliped_rect, Vec2f(mouse_pos)))
		{
			auto disp = mouse_pos - active_pos;
			auto db = dbclick_timer > 0.f;
			((cEventReceiverPrivate*)focusing)->on_mouse(KeyStateDown | KeyStateUp | (db ? KeyStateDouble : 0), Mouse_Null, disp);
			if (db)
				dbclick_timer = -1.f;
			else
				dbclick_timer = 0.5f;
		}

		if (prev_hovering != hovering)
		{
			if (prev_hovering)
			{
				prev_hovering->state = EventReceiverNormal;
				prev_hovering->state_listeners.call(EventReceiverNormal);
			}
			if (hovering)
			{
				hovering->state = (hovering == focusing && focusing_state != FocusingNormal) ? EventReceiverActive : EventReceiverHovering;
				hovering->state_listeners.call(hovering->state);
			}
		}
		else if (hovering && prev_focusing_state != focusing_state)
		{
			hovering->state = focusing_state != FocusingNormal ? EventReceiverActive : EventReceiverHovering;
			hovering->state_listeners.call(hovering->state);
		}

		if (prev_focusing != focusing)
		{
			if (prev_focusing)
				prev_focusing->focus_listeners.call(false);
			if (focusing)
			{
				focusing->focus_listeners.call(true);

				key_receiving = nullptr;
				auto e = focusing->entity;
				while (e)
				{
					auto er = e->get_component(cEventReceiver);
					if (er && er->key_listeners.impl->count())
					{
						key_receiving = er;
						break;
					}
					e = e->parent();
				}
			}
		}

		if (!prev_dragging && focusing && focusing_state == FocusingAndDragging)
			((cEventReceiverPrivate*)focusing)->on_drag_and_drop(DragStart, nullptr, mouse_pos);
		else if (prev_dragging && (!focusing || focusing_state != FocusingAndDragging))
		{
			if (prev_drag_overing)
				((cEventReceiverPrivate*)prev_drag_overing)->on_drag_and_drop(Dropped, prev_dragging, mouse_pos);
			((cEventReceiverPrivate*)prev_dragging)->on_drag_and_drop(DragEnd, prev_drag_overing, mouse_pos);
		}
		if (drag_overing)
			((cEventReceiverPrivate*)drag_overing)->on_drag_and_drop(DragOvering, focusing, mouse_pos);

		if (key_receiving)
		{
			for (auto& code : keydown_inputs)
				((cEventReceiverPrivate*)focusing)->on_key(KeyStateDown, code);
			for (auto& code : keyup_inputs)
				((cEventReceiverPrivate*)focusing)->on_key(KeyStateUp, code);
			for (auto& ch : char_inputs)
				((cEventReceiverPrivate*)focusing)->on_key(KeyStateNull, ch);
		}

		keydown_inputs.clear();
		keyup_inputs.clear();
		char_inputs.clear();
		for (int i = 0; i < array_size(key_states); i++)
			key_states[i] &= ~KeyStateJust;

		mouse_pos_prev = mouse_pos;
		mouse_scroll = 0;
		for (auto i = 0; i < array_size(mouse_buttons); i++)
			mouse_buttons[i] &= ~KeyStateJust;
	}

	sEventDispatcher* sEventDispatcher::create()
	{
		return new sEventDispatcherPrivate();
	}
}
