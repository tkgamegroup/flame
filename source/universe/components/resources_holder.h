#pragma once

#include "../component.h"

namespace flame
{
	// Reflect ctor
	struct cResourcesHolder : Component
	{
		virtual void hold(const std::filesystem::path& path, uint name) = 0;
		virtual graphics::ImagePtr get_graphics_image(uint name) = 0;
		virtual graphics::ImageAtlasPtr get_graphics_image_atlas(uint name) = 0;
		virtual graphics::MaterialPtr get_material(uint name) = 0;
		virtual int get_material_res_id(uint name) = 0;
#if USE_AUDIO_MODULE
		virtual audio::BufferPtr get_audio_buffer(uint name) = 0;
#endif

		struct Create
		{
			virtual cResourcesHolderPtr operator()(EntityPtr) = 0;
		};
		// Reflect static
		FLAME_UNIVERSE_API static Create& create;
	};
}
