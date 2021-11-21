#pragma once

#include "graphics.h"

namespace flame
{
	namespace graphics
	{
		inline Format get_image_format(uint channel, uint bpp)
		{
			if (channel == 0)
				return Format_R8_UNORM;
			switch (channel)
			{
			case 1:
				switch (bpp)
				{
				case 8:
					return Format_R8_UNORM;
				case 16:
					return Format_R16_UNORM;
				default:
					return Format_Undefined;
				}
				break;
			case 4:
				switch (bpp)
				{
				case 32:
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
			Format format = Format_R8G8B8A8_UNORM;
			std::vector<uvec2> sizes;
			uint levels = 1;
			uint layers = 1;
			SampleCount sample_count = SampleCount_1;

			std::filesystem::path filename;

			virtual ~Image() {}

			virtual ImageViewPtr get_view(const ImageSub& sub = {}, const ImageSwizzle& swizzle = {}) = 0;
			virtual DescriptorSetPtr get_shader_read_src(uint base_level = 0, uint base_layer = 0, SamplerPtr sp = nullptr) = 0;
			virtual FramebufferPtr get_shader_write_dst(uint base_level = 0, uint base_layer = 0, AttachmentLoadOp load_op = AttachmentLoadDontCare) = 0;

			virtual void change_layout(ImageLayout src_layout, ImageLayout dst_layout) = 0;
			virtual void clear(ImageLayout src_layout, ImageLayout dst_layout, const cvec4& color) = 0;

			virtual vec4 linear_sample(const vec2& uv, uint level = 0, uint layer = 0) = 0;

			virtual void generate_mipmaps() = 0;

			virtual void save(const std::filesystem::path& filename) = 0;

			struct Create
			{
				virtual ImagePtr operator()(DevicePtr device, Format format, const uvec2& size, uint levels, uint layers, SampleCount sample_count, ImageUsageFlags usage, bool is_cube = false) = 0;
				virtual ImagePtr operator()(DevicePtr device, Bitmap* bmp) = 0;
				virtual ImagePtr operator()(DevicePtr device, Format format, const uvec2& size, void* data) = 0;
			};
			FLAME_GRAPHICS_EXPORTS static Create& create;

			struct Get
			{
				virtual ImagePtr operator()(DevicePtr device, const std::filesystem::path& filename, bool srgb) = 0;
			};
			FLAME_GRAPHICS_EXPORTS static Get& get;
		};

		struct ImageView
		{
			ImagePtr image;
			ImageSub sub;
			ImageSwizzle swizzle;
		};

		struct Sampler
		{
			Filter mag_filter;
			Filter min_filter;
			bool linear_mipmap;
			AddressMode address_mode;

			virtual ~Sampler() {}

			struct Get
			{
				virtual SamplerPtr operator()(DevicePtr device, Filter mag_filter, Filter min_filter, bool linear_mipmap, AddressMode address_mode) = 0;
			};
			FLAME_GRAPHICS_EXPORTS static Get& get;
		};

		struct ImageAtlas
		{
			struct Tile
			{
				uint index;
				std::string name;
				ivec2 pos;
				ivec2 size;
				vec4 uv;
			};

			ImagePtr image;
			std::vector<Tile> tiles;

			virtual ~ImageAtlas() {}

			inline int find_tile(std::string_view name) const
			{
				for (auto id = 0; id < tiles.size(); id++)
				{
					if (tiles[id].name == name)
						return id;
				}
				return -1;
			}

			struct Get
			{
				virtual ImageAtlasPtr operator()(DevicePtr device, const std::filesystem::path& filename) = 0;
			};

			FLAME_GRAPHICS_EXPORTS static Get& get;
		};
	}
}
