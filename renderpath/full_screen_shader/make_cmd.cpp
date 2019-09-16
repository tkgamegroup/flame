#include <flame/foundation/foundation.h>
#include <flame/graphics/all.h>

namespace flame
{
	struct Ubo$
	{
		float time$;
	}unused1;

	struct PushconstantT$
	{
		Vec2f screen_size$;
	};

	struct MakeCmd$
	{
		AttributeP<std::vector<void*>> cmdbufs$i;
		AttributeP<void> renderpass$i;
		AttributeP<std::vector<void*>> framebuffers$i;
		AttributeP<void> pipeline$i;

		int frame;

		__declspec(dllexport) MakeCmd$() :
			frame(-1)
		{
		}

		__declspec(dllexport) void update$()
		{
			if (cmdbufs$i.frame > frame || renderpass$i.frame > frame || framebuffers$i.frame > frame || pipeline$i.frame > frame)
			{
				if (cmdbufs$i.v && !cmdbufs$i.v->empty() && renderpass$i.v && framebuffers$i.v && !framebuffers$i.v->empty() && pipeline$i.v)
				{
					for (auto i = 0; i < cmdbufs$i.v->size(); i++)
					{
						auto cb = (graphics::Commandbuffer*)(*cmdbufs$i.v)[i];
						cb->begin();
						auto fb = (graphics::Framebuffer*)(*framebuffers$i.v)[i];
						cb->begin_renderpass((graphics::Renderpass*)renderpass$i.v, fb, nullptr);
						auto size = Vec2f(fb->image_size);
						cb->set_viewport(Vec4f(Vec2f(0.f), size));
						cb->set_scissor(Vec4f(Vec2f(0.f), size));
						cb->bind_pipeline((graphics::Pipeline*)pipeline$i.v);
						PushconstantT$ pc;
						pc.screen_size$ = size;
						cb->push_constant(nullptr, 0, sizeof(PushconstantT$), &pc);
						cb->draw(3, 1, 0, 0);
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
				frame = maxN(cmdbufs$i.frame, renderpass$i.frame, framebuffers$i.frame, pipeline$i.frame);
			}
		}
	};

	static MakeCmd$ bp_makecmd_unused;
}
