#include "device_private.h"
#include "swapchain_private.h"
#include "command_private.h"
#include "window_private.h"

namespace flame
{
	namespace graphics
	{
		static std::vector<WindowPrivate*> windows;

		WindowPrivate::WindowPrivate(DevicePrivate* _device, NativeWindow* native) :
			device(_device),
			native(native)
		{
			if (!device)
				device = default_device;

			swapchain.reset(new SwapchainPrivate(device, native));
			commandbuffer.reset(new CommandBufferPrivate(CommandPoolPrivate::get(device)));
			submit_fence.reset(new FencePrivate(device));
			render_finished.reset(new SemaphorePrivate(device));

			native->add_destroy_listener([](Capture& c) {
				delete c.thiz<WindowPrivate>();
			}, Capture().set_thiz(this));

			if (!windows.empty())
				windows.back()->next = this;
			windows.push_back(this);
		}

		WindowPrivate::~WindowPrivate()
		{
			for (auto it = windows.begin(); it != windows.end();)
			{
				if (*it == this)
				{
					if (it != windows.begin())
						(*(it - 1))->next = (*it)->next;
					windows.erase(it);
					return;
				}
				else
					it++;
			}
		}

		void* WindowPrivate::add_renderer(void (*render)(Capture& c, uint img_idx, CommandBuffer* commandbuffer), const Capture& capture)
		{
			auto c = new Closure(render, capture);
			renders.emplace_back(c);
			return c;
		}

		void WindowPrivate::remove_renderer(void* c)
		{
			std::erase_if(renders, [&](const auto& i) {
				return i == (decltype(i))c;
			});
		}

		void WindowPrivate::update()
		{
			if (!dirty)
				return;

			submit_fence->wait();

			auto img_idx = swapchain->acquire_image();

			commandbuffer->begin();
			for (auto& r : renders)
				r->call(img_idx, commandbuffer.get());
			commandbuffer->image_barrier(swapchain->images[img_idx].get(), {}, ImageLayoutAttachment, ImageLayoutPresent);
			commandbuffer->end();

			auto queue = graphics::Queue::get(nullptr);
			queue->submit(1, &commandbuffer, swapchain->get_image_avalible(), render_finished.get(), submit_fence.get());
			queue->present(swapchain.get(), render_finished.get());

			dirty = false;
		}

		Window* Window::create(Device* device, NativeWindow* native)
		{
			return new WindowPrivate((DevicePrivate*)device, native);
		}

		Window* get_first()
		{
			return windows.empty() ? nullptr : windows[0];
		}
	}
}
