#include "../../foundation/typeinfo.h"
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
		if (eul_dirty)
			update_eul();
		return eul; 
	}

	void cNodePrivate::set_eul(const vec3& e)
	{
		if (eul == e)
			return;
		eul = e;
		rot_dirty = true;
		qut_dirty = true;
		eul_dirty = false;
		mark_transform_dirty();
		data_changed("eul"_h);
	}

	quat cNodePrivate::get_qut() 
	{ 
		if (qut_dirty)
			update_qut();
		return qut; 
	}

	void cNodePrivate::set_qut(const quat& q)
	{
		if (qut == q)
			return;
		qut = q;
		rot_dirty = true;
		eul_dirty = true;
		qut_dirty = false;
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
		rot = inverse(lookAt(vec3(0.f), t - g_pos, vec3(0.f, 1.f, 0.f)));
		rot_dirty = false;
		qut_dirty = true;
		eul_dirty = true;
		mark_transform_dirty();
	}

	void cNodePrivate::update_eul()
	{
		if (eul_dirty)
		{
			update_rot();
			if (!qut_dirty)
				eul = degrees(eulerAngles(qut)).yxz();
			else
				assert(0);
			eul_dirty = false;
		}
	}

	void cNodePrivate::update_qut()
	{
		if (qut_dirty)
		{
			update_rot();
			if (!rot_dirty)
				qut = quat(rot);
			else
				assert(0);
			qut_dirty = false;
		}
	}

	void cNodePrivate::update_rot()
	{
		if (rot_dirty)
		{
			rot_dirty = false;

			if (!qut_dirty)
				rot = mat3(qut);
			else if (!eul_dirty)
				rot = mat3(eulerAngleYXZ(radians(eul.x), radians(eul.y), radians(eul.z)));
		}
	}

	bool cNodePrivate::update_transform()
	{
		if (!transform_dirty)
			return false;

		update_rot();

		mat4 m;
		if (auto pnode = entity->get_parent_component_i<cNodeT>(0); pnode)
		{
			g_rot = pnode->g_rot * rot;
			g_scl = pnode->g_scl * scl;
			m = pnode->transform;
		}
		else
		{
			g_rot = rot;
			g_scl = scl;
			m = mat4(1.f);
		}
		m = translate(m, pos);
		m = m * mat4(rot);
		m = scale(m, scl);
		transform = m;
		g_pos = m[3];

		data_changed("transform"_h);
		transform_dirty = false;

		return true;
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

	void cNodePrivate::draw(DrawData& draw_data)
	{
		for (auto& d : drawers.list)
			d.first(draw_data);
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
