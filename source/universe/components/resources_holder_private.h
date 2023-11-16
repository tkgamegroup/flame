#pragma once

#include "resources_holder.h"

namespace flame
{
	struct cResourcesHolderPrivate : cResourcesHolder
	{
		~cResourcesHolderPrivate();

		std::unordered_map<uint, graphics::ImagePtr> graphics_images;
		std::unordered_map<uint, graphics::ImageAtlasPtr> graphics_image_atlases;
		std::unordered_map<uint, audio::BufferPtr> audio_buffers;

		void hold(const std::filesystem::path& path, uint name) override;
		graphics::ImagePtr get_graphics_image(uint name) override;
		graphics::ImageAtlasPtr get_graphics_image_atlas(uint name) override;
		audio::BufferPtr get_audio_buffer(uint name) override;
	};
}
