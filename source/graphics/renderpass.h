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
			std::vector<uvec2> dependencies;
		};

		struct Renderpass : RenderpassInfo
		{
			std::filesystem::path filename;
			std::vector<std::string> defines;

			virtual ~Renderpass() {}

			struct Create
			{
				virtual RenderpassPtr operator()(const RenderpassInfo& info) = 0;
				virtual RenderpassPtr operator()(const std::string& content, const std::vector<std::string>& defines, const std::string& filename = "" /* as key */) = 0;
			};
			FLAME_GRAPHICS_API static Create& create;

			struct Get
			{
				virtual RenderpassPtr operator()(const std::filesystem::path& filename, const std::vector<std::string>& defines) = 0;
			};
			FLAME_GRAPHICS_API static Get& get;
		};

		struct Framebuffer
		{
			RenderpassPtr renderpass;
			std::vector<ImageViewPtr> views;

			virtual ~Framebuffer() {}

			struct Create
			{
				virtual FramebufferPtr operator()(RenderpassPtr renderpass, std::span<ImageViewPtr> views) = 0;
				inline FramebufferPtr operator()(RenderpassPtr renderpass, ImageViewPtr view)
				{
					return (*this)(renderpass, { &view, 1 });
				}
				inline FramebufferPtr operator()(RenderpassPtr renderpass, std::initializer_list<ImageViewPtr> views)
				{
					return (*this)(renderpass, std::span<ImageViewPtr>((ImageViewPtr*)views.begin(), views.size()));
				}
			};
			FLAME_GRAPHICS_API static Create& create;
		};
	}
}

