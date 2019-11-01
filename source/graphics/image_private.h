#pragma once

#include <flame/graphics/image.h>
#include "graphics_private.h"

namespace flame
{
	namespace graphics
	{
		struct DevicePrivate;
		struct ImageviewPrivate;

		struct ImagePrivate : Image
		{
			DevicePrivate* d;
#if defined(FLAME_VULKAN)
			VkDeviceMemory m;
			VkImage v;
#elif defined(FLAME_D3D12)
			ID3D12Resource* v;
#endif
			ImagePrivate(Device* d, Format$ format, const Vec2u& size, uint level, uint layer, SampleCount$ sample_count, ImageUsage$ usage);
			ImagePrivate(Device* d, Format$ format, const Vec2u& size, uint level, uint layer, void* native);
			~ImagePrivate();

			void set_props();

			void init(const Vec4c& col);
			void get_pixels(const Vec2u& offset, const Vec2u& extent, void* dst);
			void set_pixels(const Vec2u& offset, const Vec2u& extent, const void* src);
		};

		struct ImageviewPrivate : Imageview
		{
			ImagePrivate* image;
			DevicePrivate* d;
#if defined(FLAME_VULKAN)
			VkImageView v;
#elif defined(FLAME_D3D12)
			ID3D12DescriptorHeap* v;
#endif
			int ref_count;

			ImageviewPrivate(Image* image, ImageviewType$ type, uint base_level, uint level_count, uint base_layer, uint layer_count, Swizzle$ swizzle_r, Swizzle$ swizzle_g, Swizzle$ swizzle_b, Swizzle$ swizzle_a);
			~ImageviewPrivate();
		};

		inline ImageAspect aspect_from_format(Format$ fmt)
		{
			if (fmt >= Format_Color_Begin && fmt <= Format_Color_End)
				return ImageAspectColor;
			if (fmt >= Format_Depth_Begin && fmt <= Format_Depth_End)
			{
				int a = ImageAspectDepth;
				if (fmt >= Format_DepthStencil_Begin && fmt <= Format_DepthStencil_End)
					a |= ImageAspectStencil;
				return (ImageAspect)a;
			}
			return ImageAspectColor;
		}

		struct SamplerPrivate : Sampler
		{
			DevicePrivate* d;
#if defined(FLAME_VULKAN)
			VkSampler v;
#elif defined(FLAME_D3D12)

#endif

			SamplerPrivate(Device* d, Filter mag_filter, Filter min_filter, bool unnormalized_coordinates);
			~SamplerPrivate();
		};
	}
}

