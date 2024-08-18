#pragma once

#include "../foundation/application.h"
#include "device.h"
#include "image.h"
#include "renderpass.h"
#include "command.h"
#include "window.h"
#include "gui.h"

struct GraphicsApplicationOptions
{
	bool use_gui = false;
	bool graphics_debug = false;
	std::vector<std::pair<uint, uint>> graphics_configs;
};

struct GraphicsApplication : Application
{
	graphics::WindowPtr main_window = nullptr;
	int render_frames = 0;
	bool always_render = true;
	bool use_gui = false;

	std::unique_ptr<graphics::CommandBufferT> command_buffer;
	std::unique_ptr<graphics::FenceT> render_fence;
	std::unique_ptr<graphics::SemaphoreT> render_semaphore;

	void create(std::string_view title,
		const uvec2& size = uvec2(1280, 720),
		WindowStyleFlags styles = WindowStyleFrame | WindowStyleResizable,
		const GraphicsApplicationOptions& options = {})
	{
		Application::create(title, size, styles);

		auto graphics_device = graphics::Device::create(options.graphics_debug, options.graphics_configs);
		main_window = graphics::Window::create(Application::main_window);

		use_gui = options.use_gui;
		if (use_gui)
		{
			graphics::gui_initialize();
			graphics::gui_set_current();
			graphics::gui_callbacks.add([this]() {
				on_gui();
				}, "app"_h);
		}

		command_buffer.reset(graphics::CommandBuffer::create(graphics::CommandPool::get()));
		command_buffer->want_executed_time = true;
		render_fence.reset(graphics::Fence::create(graphics_device));
		render_semaphore.reset(graphics::Semaphore::create());
	}

	virtual void on_render()
	{
		if (use_gui)
		{
			graphics::gui_frame();
			graphics::gui_render(command_buffer.get());
		}
	}

	bool on_update() override
	{
		auto all_minimized = true;
		auto has_input = false;

		auto windows = graphics::Window::get_list();

		for (auto w : windows)
		{
			if (!w->swapchain->images.empty())
				all_minimized = false;
			if (w->native->has_input)
				has_input = true;
		}

		if (all_minimized)
			return true;
		if (has_input)
			render_frames = 3;
		if (always_render)
			render_frames = max(render_frames, 1);
		if (render_frames <= 0)
			return true;
		render_frames--;

		render_fence->wait();

		command_buffer->calc_executed_time();
		//printf("%lfms\n", (double)command_buffer->last_executed_time / (double)1000000);

		command_buffer->begin();

		for (auto w : windows)
		{
			if (w->swapchain->images.empty())
				continue;
			w->swapchain->acquire_image();
		}

		on_render();

		for (auto w : windows)
		{
			if (w->swapchain->images.empty())
				continue;
			command_buffer->image_barrier(w->swapchain->current_image(), {}, graphics::ImageLayoutPresent);
		}

		command_buffer->end();

		auto queue = (graphics::Queue*)graphics::Queue::get();
		{
			std::vector<graphics::SemaphorePtr> wait_semaphores;
			std::vector<graphics::SwapchainPtr> swapchains;
			for (auto w : windows)
			{
				if (w->swapchain->images.empty())
					continue;
				wait_semaphores.push_back(w->swapchain->image_avalible.get());
				swapchains.push_back(w->swapchain.get());
			}

			queue->submit_and_present(command_buffer.get(), wait_semaphores, render_semaphore.get(), render_fence.get(), swapchains);
		}

		return true;
	}

	virtual void on_gui()
	{
	}
};
