#include "../entity_private.h"
#include "node_private.h"
#include "octree_private.h"

namespace flame
{
	static void new_child(cOctreePrivate* thiz, cNodePrivate* obj)
	{
		obj->under_octree = true;
		obj->update_bounds();
		thiz->octree->add(obj);
	}

	void cOctreePrivate::set_length(float _length)
	{
		length = _length;
	}

	void cOctreePrivate::on_added()
	{
		node = entity->get_component_i<cNodePrivate>(0);
		fassert(node);
		node->is_octree = true;
	}

	void cOctreePrivate::on_removed()
	{
		node->is_octree = false;
		node = nullptr;
	}

	void cOctreePrivate::on_entered_world()
	{
		node->update_transform();

		octree.reset(new OctNode(length, node->g_pos));

		for (auto& c : entity->children)
			new_child(this, c->get_component_i<cNodePrivate>(0));
	}

	void cOctreePrivate::on_left_world()
	{
		octree.reset();
	}

	void cOctreePrivate::on_child_added(EntityPtr e)
	{
		if (octree)
			new_child(this, e->get_component_i<cNodePrivate>(0));
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
