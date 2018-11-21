#pragma once

#include <memory>

#include <flame/engine/core/object.h>

namespace flame
{
	struct Texture;
	struct RenderPass;
	struct Framebuffer;

	struct DisplayLayer : Object
	{
		bool enable_depth;
		std::shared_ptr<Texture> image;
		std::unique_ptr<Texture> depth_image;
		std::shared_ptr<RenderPass> renderpass;
		std::shared_ptr<Framebuffer> framebuffer;

		virtual bool on_message(Object *sender, Message msg) override;

		DisplayLayer(bool _enable_depth = false);
		~DisplayLayer();

	private:
		void create();
	};
}
