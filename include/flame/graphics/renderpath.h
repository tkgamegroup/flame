#pragma once

#include <flame/graphics/graphics.h>

#include <vector>

namespace flame
{
	namespace graphics
	{
		struct Renderpass;
		struct Framebuffer;
		struct Image;
		struct Imageview;

		enum RenderpathPassTargetType
		{
			RenderpathPassTargetImage, // v is Image*
			RenderpathPassTargetImageview, // v is Imageview*
			RenderpathPassTargetSwapchainImages // v is std::vector<Image*>*
		};

		struct RenderpathPassTarget
		{
			RenderpathPassTargetType type;
			void* v;
		};

		struct RenderpathShaderInfo
		{
			std::wstring filename;
			std::string prefix;
		};

		struct RenderpathPipelineInfo
		{
			std::vector<void*> shaders;
		};

		struct RenderpathPassInfo
		{
			std::vector<RenderpathPassTarget> targets;
		};

		struct RenderpathInfo
		{

		};

		struct Renderpath
		{
			FLAME_GRAPHICS_EXPORTS Renderpass* renderpass() const;
			FLAME_GRAPHICS_EXPORTS Framebuffer* framebuffer(uint idx) const;
		};
	}
}
