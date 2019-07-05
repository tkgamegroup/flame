#pragma comment(lib, "flame_foundation.lib")
#pragma comment(lib, "flame_graphics.lib")

#include <flame/foundation/foundation.h>
#include <flame/graphics/all.h>

namespace flame
{
	struct MakeCmd$
	{
		AttributeP<std::vector<void*>> cmdbufs$i;
		AttributeP<void> renderpass$i;
		AttributeP<void> clearvalues$i;
		AttributeP<std::vector<void*>> framebuffers$i;
		
		__declspec(dllexport) void update$()
		{
			if (cmdbufs$i.v && !cmdbufs$i.v->empty() && renderpass$i.v && clearvalues$i.v && framebuffers$i.v && !framebuffers$i.v->empty())
			{
				for (auto i = 0; i < cmdbufs$i.v->size(); i++)
				{
					auto cb = (graphics::Commandbuffer*)(*cmdbufs$i.v)[i];
					cb->begin();
					cb->begin_renderpass((graphics::Renderpass*)renderpass$i.v, (graphics::Framebuffer*)(*framebuffers$i.v)[i], (graphics::Clearvalues*)clearvalues$i.v);
					cb->end_renderpass();
					cb->end();
				}
			}
		}
	};

	static MakeCmd$ bp_makecmd_unused;
}
