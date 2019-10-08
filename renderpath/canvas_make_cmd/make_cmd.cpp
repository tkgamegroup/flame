#include <flame/foundation/foundation.h>
#include <flame/graphics/all.h>

#include "../canvas/type.h"

using namespace flame;
using namespace graphics;

namespace flame
{
	struct MakeCmd$
	{
		AttributeP<std::vector<void*>> cbs$i;
		AttributeP<void> rnf$i;
		AttributeV<uint> image_idx$i;

		__declspec(dllexport) void update$()
		{
			auto rnf = (RenderpassAndFramebuffer*)rnf$i.v;
			auto img_idx = image_idx$i.v;
			auto cb = (Commandbuffer*)(*cbs$i.v)[img_idx];
			auto fb = (Framebuffer*)rnf->framebuffers()[img_idx];

			cb->begin();
			cb->begin_renderpass(fb, rnf->clearvalues());
			cb->end_renderpass();
			cb->end();
		}
	};
}
