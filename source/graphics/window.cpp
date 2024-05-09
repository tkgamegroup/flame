#include "../foundation/window.h"
#include "../foundation/system.h"
#include "device_private.h"
#include "image_private.h"
#include "command_private.h"
#include "shader_private.h"
#include "window_private.h"
#include "extension.h"

namespace flame
{
	namespace graphics
	{
		Format Swapchain::format = Format_B8G8R8A8_UNORM;

		SwapchainPrivate::~SwapchainPrivate()
		{
			if (app_exiting) return;

			if (window)
			{
				window->resize_listeners.remove("swapchain"_h);
				window->destroy_listeners.remove("swapchain"_h);
			}

			if (vk_swapchain)
			{
				vkDestroySwapchainKHR(device->vk_device, vk_swapchain, nullptr);
				unregister_object(vk_swapchain);
			}
			if (vk_surface)
				vkDestroySurfaceKHR(device->vk_instance, vk_surface, nullptr);
		}

		int SwapchainPrivate::acquire_image()
		{
			if (images.empty())
				return -1;
			chk_res(vkAcquireNextImageKHR(device->vk_device, vk_swapchain, UINT64_MAX, image_avalible->vk_semaphore, VK_NULL_HANDLE, &image_index));

			return image_index;
		}

		void SwapchainPrivate::build()
		{
			Queue::get()->wait_idle();

			images.clear();

			if (vk_swapchain)
			{
				vkDestroySwapchainKHR(device->vk_device, vk_swapchain, nullptr);
				unregister_object(vk_swapchain);
				vk_swapchain = nullptr;
			}
			if (vk_surface)
			{
				vkDestroySurfaceKHR(device->vk_instance, vk_surface, nullptr);
				vk_surface = nullptr;
			}

			auto size = window->size;
			if (size.x != 0U || size.y != 0U)
			{
				auto image_count = 3U;

				VkWin32SurfaceCreateInfoKHR surface_info = {};
				surface_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
				surface_info.hinstance = (HINSTANCE)get_hinst();
				surface_info.hwnd = (HWND)window->get_hwnd();
				chk_res(vkCreateWin32SurfaceKHR(device->vk_instance, &surface_info, nullptr, &vk_surface));

				VkBool32 surface_supported;
				vkGetPhysicalDeviceSurfaceSupportKHR(device->vk_physical_device, 0, vk_surface, &surface_supported);
				assert(surface_supported);

				auto present_mode_count = 0U;
				std::vector<VkPresentModeKHR> present_modes;
				vkGetPhysicalDeviceSurfacePresentModesKHR(device->vk_physical_device, vk_surface, &present_mode_count, nullptr);
				present_modes.resize(present_mode_count);
				vkGetPhysicalDeviceSurfacePresentModesKHR(device->vk_physical_device, vk_surface, &present_mode_count, present_modes.data());

				VkSurfaceCapabilitiesKHR surface_capabilities;
				vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device->vk_physical_device, vk_surface, &surface_capabilities);

				size.x = clamp(size.x, surface_capabilities.minImageExtent.width, surface_capabilities.maxImageExtent.width);
				size.y = clamp(size.y, surface_capabilities.minImageExtent.height, surface_capabilities.maxImageExtent.height);

				auto surface_format_count = 0U;
				std::vector<VkSurfaceFormatKHR> surface_formats;
				vkGetPhysicalDeviceSurfaceFormatsKHR(device->vk_physical_device, vk_surface, &surface_format_count, nullptr);
				assert(surface_format_count > 0);
				surface_formats.resize(surface_format_count);
				vkGetPhysicalDeviceSurfaceFormatsKHR(device->vk_physical_device, vk_surface, &surface_format_count, surface_formats.data());

				format = from_backend(surface_formats[0].format);

				VkSwapchainCreateInfoKHR swapchain_info = {};
				swapchain_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
				swapchain_info.surface = vk_surface;
				swapchain_info.minImageCount = image_count;
				swapchain_info.imageFormat = surface_formats[0].format;
				swapchain_info.imageColorSpace = surface_formats[0].colorSpace;
				swapchain_info.imageExtent.width = size.x;
				swapchain_info.imageExtent.height = size.y;
				swapchain_info.imageArrayLayers = 1;
				swapchain_info.imageUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
				swapchain_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
				swapchain_info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
				swapchain_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
				swapchain_info.presentMode = VK_PRESENT_MODE_FIFO_KHR;
				swapchain_info.clipped = true;
				chk_res(vkCreateSwapchainKHR(device->vk_device, &swapchain_info, nullptr, &vk_swapchain));
				register_object(vk_swapchain, "Swapchain", this);

				std::vector<VkImage> native_images;
				vkGetSwapchainImagesKHR(device->vk_device, vk_swapchain, &image_count, nullptr);
				native_images.resize(image_count);
				vkGetSwapchainImagesKHR(device->vk_device, vk_swapchain, &image_count, native_images.data());

				InstanceCommandBuffer cb;
				images.resize(image_count);
				for (auto i = 0; i < image_count; i++)
				{
					images[i].reset(ImagePrivate::create(device, format, uvec3(size, 1), native_images[i]));
					cb->image_barrier(images[i].get(), {}, ImageLayoutPresent);
					device->set_object_debug_name(images[i].get(), "Window" + str(i));
				}
				cb.excute();
			}
		}

		struct SwapchainCreate : Swapchain::Create
		{
			SwapchainPtr operator()(NativeWindow* window) override
			{
				auto ret = new SwapchainPrivate;
				ret->window = window;
				ret->build();
				ret->image_avalible.reset(Semaphore::create());

				window->resize_listeners.add([ret](const uvec2& size) {
					ret->build();
				}, "swapchain"_h);
				window->destroy_listeners.add([ret]() {
					ret->window = nullptr;
				}, "swapchain"_h);

				return ret;
			}
		}Swapchain_create;
		Swapchain::Create& Swapchain::create = Swapchain_create;

		WindowPrivate::WindowPrivate(NativeWindowPtr _native)
		{
			native = _native;

			swapchain.reset(Swapchain::create(native));

			native->destroy_listeners.add([this]() {
				for (auto it = windows.begin(); it != windows.end(); it++)
				{
					if (*it == this)
					{
						windows.erase(it);
						native = nullptr;
						delete this;
						return;
					}
				}
			}, "graphics_window"_h);
		}

		WindowPrivate::~WindowPrivate()
		{
			if (native)
				native->destroy_listeners.remove("graphics_window"_h);

			Queue::get()->wait_idle();
		}

		std::vector<WindowPtr> windows;

		struct WindowCreate : Window::Create
		{
			WindowPtr operator()(NativeWindowPtr native) override
			{
				auto ret = new WindowPrivate(native);
				windows.emplace_back(ret);
				return ret;
			}
		}Window_create;
		Window::Create& Window::create = Window_create;

		struct WindowGetList : Window::GetList
		{
			const std::vector<WindowPtr>& operator()() override
			{
				return windows;
			}
		}Window_get_list;
		Window::GetList& Window::get_list = Window_get_list;
	}
}
