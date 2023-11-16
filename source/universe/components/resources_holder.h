#pragma once

#include "../component.h"

namespace flame
{
	// Reflect ctor
	struct cResourcesHolder : Component
	{
		virtual void hold(const std::filesystem::path& path, uint name) = 0;
		virtual graphics::ImagePtr get_graphics_image(uint name) = 0;
		virtual audio::BufferPtr get_audio_buffer(uint name) = 0;

		struct Create
		{
			virtual cResourcesHolderPtr operator()(EntityPtr) = 0;
		};
		// Reflect static
		FLAME_UNIVERSE_API static Create& create;
	};
}
