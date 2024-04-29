#pragma once

#include "../system.h"

namespace flame
{
	// Reflect ctor
	struct sInput : System
	{
		vec2 offset = vec2(0.f);
		bool mbtn[MouseButton_Count] = {};
		float mbtn_duration[MouseButton_Count] = {};
		vec2 mpos = vec2(0.f);
		vec2 mdisp = vec2(0.f);
		int mscroll = 0;
		bool kbtn[KeyboardKey_Count] = {};
		float kbtn_duration[KeyboardKey_Count] = {};

		bool mouse_used = false;
		bool key_used = false;

		bool transfer_events = true;
		cReceiverPtr hovering_receiver = nullptr;
		cReceiverPtr active_receiver = nullptr;

		inline bool mpressed(MouseButton btn)
		{
			return mbtn[btn] && mbtn_duration[btn] == 0.f;
		}

		inline bool mreleased(MouseButton btn)
		{
			return !mbtn[btn] && mbtn_duration[btn] == 0.f;
		}

		inline bool kpressed(KeyboardKey key)
		{
			return kbtn[key] && kbtn_duration[key] == 0.f;
		}

		inline bool kreleased(KeyboardKey key)
		{
			return !kbtn[key] && kbtn_duration[key] == 0.f;
		}

		virtual void bind_window(NativeWindowPtr w) = 0;

		struct Instance
		{
			virtual sInputPtr operator()() = 0;
		};
		FLAME_UNIVERSE_API static Instance& instance;

		struct Create
		{
			virtual sInputPtr operator()(WorldPtr) = 0;
		};
		// Reflect static
		FLAME_UNIVERSE_API static Create& create;
	};
}
