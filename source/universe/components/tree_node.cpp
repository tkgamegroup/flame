#include "tree_node_private.h"

namespace flame
{
	cTreeNode* cTreeNode::create(void* parms)
	{
		return new cTreeNodePrivate();
	}
}
