#include "../foundation/window.h"
#include "device_private.h"
#include "image_private.h"
#include "swapchain_private.h"
#include "command_private.h"
#include "window_private.h"

namespace flame
{
	namespace graphics
	{
		WindowPrivate::~WindowPrivate()
		{
			Queue::get(nullptr)->wait_idle();
		}

		void* WindowPrivate::add_renderer(const std::function<void(uint, CommandBufferPtr)>& callback)
		{
			return &renders.emplace_back(callback);
		}

		void WindowPrivate::remove_renderer(void* lis)
		{
			std::erase_if(renders, [&](const auto& i) {
				return &i == lis;
			});
		}

		void WindowPrivate::update()
		{
			if (!dirty || swapchain->images.empty())
				return;

			submit_fence->wait();

			auto img_idx = swapchain->acquire_image();

			commandbuffer->begin();
			for (auto& r : renders)
				r(img_idx, commandbuffer.get());
			commandbuffer->image_barrier(swapchain->images[img_idx].get(), {}, ImageLayoutAttachment, ImageLayoutPresent);
			commandbuffer->end();

			auto queue = graphics::Queue::get(nullptr);
			queue->submit1(commandbuffer.get(), swapchain->image_avalible.get(), render_finished.get(), submit_fence.get());
			queue->present(swapchain.get(), render_finished.get());

			dirty = false;
		}

		std::vector<WindowPtr> windows;

		struct WindowCreate : Window::Create
		{
			WindowPtr operator()(DevicePtr device, NativeWindow* native) override
			{
				if (!device)
					device = current_device;

				auto ret = new WindowPrivate;
				ret->device = device;
				ret->native = native;

				ret->swapchain.reset(Swapchain::create(device, native));
				ret->commandbuffer.reset(CommandBuffer::create(CommandPool::get(device)));
				ret->submit_fence.reset(Fence::create(device));
				ret->render_finished.reset(Semaphore::create(device));

				native->add_destroy_listener([ret]() {
					for (auto it = windows.begin(); it != windows.end(); it++)
					{
						if (*it == ret)
						{
							windows.erase(it);
							delete ret;
							return;
						}
					}
				});

				windows.emplace_back(ret);
				return ret;
			}
		}Window_create;
		Window::Create& Window::create = Window_create;

		const std::vector<WindowPtr> get_windows()
		{
			return windows;
		}
	}
}
