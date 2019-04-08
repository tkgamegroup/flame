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

#include "device_private.h"
#include <flame/graphics/image.h>
#include <flame/graphics/renderpass.h>
#include <flame/graphics/framebuffer.h>
#include "commandbuffer_private.h"
#include "swapchain_private.h"
#include "synchronize_private.h"

#include <flame/type.h>
#ifdef FLAME_ANDROID
#include <android_native_app_glue.h>
#endif

namespace flame
{
	namespace graphics
	{
		static auto swapchain_format = Format_Swapchain_B8G8R8A8_UNORM;

		Format get_swapchain_format()
		{
			return swapchain_format;
		}

		inline SwapchainPrivate::SwapchainPrivate(Device *_d, Window *_w, SampleCount _sc)
		{
			d = (DevicePrivate*)_d;
			w = _w;
			sc = _sc;

			image_ms = nullptr;

			RenderpassInfo rp_info;
			if (sc != SampleCount_1)
			{
				rp_info.attachments.emplace_back(swapchain_format, true, sc);
				rp_info.attachments.emplace_back(swapchain_format, false, SampleCount_1);
			}
			else
				rp_info.attachments.emplace_back(swapchain_format, true, SampleCount_1);
			rp_info.subpasses[0].color_attachments.push_back(0);
			if (sc != SampleCount_1)
				rp_info.subpasses[0].resolve_attachments.push_back(1);
			rp = Renderpass::get(d, rp_info);
			rp_info.attachments[0].clear = false;
			rp_dc = Renderpass::get(d, rp_info);

			fbs[0] = nullptr;
			fbs[1] = nullptr;

			avalible_image_index = -1;

			create();

			auto thiz = this;

			w->add_resize_listener(Function<void(void* c, const Ivec2 & size)>(
				[](void* c, const Ivec2 & size) {
					(*((SwapchainPrivatePtr*)c))->destroy();
					(*((SwapchainPrivatePtr*)c))->create();
				}, sizeof(void*), &thiz));
		}

		inline SwapchainPrivate::~SwapchainPrivate()
		{
			destroy();
		}

		void SwapchainPrivate::create()
		{
			auto size = w->size;
			void* native_images[2];

#if defined(FLAME_VULKAN)

#ifdef FLAME_WINDOWS
			VkWin32SurfaceCreateInfoKHR surface_info;
			surface_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
			surface_info.flags = 0;
			surface_info.pNext = nullptr;
			surface_info.hinstance = (HINSTANCE)get_hinst();
			surface_info.hwnd = (HWND)w->get_native();
			vk_chk_res(vkCreateWin32SurfaceKHR(d->ins, &surface_info, nullptr, &s));
#else
			VkAndroidSurfaceCreateInfoKHR surface_info;
			surface_info.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
			surface_info.flags = 0;
			surface_info.pNext = nullptr;
			surface_info.window = ((android_app*)w->get_native())->window;
#endif

			VkBool32 surface_supported;
			vkGetPhysicalDeviceSurfaceSupportKHR(d->pd, 0, s, &surface_supported);
			assert(surface_supported);

			unsigned int present_mode_count = 0;
			std::vector<VkPresentModeKHR> present_modes;
			vkGetPhysicalDeviceSurfacePresentModesKHR(d->pd, s, &present_mode_count, nullptr);
			present_modes.resize(present_mode_count);
			vkGetPhysicalDeviceSurfacePresentModesKHR(d->pd, s, &present_mode_count, present_modes.data());

			VkSurfaceCapabilitiesKHR surface_capabilities;
			vkGetPhysicalDeviceSurfaceCapabilitiesKHR(d->pd, s, &surface_capabilities);

			size.x = clamp(size.x,
				surface_capabilities.minImageExtent.width,
				surface_capabilities.maxImageExtent.width);
			size.y = clamp(size.y,
				surface_capabilities.minImageExtent.height,
				surface_capabilities.maxImageExtent.height);

			unsigned int surface_format_count = 0;
			std::vector<VkSurfaceFormatKHR> surface_formats;
			vkGetPhysicalDeviceSurfaceFormatsKHR(d->pd, s, &surface_format_count, nullptr);
			assert(surface_format_count > 0);
			surface_formats.resize(surface_format_count);
			vkGetPhysicalDeviceSurfaceFormatsKHR(d->pd, s, &surface_format_count, surface_formats.data());

			swapchain_format = Z(surface_formats[0].format, true);

			VkSwapchainCreateInfoKHR swapchain_info;
			swapchain_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
			swapchain_info.flags = 0;
			swapchain_info.pNext = nullptr;
			swapchain_info.surface = s;
			swapchain_info.minImageCount = 2;
			swapchain_info.imageFormat = Z(swapchain_format);
			swapchain_info.imageColorSpace = surface_formats[0].colorSpace;
			swapchain_info.imageExtent.width = size.x;
			swapchain_info.imageExtent.height = size.y;
			swapchain_info.imageArrayLayers = 1;
			swapchain_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
			swapchain_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			swapchain_info.queueFamilyIndexCount = 0;
			swapchain_info.pQueueFamilyIndices = nullptr;
			swapchain_info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
			swapchain_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
			swapchain_info.presentMode = VK_PRESENT_MODE_FIFO_KHR;
			swapchain_info.clipped = true;
			swapchain_info.oldSwapchain = 0;
			vk_chk_res(vkCreateSwapchainKHR(d->v, &swapchain_info, nullptr, &v));

			VkImage vk_images[2];
			uint image_count = 0;
			vkGetSwapchainImagesKHR(d->v, v, &image_count, nullptr);
			vkGetSwapchainImagesKHR(d->v, v, &image_count, vk_images);
			native_images[0] = vk_images[0];
			native_images[1] = vk_images[1];

#elif defined(FLAME_D3D12)

			HRESULT res;

			DXGI_MODE_DESC backbuffer_desc = {};
			backbuffer_desc.Width = size.x;
			backbuffer_desc.Height = size.y;
			backbuffer_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

			DXGI_SAMPLE_DESC sample_desc = {};
			sample_desc.Count = 1;

			DXGI_SWAP_CHAIN_DESC desc = {};
			desc.BufferCount = 2;
			desc.BufferDesc = backbuffer_desc;
			desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
			desc.OutputWindow = (HWND)(w->get_native());
			desc.SampleDesc = sample_desc;
			desc.Windowed = true;

			IDXGISwapChain* temp_swapchain;
			res = d->factory->CreateSwapChain(((QueuePrivate*)d->gq)->v, &desc, &temp_swapchain);
			assert(SUCCEEDED(res));
			swapchain = (IDXGISwapChain3*)temp_swapchain;

			ID3D12Resource* backbuffers[2];
			for (auto i = 0; i < 2; i++)
			{
				res = swapchain->GetBuffer(i, IID_PPV_ARGS(&backbuffers[i]));
				assert(SUCCEEDED(res));
				native_images[i] = backbuffers[i];
			}
#endif

			for (int i = 0; i < 2; i++)
			{
				images[i] = Image::create_from_native(d, swapchain_format, size, 1, 1, (void*)native_images[i]);

				FramebufferInfo fb_info;
				fb_info.rp = rp;
				if (sc != SampleCount_1)
				{
					if (!image_ms || image_ms->size != size)
					{
						if (image_ms)
							Image::destroy(image_ms);
						image_ms = Image::create(d, swapchain_format, size, 1, 1, sc, ImageUsageAttachment, MemPropDevice);
					}
					fb_info.views.push_back(Imageview::get(image_ms));
				}
				fb_info.views.push_back(Imageview::get(images[i]));
				fbs[i] = Framebuffer::get(d, fb_info);
			}
		}

