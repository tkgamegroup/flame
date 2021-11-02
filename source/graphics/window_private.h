#pragma once

#include "window.h"

namespace flame
{
	namespace graphics
	{
		struct WindowPrivate : Window
		{
			DevicePrivate* device;

			NativeWindow* native;
			UniPtr<SwapchainPrivate> swapchain;
			UniPtr<CommandBufferPrivate> commandbuffer;
			UniPtr<FencePrivate> submit_fence;
			UniPtr<SemaphorePrivate> render_finished;

			std::vector<std::unique_ptr<Closure<void(Capture&, uint img_idx, CommandBuffer*)>>> renders;
			bool dirty = false;

			WindowPrivate* next = nullptr;

			WindowPrivate(DevicePrivate* device, NativeWindow* native);
			~WindowPrivate();

			void release() override { delete this; }

			NativeWindow* get_native() const override { return native; }
			SwapchainPtr get_swapchain() const override { return swapchain.get(); }

			void update() override;

			Window* get_next() const override { return next; }

			void* add_renderer(void (*render)(Capture& c, uint img_idx, CommandBuffer* commandbuffer), const Capture& capture) override;
			void remove_renderer(void* c) override;

			void mark_dirty() override { dirty = true; }
		};
	}
}
