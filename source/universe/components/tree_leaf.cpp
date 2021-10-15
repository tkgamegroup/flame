#include "tree_leaf_private.h"

namespace flame
{
	cTreeLeaf* cTreeLeaf::create(void* parms)
	{
		return new cTreeLeafPrivate();
	}
}
