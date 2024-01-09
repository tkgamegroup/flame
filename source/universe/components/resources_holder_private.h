#pragma once

#include "resources_holder.h"

namespace flame
{
	struct cResourcesHolderPrivate : cResourcesHolder
	{
		~cResourcesHolderPrivate();

		std::unordered_map<uint, graphics::ImagePtr> graphics_images;
		std::unordered_map<uint, graphics::ImageAtlasPtr> graphics_image_atlases;
		std::unordered_map<uint, std::pair<graphics::MaterialPtr, int>> graphics_materials;
#if USE_AUDIO_MODULE
		std::unordered_map<uint, audio::BufferPtr> audio_buffers;
#endif

		void hold(const std::filesystem::path& path, uint name) override;
		graphics::ImagePtr get_graphics_image(uint name) override;
		graphics::ImageAtlasPtr get_graphics_image_atlas(uint name) override;
		graphics::MaterialPtr get_material(uint name) override;
		int get_material_res_id(uint name) override;
#if USE_AUDIO_MODULE
		audio::BufferPtr get_audio_buffer(uint name) override;
#endif
	};
}
