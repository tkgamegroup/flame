#pragma once

#include "../system.h"

namespace flame
{
	/// Reflect
	struct sInput : System
	{
		vec2 offset = vec2(0.f);
		bool mbtn[MouseButton_Count] = {};
		float mbtn_duration[MouseButton_Count] = {};
		vec2 mpos = vec2(0.f);
		vec2 mdisp = vec2(0.f);
		bool kbtn[KeyboardKey_Count] = {};
		float kbtn_duration[KeyboardKey_Count] = {};

		inline bool mpressed(MouseButton btn)
		{
			return mbtn[btn] && mbtn_duration[btn] == 0.f;
		}

		inline bool kpressed(KeyboardKey key)
		{
			return kbtn[key] && kbtn_duration[key] == 0.f;
		}

		struct Instance
		{
			virtual sInputPtr operator()() = 0;
		};
		FLAME_UNIVERSE_API static Instance& instance;

		struct Create
		{
			virtual sInputPtr operator()(WorldPtr) = 0;
		};
		/// Reflect static
		FLAME_UNIVERSE_API static Create& create;
	};
}
