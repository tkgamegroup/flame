#include <flame/foundation/blueprint.h>
#include <flame/graphics/commandbuffer.h>
#include <flame/graphics/renderpass.h>

using namespace flame;
using namespace graphics;

struct FLAME_R(MakeCmd)
{
	BP::Node* n;

	FLAME_B0;
	FLAME_RV(Array<Commandbuffer*>*, cbs, i);
	FLAME_RV(RenderpassAndFramebuffer*, rnf, i);
	FLAME_RV(uint, image_idx, i);

	__declspec(dllexport) void FLAME_RF(update)(uint _frame)
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