		void SwapchainPrivate::destroy()
		{
			for (auto i = 0; i < 2; i++)
			{
				Image::destroy(images[i]);
				Framebuffer::release(fbs[i]);
			}
			if (image_ms)
			{
				Image::destroy(image_ms);
				image_ms = nullptr;
			}

#if defined(FLAME_VULKAN)
			vkDestroySwapchainKHR(d->v, v, nullptr);
			vkDestroySurfaceKHR(d->ins, s, nullptr);
#elif defined(FLAME_D3D12)

#endif
		}

		inline void SwapchainPrivate::acquire_image(Semaphore *signal_semaphore)
		{
#if defined(FLAME_VULKAN)
			vk_chk_res(vkAcquireNextImageKHR(d->v, v, UINT64_MAX, ((SemaphorePrivate*)signal_semaphore)->v, VK_NULL_HANDLE, &avalible_image_index));
#elif defined(FLAME_D3D12)

#endif
		}

		Window *Swapchain::window() const
		{
			return ((SwapchainPrivate*)this)->w;
		}

		Image *Swapchain::get_image(int idx) const
		{
			return ((SwapchainPrivate*)this)->images[idx];
		}

		uint Swapchain::get_avalible_image_index() const
		{
			return ((SwapchainPrivate*)this)->avalible_image_index;
		}

		SampleCount Swapchain::get_sample_count() const
		{
			return ((SwapchainPrivate*)this)->sc;
		}

		Renderpass *Swapchain::get_renderpass_clear() const
		{
			return ((SwapchainPrivate*)this)->rp;
		}

		Renderpass *Swapchain::get_renderpass_dont_clear() const
		{
			return ((SwapchainPrivate*)this)->rp_dc;
		}

		Framebuffer *Swapchain::get_framebuffer(uint index) const
		{
			return ((SwapchainPrivate*)this)->fbs[index];
		}

		void Swapchain::acquire_image(Semaphore *signal_semaphore)
		{
			((SwapchainPrivate*)this)->acquire_image(signal_semaphore);
		}

		Swapchain *Swapchain::create(Device *d, Window *w, SampleCount sc)
		{
			return new SwapchainPrivate(d, w, sc);
		}

		void Swapchain::destroy(Swapchain *s)
		{
			delete (SwapchainPrivate*)s;
		}
	}
}

