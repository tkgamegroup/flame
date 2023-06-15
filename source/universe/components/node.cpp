#include "../world_private.h"
#include "node_private.h"
#include "../systems/renderer_private.h"
#include "../octree.h"

namespace flame
{
	void cNodePrivate::set_pos(const vec3& p)
	{
		if (pos == p)
			return;
		pos = p;
		mark_transform_dirty();
		data_changed("pos"_h);
	}

	vec3 cNodePrivate::get_eul()
	{
		return degrees(eulerAngles(qut)).yxz();
	}

	void cNodePrivate::set_eul(const vec3& e)
	{
		qut = quat(mat3(eulerAngleYXZ(radians(e.x), radians(e.y), radians(e.z))));
		mark_transform_dirty();
		data_changed("eul"_h);
	}

	void cNodePrivate::set_qut(const quat& q)
	{
		if (qut == q)
			return;
		qut = q;
		mark_transform_dirty();
		data_changed("qut"_h);
	}

	void cNodePrivate::set_scl(const vec3& s)
	{
		if (scl == s)
			return;
		scl = s;
		mark_transform_dirty();
		data_changed("scl"_h);
	}

	void cNodePrivate::look_at(const vec3& t)
	{
		update_transform();
		qut = inverse(mat3(lookAt(vec3(0.f), t - global_pos(), vec3(0.f, 1.f, 0.f))));
		mark_transform_dirty();
	}

	bool cNodePrivate::update_transform()
	{
		if (!transform_dirty)
			return false;

		if (auto pnode = entity->get_parent_component_i<cNodeT>(0); pnode)
		{
			g_qut = qut * pnode->g_qut;
			transform = pnode->transform;
		}
		else
		{
			g_qut = qut;
			transform = mat4(1.f);
		}
		transform = translate(transform, pos);
		transform = transform * mat4(qut);
		transform = scale(transform, scl);

		data_changed("transform"_h);
		transform_dirty = false;

		return true;
	}

	vec3 cNodePrivate::global_scl()
	{
		auto ret = scl;
		auto pnode = entity->get_parent_component_i<cNodeT>(0);
		while (pnode)
		{
			ret *= pnode->scl;
			pnode = pnode->entity->get_parent_component_i<cNodeT>(0);
		}
		return ret;
	}

	void cNodePrivate::mark_transform_dirty()
	{
		transform_dirty = true;
		mark_drawing_dirty();
	}

	void cNodePrivate::mark_drawing_dirty()
	{
		if (entity->depth != (ushort)-1)
			sRenderer::instance()->dirty = true;
	}

	void cNodePrivate::on_active()
	{
		mark_transform_dirty();
	}

	void cNodePrivate::on_inactive()
	{
		if (octnode)
			octnode->remove(this);
		
		mark_drawing_dirty();
	}

	struct cNodeCreate : cNode::Create
	{
		cNodePtr operator()(EntityPtr) override
		{
			return new cNodePrivate();
		}
	}cNode_create;
	cNode::Create& cNode::create = cNode_create;
}
