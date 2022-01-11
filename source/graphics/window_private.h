#pragma once

#include "window.h"
#include "buffer_private.h"
#include "extension.h"

namespace flame
{
	namespace graphics
	{
		struct WindowPrivate : Window
		{
			DevicePrivate* device;

			std::unique_ptr<CommandBufferPrivate> commandbuffer;
			std::unique_ptr<FencePrivate> finished_fence;
			std::unique_ptr<SemaphorePrivate> finished_semaphore;

			std::unique_ptr<ImagePrivate> imgui_img_font;
			StorageBuffer<FLAME_UID, BufferUsageVertex> imgui_buf_vtx;
			StorageBuffer<FLAME_UID, BufferUsageIndex> imgui_buf_idx;
			std::unique_ptr<DescriptorSetPrivate> imgui_ds;
			std::unique_ptr<GraphicsPipelinePrivate> imgui_pl;

			~WindowPrivate();

			void* imgui_context() override;
			void imgui_new_frame() override;

			void update() override;
		};

		extern std::vector<WindowPtr> windows;
	}
}
