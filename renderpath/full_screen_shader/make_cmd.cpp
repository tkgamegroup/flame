#include <flame/foundation/foundation.h>
#include <flame/graphics/all.h>

namespace flame
{
	struct Ubo$
	{
		float time$;
	};

	struct PushconstantT$
	{
		Vec2f screen_size$;
	};

	struct MakeCmd$
	{
		AttributeP<std::vector<void*>> cbs$i;
		AttributeP<void> rp$i;
		AttributeP<std::vector<void*>> fbs$i;
		AttributeP<void> pl$i;
		AttributeP<void> ds$i;

		AttributeP<void> ubo$i;

		int frame;

		__declspec(dllexport) MakeCmd$() :
			frame(-1)
		{
		}

		__declspec(dllexport) void update$()
		{
			if (cbs$i.frame > frame || rp$i.frame > frame || fbs$i.frame > frame || pl$i.frame > frame)
			{
				if (cbs$i.v && !cbs$i.v->empty() && rp$i.v && fbs$i.v && !fbs$i.v->empty() && pl$i.v)
				{
					for (auto i = 0; i < cbs$i.v->size(); i++)
					{
						auto cb = (graphics::Commandbuffer*)(*cbs$i.v)[i];
						cb->begin();
						auto fb = (graphics::Framebuffer*)(*fbs$i.v)[i];
						cb->begin_renderpass(fb, nullptr);
						auto size = Vec2f(fb->image_size);
						cb->set_viewport(Vec4f(Vec2f(0.f), size));
						cb->set_scissor(Vec4f(Vec2f(0.f), size));
						cb->bind_pipeline((graphics::Pipeline*)pl$i.v);
						cb->bind_descriptorset((graphics::Descriptorset*)ds$i.v, 0);
						//PushconstantT$ pc;
						//pc.screen_size$ = size;
						//cb->push_constant(0, sizeof(PushconstantT$), &pc);
						cb->draw(3, 1, 0, 0);
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
				frame = maxN(cbs$i.frame, rp$i.frame, fbs$i.frame, pl$i.frame);
			}

			auto ubo = (graphics::Buffer*)ubo$i.v;
			ubo->map();
			Ubo$ data;
			data.time$ = looper().frame / 60.f;
			memcpy(ubo->mapped, &data, sizeof(Ubo$));
		}
	};
}
