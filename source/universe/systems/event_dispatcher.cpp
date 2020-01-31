#include <flame/serialize.h>
#include "../entity_private.h"
#include <flame/universe/world.h>
#include <flame/universe/systems/event_dispatcher.h>
#include <flame/universe/components/element.h>
#include "../components/event_receiver_private.h"

namespace flame
{
	struct sEventDispatcherPrivate;

	struct HoversSearcher
	{
		sEventDispatcherPrivate* thiz;
		Vec2f pos;
		bool active;
		EntityPrivate* pass;
		std::vector<cEventReceiver*> mouse_dispatch_list;

		void search(sEventDispatcherPrivate* thiz, const Vec2i& pos, EntityPrivate* root);
		void search_r(EntityPrivate* e);
	};
	static HoversSearcher hovers_searcher;

	struct sEventDispatcherPrivate : sEventDispatcher
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

		std::vector<cEventReceiver*> key_dispatch_list;

		sEventDispatcherPrivate()
		{
			window = nullptr;
			key_listener = nullptr;
			mouse_listener = nullptr;

			hovering = nullptr;
			focusing = nullptr;
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
			potential_dbclick_er = nullptr;
			potential_dbclick_time = 0.f;

			active_pos = Vec2i(0);
		}

		~sEventDispatcherPrivate()
		{
			if (window)
			{
				window->key_listeners.remove(key_listener);
				window->mouse_listeners.remove(mouse_listener);
			}
		}

		void clear_focus()
		{
			focusing->hovering = false;
			focusing->focusing = false;
			focusing->active = false;
			focusing->dragging = false;
			focusing = nullptr;
		}

		void receiver_leave_world(cEventReceiver* er)
		{
			if (er == focusing)
				clear_focus();
			if (er == hovering)
			{
				er->hovering = false;
				hovering = false;
			}
			if (er == drag_overing)
				drag_overing = nullptr;
			er->state = EventReceiverNormal;
		}

		void on_added() override
		{
			window = (Window*)world_->find_object(FLAME_CHASH("Window"), 0);
			if (window)
			{

				key_listener = window->key_listeners.add([](void* c, KeyState action, int value) {
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
				}, new_mail_p(this));

				mouse_listener = window->mouse_listeners.add([](void* c, KeyState action, MouseKey key, const Vec2i& pos) {
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
				}, new_mail_p(this));
			}
		}

		void update(Entity* root) override
		{
			if (!pending_update)
				return;
			pending_update = false;

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

			if (next_focusing != INVALID_POINTER)
			{
				focusing = next_focusing;
				next_focusing = (cEventReceiver*)INVALID_POINTER;
			}

			if (focusing)
			{
				if (!focusing->entity->global_visibility_)
					clear_focus();
				else if (focusing->active && ((KeyState)mouse_buttons[Mouse_Left] & KeyStateUp))
				{
					focusing->active = false;
					focusing->dragging = false;
				}
			}

			drag_overing = nullptr;
			if (focusing && focusing->active)
			{
				if (focusing->drag_hash != 0 && !focusing->dragging)
				{
					if (mouse_disp != 0 && (abs(mouse_pos.x() - active_pos.x()) > 4.f || abs(mouse_pos.y() - active_pos.y()) > 4.f))
						focusing->dragging = true;
				}
			}
			else if (hovering)
			{
				hovering->hovering = false;
				hovering = nullptr;
			}
			hovers_searcher.search(this, mouse_pos, (EntityPrivate*)root);

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
				key_dispatch_list.clear();

				if (prev_focusing)
				{
					prev_focusing->focusing = false;
					prev_focusing->active = false;
					prev_focusing->data_changed(FLAME_CHASH("focusing"), nullptr);
				}
				if (focusing)
				{
					auto e = focusing->entity;
					while (e)
					{
						auto er = e->get_component(cEventReceiver);
						if (er && er->accept_key)
							key_dispatch_list.push_back(er);
						e = e->parent();
					}

					focusing->focusing = true;
					focusing->data_changed(FLAME_CHASH("focusing"), nullptr);
				}
			}

