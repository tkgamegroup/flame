#include <flame/graphics/renderpath.h>
#include "renderpass_private.h"

namespace flame
{
	namespace graphics
	{
		struct RenderpathPrivate : Renderpath
		{
			RenderpassPrivate* rp;
			std::vector<FramebufferPrivate*> fbs;
		};
	}
}
