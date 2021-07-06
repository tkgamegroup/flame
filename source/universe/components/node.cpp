#include "../../foundation/typeinfo.h"
#include "../world_private.h"
#include "node_private.h"
#include "../systems/scene_private.h"
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

	void cNodePrivate::add_pos(const vec3& p)
	{
		set_pos(pos + p);
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
		rot_dirty = true;
		qut_dirty = true;
		mark_transform_dirty();
		if (entity)
			entity->component_data_changed(this, S<"euler"_h>);
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

	void cNodePrivate::set_octree_length(float len)
	{
		if (octree_length == len)
			return;
		octree_length = len;
		if (entity)
			entity->component_data_changed(this, S<"octree_length"_h>);
	}

	void cNodePrivate::add_drawer(NodeDrawer* d)
	{
		drawers.push_back(d);
	}

	void cNodePrivate::remove_drawer(NodeDrawer* d)
	{
		std::erase_if(drawers, [&](const auto& i) {
			return i == d;
		});
	}

	void cNodePrivate::add_measurer(NodeMeasurer* m)
	{
		measurers.push_back(m);
	}

	void cNodePrivate::remove_measurer(NodeMeasurer* m)
	{
		std::erase_if(measurers, [&](const auto& i) {
			return i == m;
		});
	}

	bool cNodePrivate::is_any_within_circle(const vec2& c, float r, uint filter_tag)
	{
		fassert(octree.get());

		return octree->is_colliding(c, r, filter_tag);
	}

	uint cNodePrivate::get_within_circle(const vec2& c, float r, EntityPtr* dst, uint max_count, uint filter_tag)
	{
		fassert(octree.get());

		std::vector<cNodePrivate*> res;
		octree->get_colliding(c, r, res, filter_tag);
		if (res.empty())
			return 0;

		std::vector<std::pair<EntityPrivate*, float>> vec;
		vec.resize(res.size());
		for (auto i = 0; i < res.size(); i++)
		{
			vec[i].first = res[i]->entity;
			vec[i].second = distance(c, res[i]->g_pos.xz());
		}
		std::sort(vec.begin(), vec.end(), [](const auto& a, const auto& b) {
			return a.second < b.second;
		});
		if (max_count > vec.size())
			max_count = vec.size();
		for (auto i = 0; i < max_count; i++)
			dst[i] = vec[i].first;
		return max_count;
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
			else
				fassert(0);
		}
	}

	void cNodePrivate::update_transform()
	{
		if (transform_dirty)
		{
			transform_dirty = false;
			transform_updated_times++;

			update_rot();

			mat4 m;
			if (pnode)
			{
				pnode->update_transform();
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
			for (auto m : measurers)
			{
				AABB b;
				if (m->measure(&b))
				{
					vec3 ps[8];
					b.get_points(ps);
					b.reset();
					for (auto i = 0; i < 8; i++)
						b.expand(transform * vec4(ps[i], 1.f));

					bounds.expand(b);
				}
			}

			if (!assemble_sub)
			{
				for (auto& c : entity->children)
				{
					auto node = c->get_component_i<cNodePrivate>(0);
					if (node)
					{
						node->update_bounds();
						bounds.expand(node->bounds);
					}
				}
			}

			if (entity)
				entity->component_data_changed(this, S<"bounds"_h>);
		}
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

			if (assemble_sub && pnode)
				pnode->mark_bounds_dirty();
		}

		if (!pending_reindex && octnode.first)
		{
			s_scene->add_to_reindex(this);
			pending_reindex = true;
		}
	}

	void cNodePrivate::mark_drawing_dirty()
	{
		if (s_renderer)
			s_renderer->dirty = true;
	}

	void cNodePrivate::remove_from_reindex_list()
	{
		if (!pending_reindex)
			return;
		s_scene->remove_from_reindex(this);
		pending_reindex = false;
	}

	void cNodePrivate::on_component_added(Component* c)
	{
		auto drawer = dynamic_cast<NodeDrawer*>(c);
		if (drawer)
			add_drawer(drawer);
		auto measurer = dynamic_cast<NodeMeasurer*>(c);
		if (measurer)
			add_measurer(measurer);
	}

	void cNodePrivate::on_component_removed(Component* c)
	{
		auto drawer = dynamic_cast<NodeDrawer*>(c);
		if (drawer)
			remove_drawer(drawer);
		auto measurer = dynamic_cast<NodeMeasurer*>(c);
		if (measurer)
			remove_measurer(measurer);
	}

	void cNodePrivate::on_entered_world()
	{
		auto world = entity->world;
		if (!world->first_node)
			world->first_node = entity;

		s_scene = entity->world->get_system_t<sScenePrivate>();
		fassert(s_scene);
		s_renderer = entity->world->get_system_t<sRendererPrivate>();
		fassert(s_renderer);

		pnode = entity->get_parent_component_t<cNodePrivate>();

		if (octree_length > 0.f)
		{
			update_transform();
			octree.reset(new OctNode(octree_length, g_pos + vec3(octree_length * 0.5f)));
		}
		else if (!assemble_sub)
		{
			std::function<OctNode* (cNodePrivate* n)> get_octree;
			get_octree = [&](cNodePrivate* n)->OctNode* {
				if (!n)
					return nullptr;
				if (n->octree)
					return n->octree.get();
				return get_octree(n->pnode);
			};
			octnode.first = get_octree(pnode);
		}

		mark_transform_dirty();
	}

	void cNodePrivate::on_left_world()
	{
		auto world = entity->world;
		if (world->first_node == entity)
			world->first_node = nullptr;

		if (octnode.second)
			octnode.second->remove(this);

		remove_from_reindex_list();
		mark_drawing_dirty();

		s_scene = nullptr;
		s_renderer = nullptr;

		pnode = nullptr;
		octnode = { nullptr, nullptr };
	}

	cNode* cNode::create(void* parms)
	{
		return new cNodePrivate();
	}
}
