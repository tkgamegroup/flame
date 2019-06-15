#include "device_private.h"
#include <flame/graphics/image.h>
#include <flame/graphics/renderpass.h>
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

		Format$ get_swapchain_format()
		{
			return swapchain_format;
		}

		SwapchainPrivate::SwapchainPrivate(Device *_d, Window *_w)
		{
			d = (DevicePrivate*)_d;
			w = _w;

			image_index = 0;

			create();

			auto thiz = this;

			w->add_resize_listener(Function<void(void* c, const Vec2u & size)>(
				[](void* c, const Vec2u & size) {
					(*((SwapchainPrivatePtr*)c))->destroy();
					(*((SwapchainPrivatePtr*)c))->create();
				}, sizeof(void*), &thiz));
		}

		SwapchainPrivate::~SwapchainPrivate()
		{
			destroy();
		}

		void SwapchainPrivate::create()
		{
			auto size = w->size;
			uint image_count = 3;

#if defined(FLAME_VULKAN)

#ifdef FLAME_WINDOWS
			VkWin32SurfaceCreateInfoKHR surface_info;
			surface_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
			surface_info.flags = 0;
			surface_info.pNext = nullptr;
			surface_info.hinstance = (HINSTANCE)get_hinst();
			surface_info.hwnd = (HWND)w->get_native();
			chk_res(vkCreateWin32SurfaceKHR(d->ins, &surface_info, nullptr, &s));
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

			size.x() = clamp(size.x(),
				surface_capabilities.minImageExtent.width,
				surface_capabilities.maxImageExtent.width);
			size.y() = clamp(size.y(),
				surface_capabilities.minImageExtent.height,
				surface_capabilities.maxImageExtent.height);

			unsigned int surface_format_count = 0;
			std::vector<VkSurfaceFormatKHR> surface_formats;
			vkGetPhysicalDeviceSurfaceFormatsKHR(d->pd, s, &surface_format_count, nullptr);
			assert(surface_format_count > 0);
			surface_formats.resize(surface_format_count);
			vkGetPhysicalDeviceSurfaceFormatsKHR(d->pd, s, &surface_format_count, surface_formats.data());

			swapchain_format = to_enum(surface_formats[0].format, true);

			VkSwapchainCreateInfoKHR swapchain_info;
			swapchain_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
			swapchain_info.flags = 0;
			swapchain_info.pNext = nullptr;
			swapchain_info.surface = s;
			swapchain_info.minImageCount = image_count;
			swapchain_info.imageFormat = to_enum(swapchain_format);
			swapchain_info.imageColorSpace = surface_formats[0].colorSpace;
			swapchain_info.imageExtent.width = size.x();
			swapchain_info.imageExtent.height = size.y();
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
			chk_res(vkCreateSwapchainKHR(d->v, &swapchain_info, nullptr, &v));

			std::vector<VkImage> native_images;
			vkGetSwapchainImagesKHR(d->v, v, &image_count, nullptr);
			native_images.resize(image_count);
			vkGetSwapchainImagesKHR(d->v, v, &image_count, native_images.data());

#elif defined(FLAME_D3D12)

			HRESULT res;

			DXGI_MODE_DESC backbuffer_desc = {};
			backbuffer_desc.Width = size.x();
			backbuffer_desc.Height = size.y();
			backbuffer_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

			DXGI_SAMPLE_DESC sample_desc = {};
			sample_desc.Count = 1;

			DXGI_SWAP_CHAIN_DESC desc = {};
			desc.BufferCount = image_count;
			desc.BufferDesc = backbuffer_desc;
			desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
			desc.OutputWindow = (HWND)(w->get_native());
			desc.SampleDesc = sample_desc;
			desc.Windowed = true;

			IDXGISwapChain* temp_swapchain;
			res = d->factory->CreateSwapChain(((QueuePrivate*)d->gq)->v, &desc, &temp_swapchain);
			assert(SUCCEEDED(res));
			v = (IDXGISwapChain3*)temp_swapchain;

			std::vector<void*> vk_images;

			std::vector<ID3D12Resource*> native_images;
			native_images.resize(image_count);
			for (auto i = 0; i < image_count; i++)
			{
				res = v->GetBuffer(i, IID_PPV_ARGS(&native_images[i]));
				assert(SUCCEEDED(res));
			}
#endif

			images.resize(image_count);
			for (auto i = 0; i < image_count; i++)
				images[i] = Image::create_from_native(d, swapchain_format, size, 1, 1, native_images[i]);
		}

		void SwapchainPrivate::destroy()
		{
			for (auto i : images)
				Image::destroy(i);

#if defined(FLAME_VULKAN)
			vkDestroySwapchainKHR(d->v, v, nullptr);
			vkDestroySurfaceKHR(d->ins, s, nullptr);
#elif defined(FLAME_D3D12)

#endif
		}

		void SwapchainPrivate::acquire_image(Semaphore *signal_semaphore)
		{
#if defined(FLAME_VULKAN)
			chk_res(vkAcquireNextImageKHR(d->v, v, UINT64_MAX, signal_semaphore ? ((SemaphorePrivate*)signal_semaphore)->v : nullptr, VK_NULL_HANDLE, &image_index));
#elif defined(FLAME_D3D12)
			image_index = v->GetCurrentBackBufferIndex();
#endif
		}

		Window *Swapchain::window() const
		{
			return ((SwapchainPrivate*)this)->w;
		}

		int Swapchain::image_count() const
		{
			return ((SwapchainPrivate*)this)->images.size();
		}

		Image *Swapchain::image(uint idx) const
		{
			return ((SwapchainPrivate*)this)->images[idx];
		}

		uint Swapchain::image_index() const
		{
			return ((SwapchainPrivate*)this)->image_index;
		}

		void Swapchain::acquire_image(Semaphore *signal_semaphore)
		{
			((SwapchainPrivate*)this)->acquire_image(signal_semaphore);
		}

		Swapchain *Swapchain::create(Device *d, Window *w)
		{
			return new SwapchainPrivate(d, w);
		}

		void Swapchain::destroy(Swapchain *s)
		{
			delete (SwapchainPrivate*)s;
		}

		struct Swapchain$
		{
			void* in$i;

			void* out$o;
			void* window$o;
			Vec2u size$o;
			Format$ format$o;
			Array<void*> images$o;
			Array<void*> imageviews$o;

			FLAME_GRAPHICS_EXPORTS void initialize$()
			{
				if (in$i)
				{
					out$o = in$i;
					auto sc = (graphics::Swapchain*)out$o;
					auto image_count = sc->image_count();
					if (image_count > 0)
					{
						images$o.size = image_count;
						images$o.v = new void* [image_count];
						for (auto i = 0; i < image_count; i++)
							images$o.v[i] = sc->image(i);
						imageviews$o.size = image_count;
						imageviews$o.v = new void* [image_count];
						for (auto i = 0; i < image_count; i++)
							imageviews$o.v[i] = Imageview::create((Image*)images$o.v[i]);

						auto i = (Image*)images$o.v[0];
						size$o = i->size;
						format$o = i->format;
					}
				}
			}

			FLAME_GRAPHICS_EXPORTS void finish$()
			{
				delete[]images$o.v;
				for (auto i = 0; i < imageviews$o.size; i++)
					Imageview::destroy((Imageview*)imageviews$o.v[i]);
				delete[]imageviews$o.v;
			}

		}bp_swapchain_unused;
	}
}

