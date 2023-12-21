#pragma once

#include "resources_holder.h"

namespace flame
{
	struct cResourcesHolderPrivate : cResourcesHolder
	{
		~cResourcesHolderPrivate();

		std::unordered_map<uint, graphics::ImagePtr> graphics_images;
		std::unordered_map<uint, graphics::ImageAtlasPtr> graphics_image_atlases;
#if USE_AUDIO_MODULE
		std::unordered_map<uint, audio::BufferPtr> audio_buffers;
#endif

		void hold(const std::filesystem::path& path, uint name) override;
		graphics::ImagePtr get_graphics_image(uint name) override;
		graphics::ImageAtlasPtr get_graphics_image_atlas(uint name) override;
#if USE_AUDIO_MODULE
		audio::BufferPtr get_audio_buffer(uint name) override;
#endif
	};
}
