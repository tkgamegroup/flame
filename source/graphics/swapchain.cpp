#include "../foundation/window.h"
#include "../foundation/system.h"
#include "device_private.h"
#include "image_private.h"
#include "renderpass_private.h"
#include "command_private.h"
#include "swapchain_private.h"
#include "command_ext.h"

namespace flame
{
	namespace graphics
	{
		Format Swapchain::format = Format_B8G8R8A8_UNORM;

		SwapchainPrivate::~SwapchainPrivate()
		{
			if (window)
				window->remove_resize_listener(resize_listener);

			if (vk_swapchain)
				vkDestroySwapchainKHR(device->vk_device, vk_swapchain, nullptr);
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
			device->gq.get()->wait_idle();

			images.clear();

			if (vk_swapchain)
			{
				vkDestroySwapchainKHR(device->vk_device, vk_swapchain, nullptr);
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
				uint image_count = 3;

				VkWin32SurfaceCreateInfoKHR surface_info;
				surface_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
				surface_info.flags = 0;
				surface_info.pNext = nullptr;
				surface_info.hinstance = (HINSTANCE)get_hinst();
				surface_info.hwnd = (HWND)window->get_hwnd();
				chk_res(vkCreateWin32SurfaceKHR(device->vk_instance, &surface_info, nullptr, &vk_surface));

				VkBool32 surface_supported;
				vkGetPhysicalDeviceSurfaceSupportKHR(device->vk_physical_device, 0, vk_surface, &surface_supported);
				assert(surface_supported);

				unsigned int present_mode_count = 0;
				std::vector<VkPresentModeKHR> present_modes;
				vkGetPhysicalDeviceSurfacePresentModesKHR(device->vk_physical_device, vk_surface, &present_mode_count, nullptr);
				present_modes.resize(present_mode_count);
				vkGetPhysicalDeviceSurfacePresentModesKHR(device->vk_physical_device, vk_surface, &present_mode_count, present_modes.data());

				VkSurfaceCapabilitiesKHR surface_capabilities;
				vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device->vk_physical_device, vk_surface, &surface_capabilities);

				size.x = clamp(size.x,
					surface_capabilities.minImageExtent.width,
					surface_capabilities.maxImageExtent.width);
				size.y = clamp(size.y,
					surface_capabilities.minImageExtent.height,
					surface_capabilities.maxImageExtent.height);

				unsigned int surface_format_count = 0;
				std::vector<VkSurfaceFormatKHR> surface_formats;
				vkGetPhysicalDeviceSurfaceFormatsKHR(device->vk_physical_device, vk_surface, &surface_format_count, nullptr);
				assert(surface_format_count > 0);
				surface_formats.resize(surface_format_count);
				vkGetPhysicalDeviceSurfaceFormatsKHR(device->vk_physical_device, vk_surface, &surface_format_count, surface_formats.data());

				format = graphics::get_format(surface_formats[0].format);

				VkSwapchainCreateInfoKHR swapchain_info;
				swapchain_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
				swapchain_info.flags = 0;
				swapchain_info.pNext = nullptr;
				swapchain_info.surface = vk_surface;
				swapchain_info.minImageCount = image_count;
				swapchain_info.imageFormat = to_backend(format);
				swapchain_info.imageColorSpace = surface_formats[0].colorSpace;
				swapchain_info.imageExtent.width = size.x;
				swapchain_info.imageExtent.height = size.y;
				swapchain_info.imageArrayLayers = 1;
				swapchain_info.imageUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
				swapchain_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
				swapchain_info.queueFamilyIndexCount = 0;
				swapchain_info.pQueueFamilyIndices = nullptr;
				swapchain_info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
				swapchain_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
				swapchain_info.presentMode = VK_PRESENT_MODE_FIFO_KHR;
				swapchain_info.clipped = true;
				swapchain_info.oldSwapchain = 0;
				chk_res(vkCreateSwapchainKHR(device->vk_device, &swapchain_info, nullptr, &vk_swapchain));

				std::vector<VkImage> native_images;
				vkGetSwapchainImagesKHR(device->vk_device, vk_swapchain, &image_count, nullptr);
				native_images.resize(image_count);
				vkGetSwapchainImagesKHR(device->vk_device, vk_swapchain, &image_count, native_images.data());

				InstanceCB cb(device);
				images.resize(image_count);
				for (auto i = 0; i < image_count; i++)
				{
					images[i].reset(ImagePrivate::create(device, format, size, native_images[i]));
					cb->image_barrier(images[i].get(), {}, ImageLayoutUndefined, ImageLayoutPresent);
				}
			}
		}

		SwapchainPtr Swapchain::create(DevicePtr device, NativeWindow* window)
		{
			if (!device)
				device = current_device;

			auto ret = new SwapchainPrivate;
			ret->device = device;
			ret->window = window;
			ret->build();
			ret->image_avalible.reset(Semaphore::create(device));

			ret->resize_listener = window->add_resize_listener([ret](const uvec2& size) {
				ret->build();
			});
			window->add_destroy_listener([ret]() {
				ret->window = nullptr;
			});

			return ret;
		}
	}
}
