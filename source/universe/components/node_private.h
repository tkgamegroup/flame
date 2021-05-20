#pragma once

#include "node.h"

namespace flame
{
	inline auto MAX_OCTREE_OBJECTS = 10;

	struct OctreeNode
	{
		vec3 center;
		float length;
		
		AABB bounds;

		std::vector<cNodePrivate*> objects;

		std::vector<std::unique_ptr<OctreeNode>> children;

		void set_values(float _length, vec3 _center)
		{
			length = _length;
			center = _center;

			bounds = AABB(center, length);
		}

		OctreeNode(float length, vec3 center)
		{
			set_values(length, center);
		}

		bool add(cNodePrivate* n)
		{
			fassert(!n->octree_node);

			n->update_bounds();
			if (!bounds.contains(n->bounds))
				return false;
			sub_add(n);
			return true;
		}

		void remove(cNodePrivate* n)
		{
			fassert(n->octree_node == this);

			for (auto it = objects.begin(); it != objects.end(); it++)
			{
				if (*it == n)
				{
					objects.erase(it);
					n->octree_node = nullptr;
					break;
				}
			}

			if (!children.empty())
			{
				if (should_merge())
					merge();
			}
		}

		bool is_colliding(const AABB& check_bounds)
		{
			if (!bounds.intersects(check_bounds))
				return false;

			for (auto o : objects)
			{
				if (o->bounds.intersects(check_bounds))
					return true;
			}

			if (!children.empty())
			{
				for (auto i = 0; i < 8; i++)
				{
					if (children[i]->is_colliding(check_bounds))
						return true;
				}
			}

			return false;
		}

		//bool is_colliding(const Ray& checkRay, float maxDistance = float.PositiveInfinity)
		//{
		//	float distance;
		//	if (!bounds.IntersectRay(checkRay, out distance) || distance > maxDistance)
		//		return false;

		//	for (auto i = 0; i < objects.Count; i++)
		//	{
		//		if (objects[i].AABB.IntersectRay(checkRay, out distance) && distance <= maxDistance)
		//			return true;
		//	}

		//	if (!children.empty())
		//	{
		//		for (auto i = 0; i < 8; i++)
		//		{
		//			if (children[i].is_colliding(ref checkRay, maxDistance))
		//				return true;
		//		}
		//	}

		//	return false;
		//}

		void get_colliding(const AABB& check_bounds, std::vector<cNodePrivate*>& res)
		{
			if (!bounds.intersects(check_bounds))
				return;

			for (auto obj : objects)
			{
				if (obj->bounds.intersects(check_bounds))
					res.push_back(obj);
			}

			if (!children.empty())
			{
				for (auto i = 0; i < 8; i++)
					children[i]->get_colliding(check_bounds, res);
			}
		}

		//void get_colliding(ref Ray checkRay, List<T> result, float maxDistance = float.PositiveInfinity)
		//{
		//	float distance;
		//	if (!bounds.IntersectRay(checkRay, out distance) || distance > maxDistance)
		//		return;

		//	for (auto i = 0; i < objects.Count; i++)
		//	{
		//		if (objects[i].AABB.IntersectRay(checkRay, out distance) && distance <= maxDistance)
		//			result.add(objects[i].Obj);
		//	}

		//	if (!children.empty())
		//	{
		//		for (auto i = 0; i < 8; i++)
		//			children[i].get_colliding(ref checkRay, result, maxDistance);
		//	}
		//}

		//void get_within_frustum(Plane[] planes, std::vector<cNodePrivate*>& res)
		//{
		//	if (!GeometryUtility.TestPlanesAABB(planes, bounds))
		//		return;

		//	for (auto i = 0; i < objects.Count; i++)
		//	{
		//		if (GeometryUtility.TestPlanesAABB(planes, objects[i].AABB))
		//			result.add(objects[i].Obj);
		//	}

		//	if (!children.empty())
		//	{
		//		for (auto i = 0; i < 8; i++)
		//			children[i].get_within_frustum(planes, result);
		//	}
		//}

		//void SetChildren(OctreeNode*[] childOctrees)
		//{
		//	children = childOctrees;
		//}

		OctreeNode* shrink_if_possible()
		{
			if (length < (2.f))
				return this;
			if (objects.empty() && children.empty())
				return this;

			auto best_fit = -1;
			for (auto i = 0; i < objects.size(); i++)
			{
				auto obj = objects[i];
				int new_best_fit = best_fit_child(obj->bounds.center());
				if (i == 0 || new_best_fit == best_fit)
				{
					if (children[new_best_fit]->bounds.contains(obj->bounds))
					{
						if (best_fit < 0)
							best_fit = new_best_fit;
					}
					else
						return this;
				}
				else
					return this;
			}

			if (!children.empty())
			{
				bool child_had_content = false;
				for (auto i = 0; i < children.size(); i++)
				{
					if (children[i]->has_any_objects())
					{
						if (child_had_content)
							return this;
						if (best_fit >= 0 && best_fit != i)
							return this;
						child_had_content = true;
						best_fit = i;
					}
				}
			}

			if (children.empty())
			{
				set_values(length / 2.f, children[best_fit]->bounds.center());
				return this;
			}

			if (best_fit == -1)
				return this;

			return children[best_fit].get();
		}

