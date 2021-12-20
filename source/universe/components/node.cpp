#include "../../foundation/typeinfo.h"
#include "../world_private.h"
#include "node_private.h"
#include "../systems/scene_private.h"
#include "../systems/renderer_private.h"

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
		data_changed(S<"pos"_h>);
	}

	void cNodePrivate::add_pos(const vec3& p)
	{
		set_pos(pos + p);
	}

	vec3 cNodePrivate::get_euler()
	{ 
		if (eul_dirty)
			update_eul();
		return eul; 
	}

	void cNodePrivate::set_euler(const vec3& e)
	{
		if (eul == e)
			return;
		eul = e;
		rot_dirty = true;
		qut_dirty = true;
		eul_dirty = false;
		mark_transform_dirty();
		data_changed(S<"euler"_h>);
	}

	void cNodePrivate::add_euler(const vec3& e)
	{
		set_euler(eul + e);
	}

	quat cNodePrivate::get_quat() 
	{ 
		if (qut_dirty)
			update_qut();
		return qut; 
	}

	void cNodePrivate::set_quat(const quat& q)
	{
		if (qut == q)
			return;
		qut = q;
		rot_dirty = true;
		eul_dirty = true;
		qut_dirty = false;
		mark_transform_dirty();
		data_changed(S<"quat"_h>);
	}

	void cNodePrivate::set_scale(const vec3& s)
	{
		if (scl == s)
			return;
		scl = s;
		mark_transform_dirty();
		data_changed(S<"scale"_h>);
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

	void cNodePrivate::set_octree_length(float len)
	{
		if (octree_length == len)
			return;
		octree_length = len;
		data_changed(S<"octree_length"_h>);
	}

	void cNodePrivate::set_octree_lod(uint lod)
	{
		if (octree_lod == lod)
			return;
		octree_lod = lod;
		data_changed(S<"octree_lod"_h>);
	}

	bool cNodePrivate::is_any_within_circle(const vec2& c, float r, uint filter_tag)
	{
		assert(octree.get());

		return octree->is_colliding(c, r, filter_tag);
	}

	uint cNodePrivate::get_within_circle(const vec2& c, float r, EntityPtr* dst, uint max_count, uint filter_tag)
	{
		assert(octree.get());

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
		if (dst)
		{
			for (auto i = 0; i < max_count; i++)
				dst[i] = vec[i].first;
		}
		return max_count;
	}

	void cNodePrivate::update_eul()
	{
		if (eul_dirty)
		{
			eul_dirty = false;

			assert(!qut_dirty);
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

			assert(!rot_dirty);
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
				assert(0);
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

			data_changed(S<"transform"_h>);
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
		mark_bounds_dirty(false);
		mark_drawing_dirty();
	}

	void cNodePrivate::mark_bounds_dirty(bool child_caused)
	{
		if (!child_caused && measurers.list.empty())
			return;

		if (!pending_update_bounds)
		{
			if (s_scene)
			{
				s_scene->add_to_update_bounds(this);
				pending_update_bounds = true;
			}
		}

		if (assemble_sub && pnode)
			pnode->mark_bounds_dirty(true);
	}

	void cNodePrivate::mark_drawing_dirty()
	{
		if (s_renderer)
			s_renderer->dirty = true;
	}

	void cNodePrivate::remove_from_bounds_list()
	{
		if (!pending_update_bounds)
			return;
		s_scene->remove_from_update_bounds(this);
		pending_update_bounds = false;
	}

	void cNodePrivate::draw(uint _frame, bool shadow_pass)
	{
		auto first = false;
		if (_frame > frame)
		{
			first = true;
			frame = _frame;
		}
		for (auto d : drawers)
			d->draw(s_renderer, first, shadow_pass);
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
		assert(s_scene);
		s_renderer = entity->world->get_system_t<sRendererPrivate>();
		assert(s_renderer);

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

		remove_from_bounds_list();
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
