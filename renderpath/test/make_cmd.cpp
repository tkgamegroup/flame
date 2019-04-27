#include <flame/graphics/all.h>

namespace flame
{
	struct BP_MakeCmd$
	{
		VoidPtrs cmdbufs$i;
		void* renderpass$i;
		void* clearvalues$i;
		VoidPtrs framebuffers$i;
		
		__declspec(dllexport) void initialize$c()
		{
		}
		
		__declspec(dllexport) void finish$c()
		{
		}
		
		__declspec(dllexport) void update$c()
		{
			for (auto i = 0; i < cmdbufs$i.count; i++)
			{
				auto cb = (graphics::Commandbuffer*)cmdbufs$i.v[i];
				cb->begin();
				cb->begin_renderpass((graphics::Renderpass*)renderpass$i, (graphics::Framebuffer*)framebuffers$i.v[i], (graphics::Clearvalues*)clearvalues$i);
				cb->end_renderpass();
				cb->end();
			}
		}
	};

	static BP_MakeCmd$ bp_makecmd_unused;
}