		int best_fit_child(vec3 obj_bounds_center)
		{
			return (obj_bounds_center.x <= center.x ? 0 : 1) + (obj_bounds_center.y >= center.y ? 0 : 4) + (obj_bounds_center.z <= center.z ? 0 : 2);
		}

		bool has_any_objects()
		{
			if (!objects.empty()) 
				return true;

			if (!children.empty())
			{
				for (auto i = 0; i < 8; i++)
				{
					if (children[i]->has_any_objects()) 
						return true;
				}
			}

			return false;
		}

		void sub_add(cNodePrivate* n)
		{
			if (children.empty())
			{
				if (objects.size() < MAX_OCTREE_OBJECTS || (length / 2.f) < 1.f)
				{
					objects.push_back(n);
					n->octree_node = this;
					return;
				}

				split();

				for (auto it = objects.end(); it > objects.begin(); )
				{
					it--;
					auto obj = *it;
					auto best_fit = children[best_fit_child(obj->bounds.center())].get();
					if (best_fit->bounds.contains(obj->bounds))
					{
						best_fit->sub_add(obj);
						it = objects.erase(it);
					}
				}
			}

			auto best_fit = children[best_fit_child(n->bounds.center())].get();
			if (best_fit->bounds.contains(n->bounds))
				best_fit->sub_add(n);
			else
			{
				objects.push_back(n);
				n->octree_node = this;
			}
		}

		void split()
		{
			auto quarter = length / 4.f;
			auto hf_length = length / 2;
			children.resize(8);
			children[0].reset(new OctreeNode(hf_length, center + vec3(-quarter, quarter, -quarter)));
			children[1].reset(new OctreeNode(hf_length, center + vec3(quarter, quarter, -quarter)));
			children[2].reset(new OctreeNode(hf_length, center + vec3(-quarter, quarter, quarter)));
			children[3].reset(new OctreeNode(hf_length, center + vec3(quarter, quarter, quarter)));
			children[4].reset(new OctreeNode(hf_length, center + vec3(-quarter, -quarter, -quarter)));
			children[5].reset(new OctreeNode(hf_length, center + vec3(quarter, -quarter, -quarter)));
			children[6].reset(new OctreeNode(hf_length, center + vec3(-quarter, -quarter, quarter)));
			children[7].reset(new OctreeNode(hf_length, center + vec3(quarter, -quarter, quarter)));
		}

		void merge()
		{
			for (auto i = 0; i < 8; i++)
			{
				auto c = children[i].get();
				for (auto j = c->objects.size() - 1; j >= 0; j--)
					objects.push_back(c->objects[j]);
			}
			children.clear();
		}

		bool should_merge()
		{
			auto total = objects.size();
			for (auto& c : children)
			{
				if (!c->children.empty())
					return false;
				total += c->objects.size();
			}
			return total <= MAX_OCTREE_OBJECTS;
		}
	};

	struct cNodePrivate : cNode
	{
		vec3 pos = vec3(0.f);
		vec3 eul = vec3(0.f);
		quat qut = quat(1.f, 0.f, 0.f, 0.f);
		mat3 rot = mat3(1.f);
		vec3 scl = vec3(1.f);
		bool eul_dirty = false;
		bool qut_dirty = false;
		bool rot_dirty = false;
		bool auto_update_eul = false;
		bool auto_update_qut = false;

		cNodePrivate* pnode = nullptr;
		bool transform_dirty = true;
		uint transform_updated_times = 0;
		bool bounds_dirty = true;
		vec3 g_pos;
		quat g_qut;
		mat3 g_rot;
		vec3 g_scl;
		mat4 transform;
		AABB bounds;

		OctreeNode* octree_node = nullptr;

		std::vector<std::unique_ptr<Closure<void(Capture&, sRendererPtr)>>> drawers;
		std::vector<std::unique_ptr<Closure<void(Capture&, AABB*)>>> measurers;

		sRendererPrivate* renderer = nullptr;

		vec3 get_pos() const override { return pos; }
		void set_pos(const vec3& pos) override;
		vec3 get_euler() const override { return eul; }
		void set_euler(const vec3& e) override;
		quat get_quat() const override { return qut; }
		void set_quat(const quat& quat) override;
		vec3 get_scale() const override { return scl; }
		void set_scale(const vec3 & scale) override;

		vec3 get_local_dir(uint idx) override;

		vec3 get_global_pos() override;
		vec3 get_global_dir(uint idx) override;

		void* add_drawer(void (*drawer)(Capture&, sRendererPtr), const Capture& capture) override;
		void remove_drawer(void* drawer) override;
		void* add_measure(void (*measurer)(Capture&, AABB*), const Capture& capture) override;
		void remove_measure(void* measurer) override;

		void update_eul();
		void update_qut();
		void update_rot();
		void update_transform();
		void update_bounds();

		void set_auto_update_eul();
		void set_auto_update_qut();

		void mark_transform_dirty();
		void mark_bounds_dirty();
		void mark_drawing_dirty();

		void on_self_added() override;
		void on_self_removed() override;
		void on_entered_world() override;
		void on_left_world() override;

		bool on_save_attribute(uint h) override;
	};
}
