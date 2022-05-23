#include "../foundation/window.h"
#include "device_private.h"
#include "image_private.h"
#include "swapchain_private.h"
#include "command_private.h"
#include "window_private.h"
#include "extension.h"

namespace flame
{
	namespace graphics
	{
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
