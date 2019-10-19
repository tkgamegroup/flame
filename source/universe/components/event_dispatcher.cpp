#include "../entity_private.h"
#include <flame/universe/components/element.h>
#include "event_receiver_private.h"
#include <flame/universe/components/event_dispatcher.h>

namespace flame
{
	struct cEventDispatcherPrivate : cEventDispatcher
	{
		Window* window;
		void* key_listener;
		void* mouse_listener;

		std::vector<Key> keydown_inputs;
		std::vector<Key> keyup_inputs;
		std::vector<wchar_t> char_inputs;
		bool char_input_compelete;

		cEventReceiver* potential_dbclick_er;
		float potential_dbclick_time;

		Vec2i active_pos;

		std::vector<cEventReceiver*> hovers;
		bool meet_last_hovering;

		cEventDispatcherPrivate(Window* window) :
			window(window)
		{
			hovering = nullptr;
			focusing = nullptr;
			drag_overing = nullptr;

			next_focusing = (cEventReceiver*)FLAME_INVALID_POINTER;

			char_input_compelete = true;
			for (auto i = 0; i < FLAME_ARRAYSIZE(key_states); i++)
				key_states[i] = KeyStateUp;

			mouse_pos = Vec2i(0);
			mouse_pos_prev = Vec2i(0);
			mouse_disp = Vec2i(0);
			mouse_scroll = 0;
			for (auto i = 0; i < FLAME_ARRAYSIZE(mouse_buttons); i++)
				mouse_buttons[i] = KeyStateUp;
			potential_dbclick_er = nullptr;
			potential_dbclick_time = 0.f;

			active_pos = Vec2i(0);

			key_listener = window->add_key_listener([](void* c, KeyState action, int value) {
				auto thiz = *(cEventDispatcherPrivate**)c;

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
				
			}, new_mail_p(this));

			mouse_listener = window->add_mouse_listener([](void* c, KeyState action, MouseKey key, const Vec2i& pos) {
				auto thiz = *(cEventDispatcherPrivate * *)c;

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
			}, new_mail_p(this));
		}

		~cEventDispatcherPrivate() 
		{
			window->remove_key_listener(key_listener);
			window->remove_mouse_listener(mouse_listener);
		}

		void search_hovers(EntityPrivate* e)
		{
			for (auto it = e->children.rbegin(); it != e->children.rend(); it++)
			{
				auto c = it->get();
				if (c->global_visible_)
					search_hovers(c);
			}

			auto er = (cEventReceiverPrivate*)e->find_component(cH("EventReceiver"));
			if (!er)
				return;

			er->event_dispatcher = this;
			auto active = focusing && focusing->active;
			if (active && hovering == er)
				meet_last_hovering = true;
			if (!er->element->cliped && er->element->contains(Vec2f(mouse_pos)))
			{
				if (!hovering || (active && !meet_last_hovering))
					hovers.push_back(er);
				if (!er->penetrable)
				{
					if (!hovering)
					{
						er->hovering = true;
						hovering = er;
					}

					if (focusing && focusing->dragging && !drag_overing && er != focusing && !er->acceptable_drops.empty())
					{
						auto h = focusing->drag_hash;
						for (auto _h : er->acceptable_drops)
						{
							if (_h == h)
							{
								drag_overing = er;
								break;
							}
						}
					}
				}
			}
		}

