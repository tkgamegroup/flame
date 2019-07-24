#pragma once

#include <flame/graphics/shader.h>

namespace flame
{
	namespace graphics
	{
		struct Framebuffer;
		struct Image;
		struct Commandbuffer;

		enum RenderpathPassTargetType
		{
			RenderpathPassTargetImage, // v is Image*
			RenderpathPassTargetImageview, // v is Imageview*
			RenderpathPassTargetImages // v is std::vector<Image*>*
		};

		struct RenderpathPassTarget
		{
			RenderpathPassTargetType type;
			void* v;
			bool clear;
			Vec4c clear_color;

			RenderpathPassTarget()
			{
			}

			RenderpathPassTarget(Image* v, bool clear = false, const Vec4c& clear_color = Vec4c(0)) :
				type(RenderpathPassTargetImage),
				v(v),
				clear(clear),
				clear_color(clear_color)
			{
			}

			RenderpathPassTarget(Imageview* v, bool clear = false, const Vec4c& clear_color = Vec4c(0)) :
				type(RenderpathPassTargetImageview),
				v(v),
				clear(clear),
				clear_color(clear_color)
			{
			}

			RenderpathPassTarget(std::vector<Image*>* v, bool clear = false, const Vec4c& clear_color = Vec4c(0)) :
				type(RenderpathPassTargetImages),
				v(v),
				clear(clear),
				clear_color(clear_color)
			{
			}
		};

		struct RenderpathShaderInfo
		{
			std::wstring filename;
			std::string prefix;
			std::vector<void*> stages;
		};

		struct RenderpathPushconstantInfo
		{
			uint udt_name_hash;
		};

		struct RenderpathPipelineInfo
		{
			std::vector<void*> shaders;
		};

		struct RenderpathPassInfo
		{
			std::vector<void*> color_targets;
			std::vector<void*> resolve_targets;
			void* depth_target;

			RenderpathPassInfo()
			{
				depth_target = nullptr;
			}
		};

		struct RenderpathInfo
		{
			std::vector<RenderpathPassInfo> passes;
		};

		struct Renderpath
		{
			FLAME_GRAPHICS_EXPORTS uint image_count() const;
			FLAME_GRAPHICS_EXPORTS Renderpass* renderpass() const;
			FLAME_GRAPHICS_EXPORTS Framebuffer* framebuffer(uint idx) const;
			FLAME_GRAPHICS_EXPORTS Commandbuffer* commandbuffer(uint idx) const;

			FLAME_GRAPHICS_EXPORTS static Renderpath* create(Device* d, const RenderpathInfo& info);
			FLAME_GRAPHICS_EXPORTS static void destroy(Renderpath* s);
		};
	}
}
