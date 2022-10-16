#pragma once

#include "../system.h"

namespace flame
{
	/// Reflect
	struct sAudio : System
	{
		virtual int get_buffer_res(const std::filesystem::path& filename, int id = -1) = 0;
		virtual void release_buffer_res(uint id) = 0;

		struct Instance
		{
			virtual sAudioPtr operator()() = 0;
		};
		FLAME_UNIVERSE_API static Instance& instance;

		struct Create
		{
			virtual sAudioPtr operator()(WorldPtr) = 0;
		};
		/// Reflect static
		FLAME_UNIVERSE_API static Create& create;
	};
}
