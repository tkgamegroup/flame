#include "../../foundation/window.h"
#include "../../graphics/window.h"
#include "../world_private.h"
#include "../components/element_private.h"
#include "../components/receiver_private.h"
#include "../components/camera_private.h"
#include "input_private.h"
#include "scene_private.h"
#include "renderer_private.h"

namespace flame
{
	struct sInputEventDispatcher : System
	{
		sInputPrivate* input;

		sInputEventDispatcher() { type_hash = "flame::sInputEventDispatcher"_h; }

		void update() override
		{
			input->dispatcher_events();
		}
	};

	sInputPrivate::sInputPrivate()
	{
		auto main_window = NativeWindow::list().front();
		bind_window(main_window);
	}

	sInputPrivate::~sInputPrivate()
	{
		// will crash..
		// bind_window(nullptr);
	}

	void sInputPrivate::dispatcher_events()
	{
		if (!mouse_used)
		{
			if (auto first_element = sScene::instance()->first_element; first_element)
			{
				auto last_hovering = hovering_receiver;
				auto last_hovering_valid = false;
				auto last_active = active_receiver;
				auto last_active_valid = false;
				hovering_receiver = nullptr;

				auto translate = vec2(0.f);
				auto scaling = vec2(1.f);
				if (camera)
				{
					translate = camera->translate_2d;
					scaling = camera->scaling_2d;
				}
				first_element->traversal_bfs([&, translate, scaling](EntityPtr e, int depth) {
					if (!e->global_enable)
						return false;

					if (auto receiver = e->get_component<cReceiverT>(); receiver)
					{
						if (receiver == last_hovering)
							last_hovering_valid = true;
						if (receiver == last_active)
							last_active_valid = true;
						if (receiver->element->contains(mpos, translate, scaling))
							hovering_receiver = receiver;
					}

					return true;
				});

				if (!last_hovering_valid)
					last_hovering = nullptr;
				if (!last_active_valid)
					last_active = nullptr;

				if (last_hovering != hovering_receiver)
				{
					if (last_hovering)
					{
						if (transfer_events)
						{
							last_hovering->event_listeners.call("mouse_leave"_h, vec2(0.f));
							if (last_hovering->mouse_leave_action.type)
								last_hovering->mouse_leave_action.value().exec();
						}
					}
					if (hovering_receiver)
					{
						if (transfer_events)
						{
							hovering_receiver->event_listeners.call("mouse_enter"_h, vec2(0.f));
							if (hovering_receiver->mouse_enter_action.type)
								hovering_receiver->mouse_enter_action.value().exec();
						}
					}
				}
				if (hovering_receiver)
				{
					if (transfer_events)
						hovering_receiver->event_listeners.call("mouse_move"_h, mdisp);
				}
				if (active_receiver)
				{
					if (mdisp.x != 0.f || mdisp.y != 0.f)
					{
						if (transfer_events)
							active_receiver->event_listeners.call("drag"_h, mdisp);
					}
				}

				if (mpressed(Mouse_Left))
				{
					if (hovering_receiver && hovering_receiver->entity != first_element)
					{
						if (transfer_events)
						{
							hovering_receiver->event_listeners.call("mouse_down"_h, vec2(0.f));

							active_receiver = hovering_receiver;
						}

						mouse_used = true;
					}
				}

				if (mpressed(Mouse_Right))
				{
					if (hovering_receiver && hovering_receiver->entity != first_element)
					{
						if (transfer_events)
							hovering_receiver->event_listeners.call("mouse_down"_h, vec2(1.f));

						mouse_used = true;
					}
				}

				if (last_active != active_receiver)
				{
					if (last_active)
					{
						if (transfer_events)
							last_active->event_listeners.call("lost_focus"_h, vec2(0.f));
					}
					if (active_receiver)
					{
						if (transfer_events)
							active_receiver->event_listeners.call("gain_focus"_h, vec2(0.f));
					}
				}

				if (mreleased(Mouse_Left))
				{
					if (hovering_receiver)
					{
						if (transfer_events)
							hovering_receiver->event_listeners.call("mouse_up"_h, vec2(0.f));
					}
					if (active_receiver && hovering_receiver == active_receiver)
					{
						if (transfer_events)
						{
							active_receiver->event_listeners.call("click"_h, vec2(0.f));
							if (active_receiver->click_action.type)
								active_receiver->click_action.value().exec();
						}
					}

					if (active_receiver)
					{
						auto target = active_receiver;
						active_receiver = nullptr;
						if (transfer_events)
							target->event_listeners.call("lost_focus"_h, vec2(0.f));
					}
				}

				if (mreleased(Mouse_Right))
				{
					if (hovering_receiver)
					{
						if (transfer_events)
							hovering_receiver->event_listeners.call("mouse_up"_h, vec2(1.f));
					}
				}
			}
		}

		mouse_used = false;
		key_used = false;
	}

