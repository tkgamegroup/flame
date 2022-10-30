#pragma once

#include "../system.h"

namespace flame
{
	// Reflect
	struct sAudio : System
	{
		struct Instance
		{
			virtual sAudioPtr operator()() = 0;
		};
		FLAME_UNIVERSE_API static Instance& instance;

		struct Create
		{
			virtual sAudioPtr operator()(WorldPtr) = 0;
		};
		// Reflect static
		FLAME_UNIVERSE_API static Create& create;
	};
}
