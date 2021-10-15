#include "splitter_private.h"

namespace flame
{
	cSplitter* cSplitter::create(void* parms)
	{
		return new cSplitterPrivate();
	}
}
