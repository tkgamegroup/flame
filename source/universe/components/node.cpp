#include "../../foundation/typeinfo.h"
#include "../world_private.h"
#include "node_private.h"
//#include "../systems/scene_private.h" // TODO
#include "../systems/node_renderer_private.h"

namespace flame
{
	cNodePrivate::cNodePrivate()
	{
		bounds.reset();
	}

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
			eul_dirty = false;

			auto res = eulerAngles(qut);
			eul.x = glm::degrees(res.y);
			eul.y = glm::degrees(res.x);
			eul.z = glm::degrees(res.z);
		}
	}

	void cNodePrivate::update_qut()
	{
		if (qut_dirty)
		{
			qut_dirty = false;

			qut = quat(rot);
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

	void cNodePrivate::update_transform()
	{
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
		m = scale(m, scl);
		m = m * mat4(rot);
		transform = m;
		g_pos = m[3];

		data_changed("transform"_h);
		transform_dirty = false;
	}

	void cNodePrivate::mark_transform_dirty()
	{
		if (!transform_dirty)
			transform_dirty = true;
		mark_drawing_dirty();
	}

	void cNodePrivate::mark_drawing_dirty()
	{
		if (s_renderer)
			s_renderer->dirty = true;
	}

	void cNodePrivate::draw(uint _frame, bool shadow_pass)
	{
		for (auto& d : drawers.list)
			d(s_renderer, shadow_pass);
	}

	void cNodePrivate::on_entered_world()
	{
		auto world = entity->world;
		if (!world->first_node)
			world->first_node = entity;

		// TODO
		//s_scene = entity->world->get_system_t<sScenePrivate>();
		//s_renderer = entity->world->get_system_t<sNodeRendererPrivate>();

		// TOOD
		//if (octree_length > 0.f)
		//	octree.reset(new OctNode(octree_length, g_pos + vec3(octree_length * 0.5f)));
		//else if (!assemble_sub)
		//{
		//	std::function<OctNode* (cNodePrivate* n)> get_octree;
		//	get_octree = [&](cNodePrivate* n)->OctNode* {
		//		if (!n)
		//			return nullptr;
		//		if (n->octree)
		//			return n->octree.get();
		//		return get_octree(n->pnode);
		//	};
		//	octnode.first = get_octree(pnode);
		//}

		mark_transform_dirty();
	}

	void cNodePrivate::on_left_world()
	{
		auto world = entity->world;
		if (world->first_node == entity)
			world->first_node = nullptr;

		// TODO
		//if (octnode.second)
		//	octnode.second->remove(this);

		if (entity->global_enable)
			mark_drawing_dirty();
	}

	struct cNodeCreatePrivate : cNode::Create
	{
		cNodePtr operator()(EntityPtr) override
		{
			return new cNodePrivate();
		}
	}cNode_create_private;
	cNode::Create& cNode::create = cNode_create_private;
}
