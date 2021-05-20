#include "../entity_private.h"
#include "node_private.h"
#include "octree_private.h"

namespace flame
{
	void cOctreePrivate::set_length(float _length)
	{
		length = _length;
	}

	void cOctreePrivate::on_added()
	{
		node = entity->get_component_i<cNodePrivate>(0);
		fassert(node);
	}

	void cOctreePrivate::on_removed()
	{
		node = nullptr;
	}

	void cOctreePrivate::on_entered_world()
	{
		node->update_transform();

		octree.reset(new OctreeNode(length, node->g_pos));

		for (auto& c : entity->children)
		{
			auto obj = c->get_component_i<cNodePrivate>(0);
			obj->update_bounds();
			octree->add(obj);
		}
	}

	void cOctreePrivate::on_left_world()
	{
		octree.reset();
	}

	void cOctreePrivate::on_child_added(EntityPtr e)
	{
		if (octree)
		{
			auto obj = e->get_component_i<cNodePrivate>(0);
			obj->update_bounds();
			octree->add(obj);
		}
	}

	void cOctreePrivate::on_child_removed(EntityPtr e)
	{
		if (octree)
			;
	}

	cOctree* cOctree::create(void* parms)
	{
		return f_new<cOctreePrivate>();
	}
}
