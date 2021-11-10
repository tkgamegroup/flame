#pragma once

#include "image.h"
#include "graphics_private.h"

namespace flame
{
	namespace graphics
	{
		struct ImagePrivate : Image
		{
			struct Data
			{
				Format format;
				uvec2 size;
				uint pixel_size;
				uint pitch;
				uint data_size;
				std::unique_ptr<uchar> p;

				vec4 get_pixel(ivec2 pos);
				void set_pixel(ivec2 pos, const vec4& v);
			};

			DevicePrivate* device;
			
			Format format;
			std::vector<uvec2> sizes;
			uint levels;
			uint layers;
			SampleCount sample_count;
			ImageUsageFlags usage;
			bool is_cube = false;

			std::filesystem::path filename;
			bool srgb = false;

			VkDeviceMemory vk_memory = 0;
			VkImage vk_image = 0;
			std::map<uint64, std::unique_ptr<ImageViewPrivate>> views;
			std::map<uint64, std::unique_ptr<DescriptorSetPrivate>> read_dss;
			std::map<uint64, std::unique_ptr<FramebufferPrivate>> write_fbs;
			std::vector<std::vector<Data>> data;
			uint data_size;

			void initialize(const uvec2& size);
			ImagePrivate(DevicePrivate* device, Format format, const uvec2& size, uint levels, uint layers, SampleCount sample_count, 
				ImageUsageFlags usage, bool is_cube = false);
			ImagePrivate(DevicePrivate* device, Format format, const uvec2& size, uint levels, uint layers, void* native);
			~ImagePrivate();

			void release() override;

			Format get_format() const override { return format; }
			uvec2 get_size(uint lv) const override { return sizes[lv]; }
			uint get_levels() const override { return levels; }
			uint get_layers() const override { return layers; }
			SampleCount get_sample_count() const override { return sample_count; }

			const wchar_t* get_filename() const override { return filename.c_str(); }

			Data& get_data(uint level, uint layer);

			ImageViewPtr get_view(const ImageSub& sub = {}, const ImageSwizzle& swizzle = {}) override;
			DescriptorSetPtr get_shader_read_src(uint base_level = 0, uint base_layer = 0, SamplerPtr sp = nullptr) override;
			FramebufferPtr get_shader_write_dst(uint base_level = 0, uint base_layer = 0, AttachmentLoadOp load_op = AttachmentLoadDontCare) override;

			void change_layout(ImageLayout src_layout, ImageLayout dst_layout) override;
			void clear(ImageLayout src_layout, ImageLayout dst_layout, const cvec4& color) override;

			vec4 linear_sample(const vec2& uv, uint level, uint layer) override;

			void generate_mipmaps() override;
			float alpha_test_coverage(uint level, float ref, uint channel, float scale) override;
			void scale_alpha_to_coverage(uint level, float desired, float ref, uint channel) override;

			void save(const std::filesystem::path& filename);
			void save(const wchar_t* filename) override { save(std::filesystem::path(filename)); }

			static ImagePrivate* create(DevicePrivate* device, Bitmap* bmp);
			static ImagePrivate* create(DevicePrivate* device, Format format, const uvec2& size, void* data);
			static ImagePrivate* get(DevicePrivate* device, const std::filesystem::path& filename, bool srgb);
		};

		extern std::vector<ImagePrivate*> __images;

		struct ImageViewPrivate : ImageView
		{
			DevicePtr device;
			ImagePtr image;

			ImageSub sub;
			ImageSwizzle swizzle;

			VkImageView vk_image_view;

			ImageViewPrivate(ImagePrivate* image, const ImageSub& sub = {}, const ImageSwizzle& swizzle = {});
			~ImageViewPrivate();

			ImageSub get_sub() const override { return sub; }
			ImageSwizzle get_swizzle() const override { return swizzle; }

			ImagePtr get_image() const override { return image; }
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

		struct ImageAtlasPrivate : ImageAtlas
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

			ImageAtlasPrivate(DevicePrivate* device, const std::filesystem::path& atlas_filename);
			~ImageAtlasPrivate();

			void release() override { delete this; }

			uint get_tiles_count() const override { return tiles.size(); };
			void get_tile(uint id, TileInfo* dst) const override;
			int find_tile(std::string_view name) const;
			bool find_tile(const char* name, TileInfo* dst) const override;

			ImagePtr get_image() const override { return image; }
		};
	}
}

