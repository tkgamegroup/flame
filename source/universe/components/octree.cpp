#include "../entity_private.h"
#include "node_private.h"
#include "octree_private.h"

namespace flame
{
	void cOctreePrivate::set_length(float _length)
	{
		length = _length;
	}

	void cOctreePrivate::add_object(cNodePrivate* obj)
	{

	}

	void cOctreePrivate::remove_object(cNodePrivate* obj)
	{

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
			add_object(c->get_component_i<cNodePrivate>(0));
	}

	void cOctreePrivate::on_left_world()
	{
		octree.reset();
	}

	void cOctreePrivate::on_child_added(EntityPtr e)
	{
		if (octree)
			add_object(e->get_component_i<cNodePrivate>(0));
	}

	void cOctreePrivate::on_child_removed(EntityPtr e)
	{
		if (octree)
			remove_object(e->get_component_i<cNodePrivate>(0));
	}

	cOctree* cOctree::create(void* parms)
	{
		return f_new<cOctreePrivate>();
	}
}
