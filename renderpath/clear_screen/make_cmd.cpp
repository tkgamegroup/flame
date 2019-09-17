#include <flame/foundation/foundation.h>
#include <flame/graphics/all.h>

namespace flame
{
	struct MakeCmd$
	{
		AttributeP<std::vector<void*>> cbs$i;
		AttributeP<void> rp$i;
		AttributeP<std::vector<void*>> fbs$i;
		AttributeP<void> cv$i;

		int frame;

		__declspec(dllexport) MakeCmd$() :
			frame(-1)
		{
		}

		__declspec(dllexport) void update$()
		{
			if (cbs$i.frame > frame || rp$i.frame > frame || fbs$i.frame > frame || cv$i.frame > frame)
			{
				if (cbs$i.v && !cbs$i.v->empty() && rp$i.v && fbs$i.v && !fbs$i.v->empty() && cv$i.v)
				{
					for (auto i = 0; i < cbs$i.v->size(); i++)
					{
						auto cb = (graphics::Commandbuffer*)(*cbs$i.v)[i];
						cb->begin();
						cb->begin_renderpass((graphics::Renderpass*)rp$i.v, (graphics::Framebuffer*)(*fbs$i.v)[i], (graphics::Clearvalues*)cv$i.v);
						cb->end_renderpass();
						cb->end();
					}
				}
				else
				{
					for (auto i = 0; i < cbs$i.v->size(); i++)
					{
						auto cb = (graphics::Commandbuffer*)(*cbs$i.v)[i];
						cb->begin();
						cb->end();
					}
				}
				frame = maxN(cbs$i.frame, rp$i.frame, fbs$i.frame, cv$i.frame);
			}
		}
	};

	static MakeCmd$ bp_makecmd_unused;
}
