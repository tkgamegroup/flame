#include "../../foundation/window.h"
#include "../../graphics/window.h"
#include "../components/element_private.h"
#include "../components/receiver_private.h"
#include "input_private.h"
#include "scene_private.h"

namespace flame
{
	sInputPrivate::sInputPrivate()
	{
		auto main_window = graphics::Window::get_list().front();
		auto native_widnow = main_window->native;

		native_widnow->mouse_listeners.add([this](MouseButton btn, bool down) {
			mbtn_temp[btn] = down;
		});
		native_widnow->mousemove_listeners.add([this](const ivec2& pos) {
			mpos_temp = (vec2)pos - offset;
		});
		native_widnow->key_listeners.add([this](KeyboardKey key, bool down) {
			kbtn_temp[key] = down;
		});
	}

	sInputPrivate::~sInputPrivate()
	{
	}

	void sInputPrivate::update()
	{
		for (auto i = 0; i < MouseButton_Count; i++)
		{
			if (!mbtn[i] && mbtn_temp[i])
				mbtn_duration[i] = 0.f;
			else
				mbtn_duration[i] += delta_time;
			mbtn[i] = mbtn_temp[i];
		}
		mdisp = mpos_temp - mpos;
		mpos = mpos_temp;
		for (auto i = 0; i < KeyboardKey_Count; i++)
		{
			if (!kbtn[i] && kbtn_temp[i])
				kbtn_duration[i] = 0.f;
			else
				kbtn_duration[i] += delta_time;
			kbtn[i] = kbtn_temp[i];
		}

		mouse_used = false;
		key_used = false;
		if (auto first_element = sScene::instance()->first_element; first_element)
		{
			if (mpressed(Mouse_Left))
			{
				cReceiverPtr target = nullptr;

				first_element->traversal_bfs([&](EntityPtr e, int depth) {
					if (!e->global_enable)
						return false;

					if (auto receiver = e->get_component_t<cReceiverT>(); receiver)
					{
						if (receiver->element->contains(mpos))
							target = receiver;
					}

					return true;
				});

				if (target && target->entity != first_element)
				{
					target->click_listeners.call();
					if (target->click_action.type)
						target->click_action.value().exec();

					mouse_used = true;
				}
			}
		}
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
