#include <flame/foundation/foundation.h>
#include <flame/graphics/all.h>

#include "../canvas/type.h"

namespace flame
{
	struct MakeCmd$
	{
		AttributeP<void> rnf$i;

		int frame;

		__declspec(dllexport) MakeCmd$() :
			frame(-1)
		{
		}

		__declspec(dllexport) void update$()
		{
		}
	};
}
