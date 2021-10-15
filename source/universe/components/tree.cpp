#include "tree_private.h"

namespace flame
{
	cTree* cTree::create(void* parms)
	{
		return new cTreePrivate();
	}
}
