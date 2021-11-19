#pragma once

#include "window.h"

namespace flame
{
	namespace graphics
	{
		struct WindowPrivate : Window
		{
			DevicePrivate* device;

			std::unique_ptr<CommandBufferPrivate> commandbuffer;
			std::unique_ptr<FencePrivate> submit_fence;
			std::unique_ptr<SemaphorePrivate> render_finished;

			std::list<std::function<void(uint, CommandBufferPtr)>> renders;

			~WindowPrivate();

			void update() override;

			void* add_renderer(const std::function<void(uint, CommandBufferPtr)>& callback) override;
			void remove_renderer(void* lis) override;
		};

		extern std::vector<WindowPtr> windows;
	}
}
