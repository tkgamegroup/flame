#pragma once

#include <flame/graphics/image.h>
#include "graphics_private.h"

namespace flame
{
	namespace graphics
	{
		struct DevicePrivate;
		struct BufferPrivate;
		struct ImageViewPrivate;
		struct PipelinePrivate;
		struct DescriptorSetPrivate;

		struct ImageBridge : Image
		{
			void save(const wchar_t* filename) override;
		};

		struct ImagePrivate : ImageBridge
		{
			DevicePrivate* device;
			
			Format format;
			std::vector<uvec2> sizes;
			uint levels;
			uint layers;
			SampleCount sample_count;
			ImageUsageFlags usage;
			bool is_cube = false;

			std::filesystem::path filename;

			VkDeviceMemory vk_memory = 0;
			VkImage vk_image = 0;
			std::vector<std::unique_ptr<ImageViewPrivate>> views;

			std::unique_ptr<BufferPrivate> sample_uvs;
			std::unique_ptr<BufferPrivate> sample_res;
			std::unique_ptr<DescriptorSetPrivate> sample_descriptorset;

			void init(const uvec2& size);
			void build_default_views();
			ImagePrivate(DevicePrivate* device, Format format, const uvec2& size, uint levels, uint layers, SampleCount sample_count, ImageUsageFlags usage, bool is_cube = false);
			ImagePrivate(DevicePrivate* device, Format format, const uvec2& size, uint levels, uint layers, void* native);
			~ImagePrivate();

			void release() override { delete this; }

			Format get_format() const override { return format; }
			uvec2 get_size(uint lv) const override { return sizes[lv]; }
			uint get_levels() const override { return levels; }
			uint get_layers() const override { return layers; }
			SampleCount get_sample_count() const override { return sample_count; }

			const wchar_t* get_filename() const override { return filename.c_str(); }

			ImageView* get_view(uint idx) const override { return (ImageView*)views[idx].get(); }

			void get_samples(uint count, const vec2* uvs, vec4* dst) override;

			void save(const std::filesystem::path& filename);

			static ImagePrivate* create(DevicePrivate* device, Bitmap* bmp);
			static ImagePrivate* create(DevicePrivate* device, const std::filesystem::path& filename, bool srgb, ImageUsageFlags additional_usage = ImageUsageNone, bool is_cube = false, bool generate_mipmaps = false);
		};

		inline void ImageBridge::save(const wchar_t* filename)
		{
			((ImagePrivate*)this)->save(filename);
		}

		struct ImageViewPrivate : ImageView
		{
			DevicePrivate* device;
			ImagePrivate* image;

			ImageViewType type;
			ImageSubresource subresource;
			ImageSwizzle swizzle;

			VkImageView vk_image_view;

			ImageViewPrivate(ImagePrivate* image, bool auto_released, ImageViewType type = ImageView2D, const ImageSubresource& subresource = {}, const ImageSwizzle& swizzle = {});
			~ImageViewPrivate();

			void release() override { delete this; }

			ImageViewType get_type() const override { return type; }
			ImageSubresource get_subresource() const override { return subresource; }
			ImageSwizzle get_swizzle() const override { return swizzle; }

			Image* get_image() const override { return image; }
		};

		inline ImageAspectFlags aspect_from_format(Format fmt)
		{
			if (fmt >= Format_Color_Begin && fmt <= Format_Color_End)
				return ImageAspectColor;
			if (fmt >= Format_Depth_Begin && fmt <= Format_Depth_End)
			{
				auto a = (int)ImageAspectDepth;
				if (fmt >= Format_DepthStencil_Begin && fmt <= Format_DepthStencil_End)
					a |= ImageAspectStencil;
				return (ImageAspectFlags)a;
			}
			return ImageAspectColor;
		}

		struct SamplerPrivate : Sampler
		{
			DevicePrivate* device;

			Filter mag_filter;
			Filter min_filter;
			bool linear_mipmap;
			AddressMode address_mode;

			VkSampler vk_sampler;

			SamplerPrivate(DevicePrivate* device, Filter mag_filter, Filter min_filter, bool linear_mipmap, AddressMode address_mode);
			~SamplerPrivate();

			void release() override { delete this; }

			static SamplerPrivate* get(DevicePrivate* device, Filter mag_filter, Filter min_filter, bool linear_mipmap, AddressMode address_mode);
		};

		struct ImageTilePrivate : ImageTile
		{
			uint index;
			std::string name;
			ivec2 pos;
			ivec2 size;
			vec4 uv;

			uint get_index() const override { return index; }
			const char* get_name() const override { return name.c_str(); }
			ivec2 get_pos() const override { return pos; }
			ivec2 get_size() const override { return size; }
			vec4 get_uv() const override { return uv; }
		};

		struct ImageAtlasBridge : ImageAtlas
		{
			ImageTile* get_tile(uint id) const override;
			ImageTile* find_tile(const char* name) const override;
		};

		struct ImageAtlasPrivate : ImageAtlasBridge
		{
			bool border = false;

			ImagePrivate* image;
			std::vector<std::unique_ptr<ImageTilePrivate>> tiles;

			ImageAtlasPrivate(DevicePrivate* device, const std::wstring& atlas_filename);
			~ImageAtlasPrivate();

			void release() override { delete this; }

			bool get_border() const override { return border; }

			ImageTilePrivate* get_tile(uint id) const override { return tiles[id].get(); }
			ImageTilePrivate* find_tile(const std::string& name) const;

			Image* get_image() const override { return image; }
		};

		inline ImageTile* ImageAtlasBridge::get_tile(uint id) const
		{
			return ((ImageAtlasPrivate*)this)->get_tile(id);
		}

		inline ImageTile* ImageAtlasBridge::find_tile(const char* name) const
		{
			return ((ImageAtlasPrivate*)this)->find_tile(name);
		}
	}
}

