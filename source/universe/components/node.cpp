#include "../../foundation/typeinfo.h"
#include "../world_private.h"
#include "node_private.h"
#include "../systems/renderer_private.h"

namespace flame
{
	void cNodePrivate::set_pos(const vec3& p)
	{
		if (pos == p)
			return;
		pos = p;
		mark_transform_dirty();
		if (entity)
			entity->component_data_changed(this, S<"pos"_h>);
	}

	void cNodePrivate::set_quat(const quat& q)
	{
		if (qut == q)
			return;
		qut = q;
		rot_dirty = true;
		eul_dirty = true;
		mark_transform_dirty();
		if (entity)
			entity->component_data_changed(this, S<"quat"_h>);
	}

	void cNodePrivate::set_scale(const vec3& s)
	{
		if (scl == s)
			return;
		scl = s;
		mark_transform_dirty();
		if (entity)
			entity->component_data_changed(this, S<"scale"_h>);
	}

	void cNodePrivate::set_euler(const vec3& e)
	{
		if (eul == e)
			return;
		eul = e;
		rot = mat3(eulerAngleYXZ(glm::radians(e.x), glm::radians(e.y), glm::radians(e.z)));
		rot_dirty = false;
		qut_dirty = true;
		mark_transform_dirty();
		if (entity)
			entity->component_data_changed(this, S<"euler"_h>);
	}

	vec3 cNodePrivate::get_local_dir(uint idx)
	{
		update_rot();

		return rot[idx];
	}

	vec3 cNodePrivate::get_global_pos()
	{
		update_transform();

		return transform[3];
	}

	vec3 cNodePrivate::get_global_dir(uint idx)
	{
		update_transform();

		return g_rot[idx];
	}

	void* cNodePrivate::add_drawer(void (*drawer)(Capture&, sRendererPtr), const Capture& capture)
	{
		auto c = new Closure(drawer, capture);
		drawers.emplace_back(c);
		return c;
	}

	void cNodePrivate::remove_drawer(void* drawer)
	{
		std::erase_if(drawers, [&](const auto& i) {
			return i == (decltype(i))drawer;
		});
	}

	void* cNodePrivate::add_measure(bool(*measurer)(Capture&, AABB*), const Capture& capture)
	{
		auto c = new Closure(measurer, capture);
		measurers.emplace_back(c);
		return c;
	}

	void cNodePrivate::remove_measure(void* measurer)
	{
		std::erase_if(measurers, [&](const auto& i) {
			return i == (decltype(i))measurer;
		});
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

			rot = mat3(qut);
		}
	}

	void cNodePrivate::update_transform()
	{
		if (transform_dirty)
		{
			transform_dirty = false;
			transform_updated_times++;

			if (auto_update_eul)
				update_eul();
			if (auto_update_qut)
				update_qut();
			update_rot();

			mat4 m;
			if (pnode)
			{
				pnode->update_transform();
				g_qut = pnode->g_qut * qut;
				g_rot = pnode->g_rot * rot;
				g_scl = pnode->g_scl * scl;
				m = pnode->transform;
			}
			else
			{
				g_qut = qut;
				g_rot = rot;
				g_scl = scl;
				m = mat4(1.f);
			}
			m = translate(m, pos);
			g_pos = m[3];
			m = m * mat4(rot);
			m = scale(m, scl);
			transform = m;

			if (entity)
				entity->component_data_changed(this, S<"transform"_h>);
		}
	}

	void cNodePrivate::update_bounds()
	{
		if (bounds_dirty)
		{
			update_transform();

			bounds_dirty = false;

			bounds.reset();
			for (auto& m : measurers)
			{
				AABB b;
				if (m->call(&b))
				{
					vec3 ps[8];
					b.get_points(ps);
					b.reset();
					for (auto i = 0; i < 8; i++)
						b.expand(transform * vec4(ps[i], 1.f));

					bounds.expand(b);
				}
			}
			for (auto& c : entity->children)
			{
				auto node = c->get_component_i<cNodePrivate>(0);
				if (node)
				{
					node->update_bounds();
					bounds.expand(node->bounds);
				}
			}

			if (entity)
				entity->component_data_changed(this, S<"bounds"_h>);
		}
	}

	void cNodePrivate::set_auto_update_eul()
	{
		auto_update_eul = true;
		if (pnode)
			pnode->set_auto_update_eul();
	}

	void cNodePrivate::set_auto_update_qut()
	{
		auto_update_qut = true;
		if (pnode)
			pnode->set_auto_update_qut();
	}

	void cNodePrivate::mark_transform_dirty()
	{
		if (!transform_dirty)
		{
			transform_dirty = true;

			for (auto& c : entity->children)
			{
				auto n = c->get_component_i<cNodePrivate>(0);
				if (n)
					n->mark_transform_dirty();
			}
		}
		mark_bounds_dirty();
		mark_drawing_dirty();
	}

	void cNodePrivate::mark_bounds_dirty()
	{
		if (!bounds_dirty)
		{
			bounds_dirty = true;

			if (octree_node)
				;

			if (pnode)
				pnode->mark_bounds_dirty();
		}
	}

	void cNodePrivate::mark_drawing_dirty()
	{
		if (renderer)
			renderer->dirty = true;
	}

	void cNodePrivate::on_self_added()
	{
		pnode = entity->get_parent_component_t<cNodePrivate>();
		if (pnode)
			mark_transform_dirty();
	}

	void cNodePrivate::on_self_removed()
	{
		pnode = nullptr;
	}

	void cNodePrivate::on_entered_world()
	{
		auto world = entity->world;
		if (!world->first_node)
			world->first_node = entity;
		renderer = entity->world->get_system_t<sRendererPrivate>();
		fassert(renderer);
		mark_transform_dirty();
	}

	void cNodePrivate::on_left_world()
	{
		auto world = entity->world;
		if (world->first_node == entity)
			world->first_node = nullptr;
		mark_drawing_dirty();
		renderer = nullptr;
	}

	bool cNodePrivate::on_save_attribute(uint h)
	{
		switch (h)
		{
		case S<"euler"_h>:
			update_eul();
			return true;
		case S<"quat"_h>:
			return false;
		}
		return true;
	}

	cNode* cNode::create(void* parms)
	{
		return f_new<cNodePrivate>();
	}
}
