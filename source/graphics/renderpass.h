#pragma once

#include "graphics.h"

namespace flame
{
	namespace graphics
	{
		struct AttachmentInfo
		{
			Format format = Format_R8G8B8A8_UNORM;
			AttachmentLoadOp load_op = AttachmentLoadClear;
			AttachmentStoreOp store_op = AttachmentStoreStore;
			SampleCount sample_count = SampleCount_1;
			ImageLayout initia_layout = ImageLayoutUndefined;
			ImageLayout final_layout = ImageLayoutAttachment;
		};

		struct SubpassInfo
		{
			std::vector<int> color_attachments;
			std::vector<int> resolve_attachments;
			int depth_attachment = -1;
		};

		struct RenderpassInfo
		{
			std::vector<AttachmentInfo> attachments;
			std::vector<SubpassInfo> subpasses;
		};

		struct Renderpass
		{
			RenderpassInfo info;

			virtual ~Renderpass() {}

			struct Create
			{
				virtual RenderpassPtr operator()(DevicePtr device, const RenderpassInfo& info, std::span<uvec2> dependencies = {}) = 0;
				virtual RenderpassPtr operator()(DevicePtr device, const std::string& content, const std::vector<std::string>& defines, const std::string& filename = "" /* as key */) = 0;
			};
			FLAME_GRAPHICS_EXPORTS static Create& create;

			struct Get
			{
				virtual RenderpassPtr operator()(DevicePtr device, const std::filesystem::path& filename, const std::vector<std::string>& defines) = 0;
			};
			FLAME_GRAPHICS_EXPORTS static Get& get;
		};

		struct Framebuffer
		{
			RenderpassPtr renderpass;
			std::vector<ImageViewPtr> views;

			virtual ~Framebuffer() {}

			struct Create
			{
				virtual FramebufferPtr operator()(RenderpassPtr renderpass, std::span<ImageViewPtr> views) = 0;
			};
			FLAME_GRAPHICS_EXPORTS static Create& create;
		};
	}
}

