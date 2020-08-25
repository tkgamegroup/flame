#include "node_private.h"

namespace flame
{
	cNode* cNode::create()
	{
		return f_new<cNodePrivate>();
	}
}
