#pragma once

#include "renderpass.h"
#include "graphics_private.h"
#include "image_private.h"

namespace flame
{
	namespace graphics
	{
		struct RenderpassPrivate : Renderpass
		{
			VkRenderPass vk_renderpass;

			~RenderpassPrivate();
		};

		struct FramebufferPrivate : Framebuffer
		{
			VkFramebuffer vk_framebuffer;
			ID3D12DescriptorHeap* d3d12_targets_heap = nullptr;

			~FramebufferPrivate();
		};
	}
}
