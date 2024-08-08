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
#if USE_VULKAN
			VkRenderPass vk_renderpass = 0;
#endif

			~RenderpassPrivate();
		};
#if USE_D3D12
		extern ID3D12DescriptorHeap* d3d12_rtv_heap;
		extern SparseRanges d3d12_rtv_free_list;
		extern ID3D12DescriptorHeap* d3d12_dsv_heap;
		extern SparseRanges d3d12_dsv_free_list;
#endif

		struct FramebufferPrivate : Framebuffer
		{
#if USE_D3D12
			int d3d12_rtv_off = -1;
			uint d3d12_rtv_num = 0;
			D3D12_CPU_DESCRIPTOR_HANDLE d3d12_rtv_cpu_handle;
			int d3d12_dsv_off = -1;
			D3D12_CPU_DESCRIPTOR_HANDLE d3d12_dsv_cpu_handle;
#elif USE_VULKAN
			VkFramebuffer vk_framebuffer = 0;
#endif

			~FramebufferPrivate();
		};
	}
}
