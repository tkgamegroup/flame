#pragma once

#include "window.h"
#include "buffer_private.h"
#include "buffer_ext.h"

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

			std::unique_ptr<RenderpassPrivate> renderpass_clear;
			std::unique_ptr<RenderpassPrivate> renderpass_load;
			std::vector<std::unique_ptr<FramebufferPrivate>> framebuffers;

			std::unique_ptr<ImagePrivate> imgui_img_font;
			StorageBuffer<BufferUsageVertex> imgui_buf_vtx;
			StorageBuffer<BufferUsageIndex> imgui_buf_idx;
			std::unique_ptr<DescriptorSetPrivate> imgui_ds;
			std::unique_ptr<GraphicsPipelinePrivate> imgui_pl;
			
			std::list<std::function<void(void* ctx)>> imgui_callbacks;

			std::list<std::function<void(uint, CommandBufferPtr)>> renders;

			~WindowPrivate();

			void* add_imgui_callback(const std::function<void(void* ctx)>& callback) override;
			void remove_imgui_callback(void* lis) override;

			void* add_renderer(const std::function<void(uint, CommandBufferPtr)>& callback) override;
			void remove_renderer(void* lis) override;

			void imgui_new_frame() override;

			void update() override;
		};

		extern std::vector<WindowPtr> windows;
	}
}