	void sInputPrivate::bind_window(NativeWindowPtr w)
	{
		if (bound_window == w)
			return;

		if (bound_window)
		{
			bound_window->mouse_listeners.remove("input_system"_h);
			bound_window->mouse_move_listeners.remove("input_system"_h);
			bound_window->mouse_scroll_listeners.remove("input_system"_h);
			bound_window->key_listeners.remove("input_system"_h);
		}

		if (w)
		{
			w->mouse_listeners.add([this](MouseButton btn, bool down) {
				mbtn_temp[btn] = down;
			}, "input_system"_h);
			w->mouse_move_listeners.add([this](const ivec2& pos) {
				mpos_temp = (vec2)pos - offset;
			}, "input_system"_h);
			w->mouse_scroll_listeners.add([this](int scroll) {
				mscr_temp = scroll;
			}, "input_system"_h);
			w->key_listeners.add([this](KeyboardKey key, bool down) {
				kbtn_temp[key] = down;
			}, "input_system"_h);
		}

		bound_window = w;
	}

	void sInputPrivate::start()
	{
		auto renderer = sRenderer::instance();
		if (!renderer->render_tasks.empty())
			camera = renderer->render_tasks.front()->camera;

		{
			auto i_pos = -1;
			for (auto i = 0; i < world->systems.size(); i++)
			{
				if (world->systems[i]->type_hash == "flame::sHud"_h)
				{
					i_pos = i;
					break;
				}
			}
			if (i_pos == -1)
			{
				for (auto i = 0; i < world->systems.size(); i++)
				{
					if (world->systems[i]->type_hash == "flame::sInput"_h)
					{
						i_pos = i;
						break;
					}
				}
			}
			add_event([this, i_pos]() {
				auto s = new sInputEventDispatcher;
				s->input = this;
				world->add_system_p(s, i_pos);
				return false;
			});
		}
	}

	void sInputPrivate::update()
	{
		for (auto i = 0; i < MouseButton_Count; i++)
		{
			if (mbtn[i] != mbtn_temp[i])
				mbtn_duration[i] = 0.f;
			else
				mbtn_duration[i] += delta_time;
			mbtn[i] = mbtn_temp[i];
		}
		mdisp = mpos_temp - mpos;
		mpos = mpos_temp;
		for (auto i = 0; i < KeyboardKey_Count; i++)
		{
			if (kbtn[i] != kbtn_temp[i])
				kbtn_duration[i] = 0.f;
			else
				kbtn_duration[i] += delta_time;
			kbtn[i] = kbtn_temp[i];
		}

		mscroll = mscr_temp;
		mscr_temp = 0;
	}

	static sInputPtr _instance = nullptr;

	struct sInputInstance : sInput::Instance
	{
		sInputPtr operator()() override
		{
			return _instance;
		}
	}sInput_instance;
	sInput::Instance& sInput::instance = sInput_instance;

	struct sInputCreate : sInput::Create
	{
		sInputPtr operator()(WorldPtr w) override
		{
			if (!w)
				return nullptr;

			assert(!_instance);
			_instance = new sInputPrivate();
			return _instance;
		}
	}sInput_create;
	sInput::Create& sInput::create = sInput_create;
}
