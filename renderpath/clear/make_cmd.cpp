#include <flame/foundation/blueprint.h>
#include <flame/graphics/commandbuffer.h>
#include <flame/graphics/renderpass.h>

namespace flame
{
	struct MakeCmd$
	{
		AttributeP<Array<graphics::Commandbuffer*>> cbs$i;
		AttributeP<Array<graphics::Framebuffer*>> fbs$i;
		AttributeP<graphics::Clearvalues> cv$i;

		int frame;

		__declspec(dllexport) MakeCmd$() :
			frame(-1)
		{
		}

		__declspec(dllexport) void update$(BP* scene)
		{
			if (cbs$i.frame > frame || fbs$i.frame > frame || cv$i.frame > frame)
			{
				if (cbs$i.v && cbs$i.v->s && fbs$i.v && fbs$i.v->s && cv$i.v)
				{
					for (auto i = 0; i < cbs$i.v->s; i++)
					{
						auto cb = cbs$i.v->v[i];
						cb->begin();
						cb->begin_renderpass(fbs$i.v->v[i], cv$i.v);
						cb->end_renderpass();
						cb->end();
					}
				}
				else
				{
					for (auto i = 0; i < cbs$i.v->s; i++)
					{
						auto cb = cbs$i.v->v[i];
						cb->begin();
						cb->end();
					}
				}
				frame = maxN(cbs$i.frame, fbs$i.frame, cv$i.frame);
			}
		}
	};
}
