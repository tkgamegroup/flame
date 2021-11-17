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
			DevicePrivate* device;
			VkRenderPass vk_renderpass;

			~RenderpassPrivate();
		};

		extern std::vector<RenderpassPrivate*> __renderpasses;

		struct FramebufferPrivate : Framebuffer
		{
			DevicePrivate* device;
			VkFramebuffer vk_framebuffer;

			~FramebufferPrivate();
		};
	}
}