		virtual void update() override
		{
			mouse_disp = mouse_pos - mouse_pos_prev;
			if (potential_dbclick_er)
			{
				potential_dbclick_time += looper().delta_time;
				if (potential_dbclick_time > 0.5f)
				{
					potential_dbclick_er = nullptr;
					potential_dbclick_time = 0.f;
				}
			}

			auto prev_hovering = hovering;
			auto prev_hovering_active = (hovering && hovering->active) ? true : false;
			auto prev_focusing = focusing;
			auto prev_dragging = (!focusing || !focusing->dragging) ? nullptr : focusing;
			auto prev_drag_overing = drag_overing;

			if (next_focusing != FLAME_INVALID_POINTER)
			{
				focusing = next_focusing;
				next_focusing = (cEventReceiver*)FLAME_INVALID_POINTER;
			}

			if (focusing)
			{
				if (!focusing->entity->global_visible_)
				{
					focusing->hovering = false;
					focusing->focusing = false;
					focusing->active = false;
					focusing->dragging = false;
					focusing = nullptr;
				}
				else if (focusing->active && is_mouse_up((KeyState)mouse_buttons[Mouse_Left], Mouse_Left))
				{
					focusing->active = false;
					focusing->dragging = false;
				}
			}

			hovers.clear();
			drag_overing = nullptr;
			if (focusing && focusing->active)
			{
				hovers.push_back(focusing);

				if (focusing->drag_hash != 0 && !focusing->dragging)
				{
					if (mouse_disp != 0 && (abs(mouse_pos.x() - active_pos.x()) > 4.f || abs(mouse_pos.y() - active_pos.y()) > 4.f))
						focusing->dragging = true;
				}
			}
			else
			{
				if (hovering)
				{
					hovering->hovering = false;
					hovering = nullptr;
				}
			}
			meet_last_hovering = false;
			search_hovers((EntityPrivate*)entity);

			if (is_mouse_down((KeyState)mouse_buttons[Mouse_Left], Mouse_Left, true))
			{
				focusing = nullptr;

				if (hovering)
				{
					focusing = hovering;
					hovering->active = true;
					active_pos = mouse_pos;
				}
			}

			if (prev_focusing != focusing)
			{
				if (prev_focusing)
				{
					prev_focusing->focusing = false;
					prev_focusing->active = false;
					((cEventReceiverPrivate*)prev_focusing)->on_focus(Focus_Lost);
				}
				if (focusing)
				{
					focusing->focusing = true;
					((cEventReceiverPrivate*)focusing)->on_focus(Focus_Gain);
				}
			}

			if (focusing)
			{
				for (auto& code : keydown_inputs)
					((cEventReceiverPrivate*)focusing)->on_key(KeyStateDown, code);
				for (auto& code : keyup_inputs)
					((cEventReceiverPrivate*)focusing)->on_key(KeyStateUp, code);
				for (auto& ch : char_inputs)
					((cEventReceiverPrivate*)focusing)->on_key(KeyStateNull, ch);
			}
			for (auto it = hovers.rbegin(); it != hovers.rend(); it++)
			{
				auto er = *it;
				if (mouse_disp != 0)
					((cEventReceiverPrivate*)er)->on_mouse(KeyStateNull, Mouse_Null, mouse_disp);
				if (mouse_scroll != 0)
					((cEventReceiverPrivate*)er)->on_mouse(KeyStateNull, Mouse_Middle, Vec2i(mouse_scroll, 0));
				for (auto i = 0; i < FLAME_ARRAYSIZE(mouse_buttons); i++)
				{
					auto s = mouse_buttons[i];
					if (s & KeyStateJust)
						((cEventReceiverPrivate*)er)->on_mouse((KeyState)s, (MouseKey)i, mouse_pos);
				}
			}
			if (focusing && is_mouse_up((KeyState)mouse_buttons[Mouse_Left], Mouse_Left, true) && focusing->element->contains(Vec2f(mouse_pos)))
			{
				auto disp = mouse_pos - active_pos;
				((cEventReceiverPrivate*)focusing)->on_mouse(KeyState(KeyStateDown | KeyStateUp), Mouse_Null, disp);
				if (potential_dbclick_er == focusing)
				{
					((cEventReceiverPrivate*)focusing)->on_mouse(KeyState(KeyStateDown | KeyStateUp | KeyStateDouble), Mouse_Null, disp);
					potential_dbclick_er = nullptr;
					potential_dbclick_time = 0.f;
				}
				else
					potential_dbclick_er = focusing;
			}

			if (prev_hovering != hovering)
			{
				if (prev_hovering)
				{
					((cEventReceiverPrivate*)prev_hovering)->on_state_changed(prev_hovering->state, EventReceiverNormal);
					prev_hovering->state = EventReceiverNormal;
				}
				if (hovering)
				{
					hovering->state = hovering->active ? EventReceiverActive : EventReceiverHovering;
					((cEventReceiverPrivate*)hovering)->on_state_changed(EventReceiverNormal, hovering->state);
				}
			}
			else if (hovering && hovering->active != prev_hovering_active)
			{
				hovering->state = hovering->active ? EventReceiverActive : EventReceiverHovering;
				((cEventReceiverPrivate*)hovering)->on_state_changed(prev_hovering_active ? EventReceiverActive : EventReceiverHovering, hovering->state);
			}
			if (!prev_dragging && focusing && focusing->dragging)
				((cEventReceiverPrivate*)focusing)->on_drag_and_drop(DragStart, nullptr, mouse_pos);
			else if (prev_dragging && (!focusing || !focusing->dragging))
			{
				if (prev_drag_overing)
					((cEventReceiverPrivate*)prev_drag_overing)->on_drag_and_drop(Dropped, prev_dragging, mouse_pos);
				((cEventReceiverPrivate*)prev_dragging)->on_drag_and_drop(DragEnd, prev_drag_overing, mouse_pos);
			}
			if (drag_overing)
				((cEventReceiverPrivate*)drag_overing)->on_drag_and_drop(DragOvering, focusing, mouse_pos);

			keydown_inputs.clear();
			keyup_inputs.clear();
			char_inputs.clear();
			for (int i = 0; i < FLAME_ARRAYSIZE(key_states); i++)
				key_states[i] &= ~KeyStateJust;

			mouse_pos_prev = mouse_pos;
			mouse_scroll = 0;
			for (auto i = 0; i < FLAME_ARRAYSIZE(mouse_buttons); i++)
				mouse_buttons[i] &= ~KeyStateJust;
		}
	};

	cEventDispatcher* cEventDispatcher::create(Window* window)
	{
		return new cEventDispatcherPrivate(window);
	}
}
