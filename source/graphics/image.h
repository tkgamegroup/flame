#pragma once

#include "graphics.h"

namespace flame
{
	namespace graphics
	{
		inline Format get_image_format(uint channel, uint byte_per_channel)
		{
			switch (channel)
			{
			case 0:
				switch (byte_per_channel)
				{
				case 1:
					return Format_R8_UNORM;
				default:
					return Format_Undefined;

				}
				break;
			case 1:
				switch (byte_per_channel)
				{
				case 1:
					return Format_R8_UNORM;
				case 2:
					return Format_R16_UNORM;
				default:
					return Format_Undefined;
				}
				break;
			case 4:
				switch (byte_per_channel)
				{
				case 1:
					return Format_R8G8B8A8_UNORM;
				default:
					return Format_Undefined;
				}
				break;
			default:
				return Format_Undefined;
			}
		}

		inline uint get_pixel_size(Format format)
		{
			switch (format)
			{
			case Format_R8_UNORM:
				return 1;
			case Format_R16_UNORM:
				return 2;
			case Format_R32_SFLOAT:
				return 4;
			case Format_R8G8B8A8_UNORM: case Format_B8G8R8A8_UNORM:
				return 4;
			case Format_R16G16B16A16_UNORM: case Format_R16G16B16A16_SFLOAT:
				return 8;
			case Format_R32G32B32A32_SFLOAT:
				return 16;
			case Format_Depth16:
				return 2;
			}
			return 0;
		}

		struct Image
		{
			virtual void release() = 0;

			virtual Format get_format() const = 0;
			virtual uvec2 get_size(uint lv = 0) const = 0;
			virtual uint get_levels() const = 0;
			virtual uint get_layers() const = 0;
			virtual SampleCount get_sample_count() const = 0;

			virtual const wchar_t* get_filename() const = 0;

			virtual ImageViewPtr get_view(const ImageSub& sub = {}, const ImageSwizzle& swizzle = {}) = 0;
			virtual DescriptorSetPtr get_shader_read_src(uint base_level = 0, uint base_layer = 0, SamplerPtr sp = nullptr) = 0;
			virtual FramebufferPtr get_shader_write_dst(uint base_level = 0, uint base_layer = 0, bool clear = false) = 0;

			virtual void change_layout(ImageLayout src_layout, ImageLayout dst_layout) = 0;
			virtual void clear(ImageLayout src_layout, ImageLayout dst_layout, const cvec4& color) = 0;

			virtual vec4 linear_sample(const vec2& uv, uint level = 0, uint layer = 0) = 0;

			virtual void generate_mipmaps() = 0;

			virtual void save(const wchar_t* filename) = 0;

			FLAME_GRAPHICS_EXPORTS static Image* create(Device* device, Format format, const uvec2& size, uint level, uint layer, SampleCount sample_count, 
				ImageUsageFlags usage, bool is_cube = false);
			FLAME_GRAPHICS_EXPORTS static Image* create(Device* device, Bitmap* bmp);
			FLAME_GRAPHICS_EXPORTS static Image* get(Device* device, const wchar_t* filename, bool srgb);
		};

		struct ImageView
		{
			virtual ImagePtr get_image() const = 0;

			virtual ImageSub get_sub() const = 0;
			virtual ImageSwizzle get_swizzle() const = 0;
		};

		struct Sampler
		{
			virtual void release() = 0;

			FLAME_GRAPHICS_EXPORTS static Sampler* get(Device* device, Filter mag_filter, Filter min_filter, bool linear_mipmap, AddressMode address_mode);
		};

		struct ImageAtlas
		{
			struct TileInfo
			{
				uint id;
				const char* name;
				ivec2 pos;
				ivec2 size;
				vec4 uv;
			};

			virtual void release() = 0; 

			virtual uint get_tiles_count() const = 0;
			virtual void get_tile(uint id, TileInfo* dst) const = 0;
			virtual bool find_tile(const char* name, TileInfo* dst) const = 0;

			virtual ImagePtr get_image() const = 0;

			FLAME_GRAPHICS_EXPORTS static ImageAtlas* get(Device* device, const wchar_t* filename);
		};
	}
}
