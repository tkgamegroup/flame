#include <flame/foundation/blueprint.h>
#include <flame/graphics/commandbuffer.h>
#include <flame/graphics/renderpass.h>

#include <flame/reflect_macros.h>

using namespace flame;
using namespace graphics;

struct R(MakeCmd)
{
	BP::Node* n;

	BASE0;
	RV(Array<Commandbuffer*>*, cbs, i);
	RV(RenderpassAndFramebuffer*, rnf, i);
	RV(uint, image_idx, i);

	__declspec(dllexport) void RF(update)(uint _frame)
	{
		if (cbs && rnf)
		{
			auto cb = cbs->at(image_idx);
			cb->begin();
			cb->begin_renderpass(rnf->framebuffer(image_idx), rnf->clearvalues());
			cb->end_renderpass();
			cb->end();
		}
	}
};
