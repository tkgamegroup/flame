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

#include <vector>

namespace flame
{
	namespace graphics
	{
		struct DevicePrivate;

		struct DescriptorpoolPrivate : Descriptorpool
		{
			DevicePrivate *d;
			VkDescriptorPool v;

			DescriptorpoolPrivate(Device *d);
			~DescriptorpoolPrivate();
		};

		struct DescriptorsetlayoutPrivate : Descriptorsetlayout
		{
			std::vector<Binding> bindings;

			DevicePrivate *d;
			VkDescriptorSetLayout v;

			int ref_count;

			DescriptorsetlayoutPrivate(Device *d, const std::vector<Binding> &_bindings);
			~DescriptorsetlayoutPrivate();
		};

		struct DescriptorsetPrivate : Descriptorset
		{
			DescriptorpoolPrivate *p;
			VkDescriptorSet v;

			DescriptorsetPrivate(Descriptorpool *p, Descriptorsetlayout *l);
			~DescriptorsetPrivate();

			void set_uniformbuffer(int binding, int index, Buffer *b, int offset = 0, int range = 0);
			void set_storagebuffer(int binding, int index, Buffer *b, int offset = 0, int range = 0);
			void set_imageview(int binding, int index, Imageview *iv, Sampler *sampler);
			void set_storageimage(int binding, int index, Imageview *iv);
		};
	}
}