			for (auto er : key_dispatch_list)
			{
				for (auto& code : keydown_inputs)
					((cEventReceiverPrivate*)er)->on_key(KeyStateDown, code);
				for (auto& code : keyup_inputs)
					((cEventReceiverPrivate*)er)->on_key(KeyStateUp, code);
				for (auto& ch : char_inputs)
					((cEventReceiverPrivate*)er)->on_key(KeyStateNull, ch);
			}
			for (auto er : hovers_searcher.mouse_dispatch_list)
			{
				if (mouse_disp != 0)
					((cEventReceiverPrivate*)er)->on_mouse(KeyStateNull, Mouse_Null, mouse_disp);
				if (mouse_scroll != 0)
					((cEventReceiverPrivate*)er)->on_mouse(KeyStateNull, Mouse_Middle, Vec2i(mouse_scroll, 0));
				for (auto i = 0; i < array_size(mouse_buttons); i++)
				{
					auto s = mouse_buttons[i];
					if (s & KeyStateJust)
						((cEventReceiverPrivate*)er)->on_mouse((KeyState)s, (MouseKey)i, mouse_pos);
				}
			}
			if (focusing && is_mouse_up((KeyState)mouse_buttons[Mouse_Left], Mouse_Left, true) && rect_contains(focusing->element->cliped_rect, Vec2f(mouse_pos)))
			{
				auto disp = mouse_pos - active_pos;
				((cEventReceiverPrivate*)focusing)->on_mouse(KeyState(KeyStateDown | KeyStateUp), Mouse_Null, disp);
				if (focusing) // since focusing may change to null after on_mouse
				{
					if (potential_dbclick_er == focusing)
					{
						((cEventReceiverPrivate*)focusing)->on_mouse(KeyState(KeyStateDown | KeyStateUp | KeyStateDouble), Mouse_Null, disp);
						potential_dbclick_er = nullptr;
						potential_dbclick_time = 0.f;
					}
					else
						potential_dbclick_er = focusing;
				}
			}

			if (prev_hovering != hovering)
			{
				if (prev_hovering)
				{
					prev_hovering->state = EventReceiverNormal;
					prev_hovering->data_changed(FLAME_CHASH("state"), nullptr);
				}
				if (hovering)
				{
					hovering->state = hovering->active ? EventReceiverActive : EventReceiverHovering;
					hovering->data_changed(FLAME_CHASH("state"), nullptr);
				}
			}
			else if (hovering && hovering->active != prev_hovering_active)
			{
				hovering->state = hovering->active ? EventReceiverActive : EventReceiverHovering;
				hovering->data_changed(FLAME_CHASH("state"), nullptr);
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
			for (int i = 0; i < array_size(key_states); i++)
				key_states[i] &= ~KeyStateJust;

			mouse_pos_prev = mouse_pos;
			mouse_scroll = 0;
			for (auto i = 0; i < array_size(mouse_buttons); i++)
				mouse_buttons[i] &= ~KeyStateJust;
		}
	};

	void HoversSearcher::search(sEventDispatcherPrivate* _thiz, const Vec2i& _pos, EntityPrivate* root)
	{
		thiz = _thiz;
		pos = Vec2f(_pos);
		active = thiz->focusing && thiz->focusing->active;
		pass = (EntityPrivate*)INVALID_POINTER;
		mouse_dispatch_list.clear();
		if (active)
			mouse_dispatch_list.push_back(thiz->focusing);
		search_r(root);
		std::reverse(mouse_dispatch_list.begin(), mouse_dispatch_list.end());
	}

	void HoversSearcher::search_r(EntityPrivate* e)
	{
		for (auto it = e->children.rbegin(); it != e->children.rend(); it++)
		{
			auto c = it->get();
			if (c->global_visibility_)
				search_r(c);
		}

		auto er = (cEventReceiverPrivate*)e->get_component(cEventReceiver);
		if (!er)
			return;
		auto ban = !pass;
		if (!ban && pass != INVALID_POINTER)
		{
			auto p = e;
			while (p)
			{
				if (p == pass)
					break;
				p = p->parent;
			}
			ban = !p;
		}

		if (!er->element->cliped && rect_contains(er->element->cliped_rect, pos))
		{
			if (!ban && (!thiz->hovering || !active || er != thiz->focusing))
				mouse_dispatch_list.push_back(er);
			if (!er->pass)
			{
				if (!ban && !thiz->hovering)
				{
					er->hovering = true;
					thiz->hovering = er;
				}

				if (thiz->focusing && thiz->focusing->dragging && !thiz->drag_overing && er != thiz->focusing)
				{
					auto hash = thiz->focusing->drag_hash;
					for (auto h : er->acceptable_drops)
					{
						if (h == hash)
						{
							thiz->drag_overing = er;
							break;
						}
					}
				}
			}
			pass = (EntityPrivate*)er->pass;
		}
	}

	void sEventDispatcher::receiver_leave_world(cEventReceiver* er)
	{
		((sEventDispatcherPrivate*)this)->receiver_leave_world(er);
	}

	sEventDispatcher* sEventDispatcher::create()
	{
		return new sEventDispatcherPrivate();
	}
}
