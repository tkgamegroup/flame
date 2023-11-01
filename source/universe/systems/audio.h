#pragma once

#include "../system.h"

namespace flame
{
	// Reflect ctor
	struct sAudio : System
	{
		virtual void play_once(const std::filesystem::path& path) = 0;

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
