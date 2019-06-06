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

#include <flame/graphics/renderpass.h>
#include "graphics_private.h"

namespace flame
{
	namespace graphics
	{
		struct DevicePrivate;

		struct RenderpassPrivate : Renderpass
		{
			DevicePrivate *d;
			std::vector<Format$> attachments;
#if defined(FLAME_VULKAN)
			VkRenderPass v;
#endif
			RenderpassPrivate(Device *d, const RenderpassInfo& info);
			~RenderpassPrivate();
		};

		struct ClearvaluesPrivate : Clearvalues
		{
			RenderpassPrivate* renderpass;

#if defined(FLAME_VULKAN)
			std::vector<VkClearValue> v;
#elif defined(FLAME_D3D12)
			std::vector<Vec4f> v;
#endif

			ClearvaluesPrivate(Renderpass *r);
			~ClearvaluesPrivate();

			void set(int idx, const Vec4c& col);
		};

		struct FramebufferPrivate : Framebuffer
		{
			DevicePrivate* d;
			FramebufferInfo info;
#if defined(FLAME_VULKAN)
			VkFramebuffer v;
#elif defined(FLAME_D3D12)

#endif
			FramebufferPrivate(Device* d, const FramebufferInfo& info);
			~FramebufferPrivate();
		};
	}
}
