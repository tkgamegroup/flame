#pragma once

#include "graphics.h"

namespace flame
{
	namespace graphics
	{
		struct Material
		{
			virtual vec4 get_color() const = 0;
			virtual float get_metallic() const = 0;
			virtual float get_roughness() const = 0;
			virtual float get_alpha_test() const = 0;

			virtual void get_pipeline_file(wchar_t* dst) const = 0;
			inline std::filesystem::path get_pipeline_file() const
			{
				wchar_t buf[260];
				get_pipeline_file(buf);
				return buf;
			}

			virtual const char* get_pipeline_defines() const = 0;

			virtual void get_texture_file(uint idx, wchar_t* dst) const = 0;
			virtual bool get_texture_srgb(uint idx) const = 0;
			virtual SamplerPtr get_texture_sampler(DevicePtr device, uint idx) const = 0;

			FLAME_GRAPHICS_EXPORTS static Material* get(const wchar_t* filename);
		};
	}
}
