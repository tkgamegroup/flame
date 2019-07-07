#include <flame/foundation/foundation.h>
#include <flame/graphics/all.h>

namespace flame
{
	struct MakeCmd$
	{
		AttributeP<std::vector<void*>> cmdbufs$i;
		AttributeP<void> renderpass$i;
		AttributeP<std::vector<void*>> framebuffers$i;
		AttributeP<void> clearvalues$i;

		int frame;

		__declspec(dllexport) MakeCmd$() :
			frame(-1)
		{
		}

		__declspec(dllexport) void update$()
		{
			if (cmdbufs$i.frame > frame || renderpass$i.frame > frame || framebuffers$i.frame > frame || clearvalues$i.frame > frame)
			{
				if (cmdbufs$i.v && !cmdbufs$i.v->empty() && renderpass$i.v && framebuffers$i.v && !framebuffers$i.v->empty() && clearvalues$i.v)
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
				else
				{
					for (auto i = 0; i < cmdbufs$i.v->size(); i++)
					{
						auto cb = (graphics::Commandbuffer*)(*cmdbufs$i.v)[i];
						cb->begin();
						cb->end();
					}
				}
				frame = maxN(cmdbufs$i.frame, renderpass$i.frame, framebuffers$i.frame, clearvalues$i.frame);
			}
		}
	};

	static MakeCmd$ bp_makecmd_unused;
}
