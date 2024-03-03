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
			case Format_BC4_UNORM: case Format_BC4_SNORM:
				return 1;
			case Format_R16_UNORM:
			case Format_Depth16:
				return 2;
			case Format_R32_SFLOAT:
			case Format_R8G8B8A8_UNORM:
			case Format_B8G8R8A8_UNORM:
			case Format_A2R10G10B10_UNORM:
			case Format_B10G11R11_UFLOAT:
			case Format_BC7_UNORM:
			case Format_Depth32:
				return 4;
			case Format_R16G16B16A16_UNORM: 
			case Format_R16G16B16A16_SFLOAT:
				return 8;
			case Format_R32G32B32A32_SFLOAT:
				return 16;
			}
			return 0;
		}

		inline uint get_num_channels(Format format)
		{
			switch (format)
			{
			case Format_R8_UNORM:
			case Format_R16_UNORM:
			case Format_R32_SFLOAT:
			case Format_Depth16:
			case Format_BC4_UNORM: 
			case Format_BC4_SNORM:
				return 1;
			case Format_R8G8B8A8_UNORM: 
			case Format_B8G8R8A8_UNORM:
			case Format_R16G16B16A16_UNORM: 
			case Format_R16G16B16A16_SFLOAT:
			case Format_R32G32B32A32_SFLOAT:
			case Format_BC7_UNORM:
				return 4;
			}
			return 0;
		}

		struct ImageConfig
		{
			bool srgb = false;
			bool auto_mipmapping = false;
			float alpha_test = 0.f;
			vec4 border = vec4(0.f);
			uvec2 sheet_size = uvec2(1);
		};

		// Reflect
		struct Image
		{
			struct Layer
			{
				ImageLayout layout = ImageLayoutUndefined;
				std::unique_ptr<uchar[]> staging_data;
			};

			struct Level
			{
				uvec2 extent = uvec2(0);
				uint pitch = 0;
				uint data_size = 0;
				std::vector<Layer> layers;
			};

			Format format = Format_R8G8B8A8_UNORM;
			ImageUsageFlags usage;
			uvec3 extent = uvec3(0);
			uint n_channels = 0;
			uint pixel_size = 0;
			uint n_levels = 1;
			uint n_layers = 1;
			SampleCount sample_count = SampleCount_1;
			std::vector<Level> levels;
			uint data_size;

			std::filesystem::path filename;
			uint ref = 0;

			std::vector<std::pair<uint, void*>> dependencies;

			virtual ~Image() {}

			virtual ImageLayout get_layout(const ImageSub& sub = {}) = 0;
			virtual ImageViewPtr get_view(const ImageSub& sub = {}, const ImageSwizzle& swizzle = {}, bool cube = false) = 0;
			virtual DescriptorSetPtr get_shader_read_src(uint base_level = 0, uint base_layer = 0, SamplerPtr sp = nullptr, const ImageSwizzle& swizzle = {}) = 0;
			virtual FramebufferPtr get_shader_write_dst(uint base_level = 0, uint base_layer = 0, AttachmentLoadOp load_op = AttachmentLoadDontCare) = 0;

			virtual void change_layout(ImageLayout dst_layout) = 0;
			virtual void clear(const vec4& color, ImageLayout dst_layout) = 0;

			virtual vec4 get_pixel(int x, int y, uint level, uint layer) = 0;
			virtual vec4 get_staging_pixel(int x, int y, uint level, uint layer) = 0;
			virtual void set_staging_pixel(int x, int y, uint level, uint layer, const vec4& v) = 0;
			virtual void upload_staging_pixels(int x, int y, int w, int h, uint level, uint layer) = 0;
			virtual vec4 linear_sample_staging_pixels(const vec2& uv, uint level = 0, uint layer = 0) = 0;
			virtual void clear_staging_pixels() = 0;

			virtual void save(const std::filesystem::path& filename, bool compress = false) = 0;

			struct Create
			{
				virtual ImagePtr operator()(Format format, const uvec3& extent, ImageUsageFlags usage, uint levels = 1, uint layers = 1, SampleCount sample_count = SampleCount_1) = 0;
				virtual ImagePtr operator()(Format format, const uvec3& extent, void* data) = 0;
			};
			FLAME_GRAPHICS_API static Create& create;

			struct Get
			{
				virtual ImagePtr operator()(const std::filesystem::path& filename) = 0;
			};
			FLAME_GRAPHICS_API static Get& get;

			struct GetConfig
			{
				virtual void operator()(const std::filesystem::path& filename, ImageConfig* out) = 0;
			};
			FLAME_GRAPHICS_API static GetConfig& get_config;

			struct Release
			{
				virtual void operator()(ImagePtr image) = 0;
			};
			FLAME_GRAPHICS_API static Release& release;
		};

		struct ImageView
		{
			ImagePtr image;
			ImageSub sub;
			ImageSwizzle swizzle;

			virtual DescriptorSetPtr get_shader_read_src(SamplerPtr sp = nullptr) = 0;
		};

		struct Sampler
		{
			Filter mag_filter;
			Filter min_filter;
			bool linear_mipmap;
			AddressMode address_mode;
			BorderColor border_color;

			virtual ~Sampler() {}

			struct Create
			{
				virtual SamplerPtr operator()(Filter mag_filter, Filter min_filter, bool linear_mipmap, AddressMode address_mode, float custom_border_color) = 0;
			};
			FLAME_GRAPHICS_API static Create& create;

			struct Get
			{
				virtual SamplerPtr operator()(Filter mag_filter, Filter min_filter, bool linear_mipmap, AddressMode address_mode, BorderColor border_color = BorderColorWhite) = 0;
			};
			FLAME_GRAPHICS_API static Get& get;
		};

		// Reflect
		struct ImageAtlas
		{
			struct Item
			{
				vec4 uvs;
				vec4 border;
			};

			ImagePtr image;
			ImageViewPtr view;
			std::unordered_map<uint, Item> items;

			std::filesystem::path filename;
			uint ref = 0;

			inline ImageDesc get_item(uint name)
			{
				ImageDesc ret;
				ret.view = nullptr;
				if (auto it = items.find(name); it != items.end())
				{
					ret.view = view;
					ret.uvs = it->second.uvs;
					auto sz = (vec2)((Image*)image)->extent.xy();
					ret.border_uvs.xy = it->second.border.xy / sz;
					ret.border_uvs.zw = vec2(1.f) - it->second.border.zw / sz;
				}
				return ret;
			}

			struct Generate
			{
				virtual void operator()(const std::filesystem::path& folder) = 0;
			};
			FLAME_GRAPHICS_API static Generate& generate;

			struct Get
			{
				// should use the .png file
				virtual ImageAtlasPtr operator()(const std::filesystem::path& filename) = 0;
			};
			FLAME_GRAPHICS_API static Get& get;

			struct Release
			{
				virtual void operator()(ImageAtlasPtr atlas) = 0;
			};
			FLAME_GRAPHICS_API static Release& release;
		};
	}
}
