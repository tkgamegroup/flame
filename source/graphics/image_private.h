// MIT License
// 
// Copyright (c) 2018 wjs
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <flame/graphics/image.h>
#include "graphics_private.h"

#include <vector>

namespace flame
{
	namespace graphics
	{
		struct DevicePrivate;
		struct ImageviewPrivate;

		struct ImagePrivate : Image
		{
			int usage;
			int mem_prop;

			DevicePrivate *d;
			std::vector<ImageviewPrivate*> views;
#if defined(FLAME_VULKAN)
			VkDeviceMemory m;
			VkImage v;
#elif defined(FLAME_D3D12)

#endif
			ImagePrivate(Device *d, Format format, const Ivec2 &size, int level, int layer, SampleCount sample_count, int usage, int mem_prop);
			ImagePrivate(Device *d, Format format, const Ivec2 &size, int level, int layer, void *native);
			~ImagePrivate();

			void set_props();

			void init(const Bvec4 &col);
			void get_pixels(int x, int y, int cx, int cy, void *dst);
			void set_pixels(int x, int y, int cx, int cy, const void *src);

			void save_png(const wchar_t *filename);
		};

		struct ImageviewPrivate : Imageview
		{
			ImagePrivate *i;
#if defined(FLAME_VULKAN)
			VkImageView v;
#elif defined(FLAME_D3D12)

#endif
			int ref_count;

			ImageviewPrivate(Image *i, ImageviewType type = Imageview2D, int base_level = 0, int level_count = 1, int base_layer = 0, int layer_count = 1, ComponentMapping *mapping = nullptr);
			~ImageviewPrivate();

			bool same(Image *i, ImageviewType type, int base_level, int level_count, int base_layer, int layer_count, ComponentMapping *mapping);
		};

		inline ImageAspect aspect_from_format(Format fmt)
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
			DevicePrivate *d;
#if defined(FLAME_VULKAN)
			VkSampler v;
#elif defined(FLAME_D3D12)

#endif

			SamplerPrivate(Device *d, Filter mag_filter, Filter min_filter, bool unnormalized_coordinates);
			~SamplerPrivate();
		};
	}
}

