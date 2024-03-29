#include "debug.h"
#include "material_private.h"
#include "buffer_private.h"
#include "image_private.h"
#include "shader_private.h"

#ifdef HAS_RENDERDOC
#include RENDERDOC_HEADER
RENDERDOC_API_1_6_0* rdoc_api = nullptr;
void get_rdoc_api()
{
	if (auto mod = GetModuleHandleA("renderdoc.dll"))
	{
		auto get_api = (pRENDERDOC_GetAPI)GetProcAddress(mod, "RENDERDOC_GetAPI");
		assert(get_api(eRENDERDOC_API_Version_1_6_0, (void**)&rdoc_api) == 1);
	}
}
#endif

namespace flame
{
	namespace graphics
	{
		struct DebugGetBuffers : Debug::GetBuffers
		{
			std::vector<BufferPtr> operator()() override
			{
				return buffers;
			}
		}Debug_get_buffers;
		Debug::GetBuffers& Debug::get_buffers = Debug_get_buffers;

		struct DebugGetImages : Debug::GetImages
		{
			std::vector<ImagePtr> operator()() override
			{
				return images;
			}
		}Debug_get_images;
		Debug::GetImages& Debug::get_images = Debug_get_images;

		struct DebugGetMaterials : Debug::GetMaterials
		{
			std::vector<MaterialPtr> operator()() override
			{
				return materials;
			}
		}Debug_get_materials;
		Debug::GetMaterials& Debug::get_materials = Debug_get_materials;

		struct DebugGetShaders : Debug::GetShaders
		{
			std::vector<ShaderPtr> operator()() override
			{
				return shaders;
			}
		}Debug_get_shaders;
		Debug::GetShaders& Debug::get_shaders = Debug_get_shaders;

		struct DebugGetGraphicsPipelines : Debug::GetGraphicsPipelines
		{
			std::vector<GraphicsPipelinePtr> operator()() override
			{
				return graphics_pipelines;
			}
		}Debug_get_graphics_pipelines;
		Debug::GetGraphicsPipelines& Debug::get_graphics_pipelines = Debug_get_graphics_pipelines;

		void Debug::start_capture_frame()
		{
#ifdef HAS_RENDERDOC
			if (!rdoc_api)
				get_rdoc_api();
			if (rdoc_api)
				rdoc_api->StartFrameCapture(nullptr, nullptr);
#endif
		}

		void Debug::end_capture_frame()
		{
#ifdef HAS_RENDERDOC
			if (!rdoc_api)
				get_rdoc_api();
			if (rdoc_api)
				rdoc_api->EndFrameCapture(nullptr, nullptr);
#endif
		}
	}
}
