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
			std::unique_ptr<CommandBufferPrivate> commandbuffer;
			std::unique_ptr<FencePrivate> finished_fence;
			std::unique_ptr<SemaphorePrivate> finished_semaphore;

			void* mouse_lis = nullptr;
			void* mousemove_lis = nullptr;
			void* scroll_lis = nullptr;
			void* key_lis = nullptr;
			void* char_lis = nullptr;
			void* resize_lis = nullptr;
			void* destroy_lis = nullptr;

			std::unique_ptr<ImagePrivate> imgui_img_font;
			StorageBuffer<FLAME_UID, BufferUsageVertex> imgui_buf_vtx;
			StorageBuffer<FLAME_UID, BufferUsageIndex> imgui_buf_idx;
			std::unique_ptr<DescriptorSetPrivate> imgui_ds;
			GraphicsPipelinePtr imgui_pl;

			WindowPrivate(NativeWindowPtr native);
			~WindowPrivate();

			void* imgui_context() override;
			void imgui_new_frame() override;

			void update() override;
		};

		extern std::vector<WindowPtr> windows;
	}
}
