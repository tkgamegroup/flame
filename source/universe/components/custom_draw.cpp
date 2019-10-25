#include "../universe_private.h"
#include <flame/universe/components/custom_draw.h>

namespace flame
{
	struct cCustomDrawPrivate : cCustomDraw
	{
		cCustomDrawPrivate::cCustomDrawPrivate()
		{
			cmds.hub = new ListenerHub;
		}

		cCustomDrawPrivate::~cCustomDrawPrivate()
		{
			delete (ListenerHub*)cmds.hub;
		}
	};

	cCustomDraw* cCustomDraw::create()
	{
		return new cCustomDrawPrivate();
	}
}
