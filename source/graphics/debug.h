#pragma once

#include "graphics.h"

namespace flame
{
	namespace graphics
	{
		struct Debug
		{
			struct GetBuffers
			{
				virtual std::vector<BufferPtr> operator()() = 0;
			};
			FLAME_GRAPHICS_API static GetBuffers& get_buffers;

			struct GetImages
			{
				virtual std::vector<ImagePtr> operator()() = 0;
			};
			FLAME_GRAPHICS_API static GetImages& get_images;

			struct GetMaterials
			{
				virtual std::vector<MaterialPtr> operator()() = 0;
			};
			FLAME_GRAPHICS_API static GetMaterials& get_materials;

			struct GetShaders
			{
				virtual std::vector<ShaderPtr> operator()() = 0;
			};
			FLAME_GRAPHICS_API static GetShaders& get_shaders;

			struct GetGraphicsPipelines
			{
				virtual std::vector<GraphicsPipelinePtr> operator()() = 0;
			};
			FLAME_GRAPHICS_API static GetGraphicsPipelines& get_graphics_pipelines;

			FLAME_GRAPHICS_API static void start_capture_frame();
			FLAME_GRAPHICS_API static void end_capture_frame();
		};
	}
}
