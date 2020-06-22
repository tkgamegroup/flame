#include "device_private.h"
#include "synchronize_private.h"
#include "image_private.h"
#include <flame/graphics/renderpass.h>
#include "commandbuffer_private.h"
#include "swapchain_private.h"

#ifdef FLAME_ANDROID
#include <android_native_app_glue.h>
#endif

namespace flame
{
	namespace graphics
	{
		static auto swapchain_format = Format_Swapchain_B8G8R8A8_UNORM;

		SwapchainPrivate::SwapchainPrivate(Device* _d, Window* w, ImageUsageFlags extra_usages = 0) :
			_d((DevicePrivate*)_d),
			_w(w),
			_extra_usages(extra_usages),
			_s(nullptr),
			_v(nullptr)
		{
			update();

			resize_listener = w->add_resize_listener([](Capture& c, const Vec2u& size) {
				c.thiz<SwapchainPrivate>()->update();
			}, Capture().set_thiz(this));
			w->add_destroy_listener([](Capture& c) {
				c.thiz<SwapchainPrivate>()->_w = nullptr;
			}, Capture().set_thiz(this));

			_image_avalible.reset(new SemaphorePrivate(_d));
		}

		SwapchainPrivate::~SwapchainPrivate()
		{
			if (_w)
				_w->remove_resize_listener(resize_listener);

#if defined(FLAME_VULKAN)
			if (_v)
				vkDestroySwapchainKHR(_d->_v, _v, nullptr);
			if (_s)
				vkDestroySurfaceKHR(_d->_instance, _s, nullptr);
#elif defined(FLAME_D3D12)

#endif
		}

		void SwapchainPrivate::_acquire_image()
		{
#if defined(FLAME_VULKAN)
			chk_res(vkAcquireNextImageKHR(_d->_v, _v, UINT64_MAX, _image_avalible->v, VK_NULL_HANDLE, &_image_index));
#elif defined(FLAME_D3D12)
			image_index = v->GetCurrentBackBufferIndex();
#endif
		}

		void SwapchainPrivate::update()
		{
			_d->_graphics_queue.get()->_wait_idle();

			_images.clear();

#if defined(FLAME_VULKAN)
			if (_v)
			{
				vkDestroySwapchainKHR(_d->_v, _v, nullptr);
				_v = nullptr;
			}
			if (_s)
			{
				vkDestroySurfaceKHR(_d->_instance, _s, nullptr);
				_s = nullptr;
			}
#elif defined(FLAME_D3D12)

#endif

			auto size = _w->get_size();
			if (size != 0U)
			{
				uint image_count = 3;

#if defined(FLAME_VULKAN)

#ifdef FLAME_WINDOWS
				VkWin32SurfaceCreateInfoKHR surface_info;
				surface_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
				surface_info.flags = 0;
				surface_info.pNext = nullptr;
				surface_info.hinstance = (HINSTANCE)get_hinst();
				surface_info.hwnd = (HWND)_w->get_native();
				chk_res(vkCreateWin32SurfaceKHR(_d->_instance, &surface_info, nullptr, &_s));
#else
				VkAndroidSurfaceCreateInfoKHR surface_info;
				surface_info.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
				surface_info.flags = 0;
				surface_info.pNext = nullptr;
				surface_info.window = ((android_app*)w->get_native())->window;
#endif

				VkBool32 surface_supported;
				vkGetPhysicalDeviceSurfaceSupportKHR(_d->_physical_device, 0, _s, &surface_supported);
				assert(surface_supported);

				unsigned int present_mode_count = 0;
				std::vector<VkPresentModeKHR> present_modes;
				vkGetPhysicalDeviceSurfacePresentModesKHR(_d->_physical_device, _s, &present_mode_count, nullptr);
				present_modes.resize(present_mode_count);
				vkGetPhysicalDeviceSurfacePresentModesKHR(_d->_physical_device, _s, &present_mode_count, present_modes.data());

				VkSurfaceCapabilitiesKHR surface_capabilities;
				vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_d->_physical_device, _s, &surface_capabilities);

				size.x() = clamp(size.x(),
					surface_capabilities.minImageExtent.width,
					surface_capabilities.maxImageExtent.width);
				size.y() = clamp(size.y(),
					surface_capabilities.minImageExtent.height,
					surface_capabilities.maxImageExtent.height);

				unsigned int surface_format_count = 0;
				std::vector<VkSurfaceFormatKHR> surface_formats;
				vkGetPhysicalDeviceSurfaceFormatsKHR(_d->_physical_device, _s, &surface_format_count, nullptr);
				assert(surface_format_count > 0);
				surface_formats.resize(surface_format_count);
				vkGetPhysicalDeviceSurfaceFormatsKHR(_d->_physical_device, _s, &surface_format_count, surface_formats.data());

				swapchain_format = graphics::get_format(surface_formats[0].format, true);

				VkSwapchainCreateInfoKHR swapchain_info;
				swapchain_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
				swapchain_info.flags = 0;
				swapchain_info.pNext = nullptr;
				swapchain_info.surface = _s;
				swapchain_info.minImageCount = image_count;
				swapchain_info.imageFormat = to_backend(swapchain_format);
				swapchain_info.imageColorSpace = surface_formats[0].colorSpace;
				swapchain_info.imageExtent.width = size.x();
				swapchain_info.imageExtent.height = size.y();
				swapchain_info.imageArrayLayers = 1;
				swapchain_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | (_extra_usages ? get_backend_image_usage_flags(_extra_usages) : 0);
				swapchain_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
				swapchain_info.queueFamilyIndexCount = 0;
				swapchain_info.pQueueFamilyIndices = nullptr;
				swapchain_info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
				swapchain_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
				swapchain_info.presentMode = VK_PRESENT_MODE_FIFO_KHR;
				swapchain_info.clipped = true;
				swapchain_info.oldSwapchain = 0;
				chk_res(vkCreateSwapchainKHR(_d->_v, &swapchain_info, nullptr, &_v));

				std::vector<VkImage> native_images;
				vkGetSwapchainImagesKHR(_d->_v, _v, &image_count, nullptr);
				native_images.resize(image_count);
				vkGetSwapchainImagesKHR(_d->_v, _v, &image_count, native_images.data());

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

				_images.resize(image_count);
				for (auto i = 0; i < image_count; i++)
					_images[i].reset(new ImagePrivate(_d, swapchain_format, size, 1, 1, native_images[i]));
			}

			_hash = 0;
			for (auto i = 0; i < sizeof(_v); i += sizeof(uint))
				_hash = hash_update(_hash, ((uint*)&_v)[i]);
		}

		Format Swapchain::get_format()
		{
			return swapchain_format;
		}

		Swapchain *Swapchain::create(Device *d, Window* w, ImageUsageFlags extra_usages)
		{
			return new SwapchainPrivate(d, w, extra_usages);
		}
	}
}

