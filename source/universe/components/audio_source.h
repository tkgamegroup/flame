#pragma once

#include "../component.h"

namespace flame
{
	// Reflect ctor
	struct cAudioSource : Component
	{
		// Reflect requires
		cNodePtr node = nullptr;

		// Reflect
		std::vector<std::pair<std::filesystem::path, std::string>> buffer_names;
		// Reflect
		virtual void set_buffer_names(const std::vector<std::pair<std::filesystem::path, std::string>>& names) = 0;

		virtual void play(uint name, float volumn = 1.f) = 0;
		virtual void stop(uint name) = 0;
		virtual void pause(uint name) = 0;
		virtual void rewind(uint name) = 0;

		struct Create
		{
			virtual cAudioSourcePtr operator()(EntityPtr e) = 0;
		};
		// Reflect static
		FLAME_UNIVERSE_API static Create& create;
	};
}
