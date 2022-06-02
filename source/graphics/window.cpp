#include "../foundation/window.h"
#include "../foundation/system.h"
#include "device_private.h"
#include "image_private.h"
#include "command_private.h"
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
				window->resize_listeners.remove(resize_lis);
				window->destroy_listeners.remove(destroy_lis);
			}

			if (vk_swapchain)
			{
				vkDestroySwapchainKHR(device->vk_device, vk_swapchain, nullptr);
				unregister_backend_object(vk_swapchain);
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
				unregister_backend_object(vk_swapchain);
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
				register_backend_object(vk_swapchain, tn<decltype(*this)>(), this);

				std::vector<VkImage> native_images;
				vkGetSwapchainImagesKHR(device->vk_device, vk_swapchain, &image_count, nullptr);
				native_images.resize(image_count);
				vkGetSwapchainImagesKHR(device->vk_device, vk_swapchain, &image_count, native_images.data());

				InstanceCB cb;
				images.resize(image_count);
				for (auto i = 0; i < image_count; i++)
				{
					images[i].reset(ImagePrivate::create(device, format, size, native_images[i]));
					cb->image_barrier(images[i].get(), {}, ImageLayoutPresent);
				}
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

				ret->resize_lis = window->resize_listeners.add([ret](const uvec2& size) {
					ret->build();
					});
				ret->destroy_lis = window->destroy_listeners.add([ret]() {
					ret->window = nullptr;
					});

				return ret;
			}
		}Swapchain_create;
		Swapchain::Create& Swapchain::create = Swapchain_create;

		WindowPrivate::WindowPrivate(NativeWindowPtr _native)
		{
			native = _native;

			swapchain.reset(Swapchain::create(native));
			commandbuffer.reset(CommandBuffer::create(CommandPool::get()));
			commandbuffer->want_executed_time = true;
			finished_fence.reset(Fence::create(device));
			finished_semaphore.reset(Semaphore::create());

			destroy_lis = native->destroy_listeners.add([this]() {
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
			});
		}

		WindowPrivate::~WindowPrivate()
		{
			if (native)
			{
				native->mouse_listeners.remove(mouse_lis);
				native->mousemove_listeners.remove(mousemove_lis);
				native->scroll_listeners.remove(scroll_lis);
				native->key_listeners.remove(key_lis);
				native->char_listeners.remove(char_lis);
				native->resize_listeners.remove(resize_lis);
				native->destroy_listeners.remove(destroy_lis);
			}

			Queue::get()->wait_idle();
		}

		void WindowPrivate::render()
		{
			if (!dirty || swapchain->images.empty())
				return;

			finished_fence->wait();
			commandbuffer->calc_executed_time();
			//printf("%lfms\n", (double)commandbuffer->last_executed_time / (double)1000000);

			auto img_idx = swapchain->acquire_image();
			auto curr_img = swapchain->images[img_idx].get();

			commandbuffer->begin();

			for (auto& l : renderers.list)
				l.first(img_idx, commandbuffer.get());

			commandbuffer->image_barrier(curr_img, {}, ImageLayoutPresent);
			commandbuffer->end();

			auto queue = graphics::Queue::get();
			queue->submit1(commandbuffer.get(), swapchain->image_avalible.get(), finished_semaphore.get(), finished_fence.get());
			queue->present(swapchain.get(), finished_semaphore.get());

			dirty = false;
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
