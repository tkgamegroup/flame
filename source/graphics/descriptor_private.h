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

#pragma once

#include <flame/graphics/descriptor.h>
#include "graphics_private.h"

namespace flame
{
	namespace graphics
	{
		struct DevicePrivate;

		struct DescriptorpoolPrivate : Descriptorpool
		{
			DevicePrivate *d;
#if defined(FLAME_VULKAN)
			VkDescriptorPool v;
#elif defined(FLAME_D3D12)

#endif

			DescriptorpoolPrivate(Device *d);
			~DescriptorpoolPrivate();
		};

		struct DescriptorsetlayoutPrivate : Descriptorsetlayout
		{
			std::vector<Binding> bindings;

			DevicePrivate *d;
#if defined(FLAME_VULKAN)
			VkDescriptorSetLayout v;
#elif defined(FLAME_D3D12)

#endif
			DescriptorsetlayoutPrivate(Device *d, const std::vector<Binding> &_bindings);
			~DescriptorsetlayoutPrivate();
		};

		struct DescriptorsetPrivate : Descriptorset
		{
			DescriptorpoolPrivate *p;
#if defined(FLAME_VULKAN)
			VkDescriptorSet v;
#elif defined(FLAME_D3D12)

#endif

			DescriptorsetPrivate(Descriptorpool *p, Descriptorsetlayout *l);
			~DescriptorsetPrivate();

			void set_uniformbuffer(uint binding, uint index, Buffer *b, uint offset = 0, uint range = 0);
			void set_storagebuffer(uint binding, uint index, Buffer *b, uint offset = 0, uint range = 0);
			void set_imageview(uint binding, uint index, Imageview *iv, Sampler *sampler);
			void set_storageimage(uint binding, uint index, Imageview *iv);
		};
	}
}
