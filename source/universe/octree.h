#include "entity.h"
#include "components/node.h"

namespace flame
{
	inline auto OCTREE_MAX_OBJECTS = 12;

	struct OctNode
	{
		vec3 center;
		float length;

		AABB bounds;

		std::vector<cNodePtr> objects;

		OctNode* parent = nullptr;
		std::vector<std::unique_ptr<OctNode>> children;

		void set_values(float _length, vec3 _center)
		{
			length = _length;
			center = _center;

			bounds = AABB(center, length);
		}

		OctNode(float length, vec3 center, OctNode* _parent = nullptr)
		{
			parent = _parent;
			set_values(length, center);
		}

		void add(cNodePtr n)
		{
			assert(!n->octnode);
			if (bounds.contains(n->bounds))
				sub_add(n);
		}

		void remove(cNodePtr n)
		{
			for (auto it = objects.begin(); it != objects.end(); it++)
			{
				if (*it == n)
				{
					objects.erase(it);
					n->octnode = nullptr;
					break;
				}
			}

			if (!children.empty())
			{
				if (should_merge())
					merge();
			}
		}

		bool is_colliding(const AABB& check_bounds, uint any_filter = 0xffffffff, uint all_filter = 0, uint parent_search_times = 0)
		{
			if (!bounds.intersects(check_bounds))
				return false;

			for (auto obj : objects)
			{
				auto e = obj->entity;
				auto t = parent_search_times;
				while (e)
				{
					if (e->global_enable && (any_filter & e->tag) != 0 && (all_filter & e->tag) == all_filter)
						break;
					e = e->parent;
					t--;
					if (t == 0)
					{
						e = nullptr;
						break;
					}
				}

				if (e)
				{
					if (obj->bounds.intersects(check_bounds))
						return true;
				}
			}

			for (auto& c : children)
			{
				if (c->is_colliding(check_bounds, any_filter, all_filter, parent_search_times))
					return true;
			}

			return false;
		}

		void get_colliding(const AABB& check_bounds, std::vector<std::pair<EntityPtr, cNodePtr>>& res, uint any_filter = 0xffffffff, uint all_filter = 0, uint parent_search_times = 0)
		{
			if (!bounds.intersects(check_bounds))
				return;

			for (auto obj : objects)
			{
				auto e = obj->entity;
				auto t = parent_search_times;
				while (e)
				{
					if (e->global_enable && (any_filter & e->tag) != 0 && (all_filter & e->tag) == all_filter)
						break;
					e = e->parent;
					t--;
					if (t == 0)
					{
						e = nullptr;
						break;
					}
				}

				if (e)
				{
					if (obj->bounds.intersects(check_bounds))
						res.emplace_back(e, obj);
				}
			}

			for (auto& c : children)
				c->get_colliding(check_bounds, res, any_filter, all_filter, parent_search_times);
		}

		bool is_colliding(const vec2& check_center, float check_radius, uint any_filter = 0xffffffff, uint all_filter = 0, uint parent_search_times = 0)
		{
			if (!bounds.intersects(check_center, check_radius))
				return false;

			for (auto obj : objects)
			{
				auto e = obj->entity;
				auto t = parent_search_times;
				while (e)
				{
					if (e->global_enable && (any_filter & e->tag) != 0 && (all_filter & e->tag) == all_filter)
						break;
					e = e->parent;
					t--;
					if (t == 0)
					{
						e = nullptr;
						break;
					}
				}

				if (e)
				{
					if (obj->bounds.intersects(check_center, check_radius))
						return true;
				}
			}

			for (auto& c : children)
			{
				if (c->is_colliding(check_center, check_radius, any_filter, all_filter))
					return true;
			}

			return false;
		}

		void get_colliding(const vec2& check_center, float check_radius, std::vector<std::pair<EntityPtr, cNodePtr>>& res, uint any_filter = 0xffffffff, uint all_filter = 0, uint parent_search_times = 0)
		{
			if (!bounds.intersects(check_center, check_radius))
				return;

			for (auto obj : objects)
			{
				auto e = obj->entity;
				auto t = parent_search_times;
				while (e)
				{
					if (e->global_enable && (any_filter & e->tag) != 0 && (all_filter & e->tag) == all_filter)
						break;
					e = e->parent;
					t--;
					if (t == 0)
					{
						e = nullptr;
						break;
					}
				}

				if (e)
				{
					if (obj->bounds.intersects(check_center, check_radius))
						res.emplace_back(e, obj);
				}
			}

			for (auto& c : children)
				c->get_colliding(check_center, check_radius, res, any_filter, all_filter);
		}

		bool is_colliding(const vec3& check_center, float check_radius, uint any_filter = 0xffffffff, uint all_filter = 0, uint parent_search_times = 0)
		{
			if (!bounds.intersects(check_center, check_radius))
				return false;

			for (auto obj : objects)
			{
				auto e = obj->entity;
				auto t = parent_search_times;
				while (e)
				{
					if (e->global_enable && (any_filter & e->tag) != 0 && (all_filter & e->tag) == all_filter)
						break;
					e = e->parent;
					t--;
					if (t == 0)
					{
						e = nullptr;
						break;
					}
				}

				if (e)
				{
					if (obj->bounds.intersects(check_center, check_radius))
						return true;
				}
			}

			for (auto& c : children)
			{
				if (c->is_colliding(check_center, check_radius, any_filter, all_filter))
					return true;
			}

			return false;
		}

		void get_colliding(const vec3& check_center, float check_radius, std::vector<std::pair<EntityPtr, cNodePtr>>& res, uint any_filter = 0xffffffff, uint all_filter = 0, uint parent_search_times = 0)
		{
			if (!bounds.intersects(check_center, check_radius))
				return;

			for (auto obj : objects)
			{
				auto e = obj->entity;
				auto t = parent_search_times;
				while (e)
				{
					if (e->global_enable && (any_filter & e->tag) != 0 && (all_filter & e->tag) == all_filter)
						break;
					e = e->parent;
					t--;
					if (t == 0)
					{
						e = nullptr;
						break;
					}
				}

				if (e)
				{
					if (obj->bounds.intersects(check_center, check_radius))
						res.emplace_back(e, obj);
				}
			}

			for (auto& c : children)
				c->get_colliding(check_center, check_radius, res, any_filter, all_filter);
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

		//	for (auto& c : children)
		//	{
		//		if (c->is_colliding(ref checkRay, maxDistance))
		//			return true;
		//	}

		//	return false;
		//}

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

		//	for (auto& c : children)
		//		c->get_colliding(ref checkRay, result, maxDistance);
		//}

		void get_within_frustum(const Frustum& frustum, std::vector<std::pair<EntityPtr, cNodePtr>>& res, uint any_filter = 0xffffffff, uint all_filter = 0, uint parent_search_times = 0)
		{
			if (!AABB_frustum_check(frustum, bounds))
				return;

			for (auto obj : objects)
			{
				auto e = obj->entity;
				auto t = parent_search_times;
				while (e)
				{
					if (e->global_enable && (any_filter & e->tag) != 0 && (all_filter & e->tag) == all_filter)
						break;
					e = e->parent;
					t--;
					if (t == 0)
					{
						e = nullptr;
						break;
					}
				}

				if (e)
				{
					if (AABB_frustum_check(frustum, obj->bounds))
						res.emplace_back(e, obj);
				}
			}

			for (auto& c : children)
				c->get_within_frustum(frustum, res, any_filter, all_filter);
		}

		OctNode* shrink_if_possible()
		{
			if (length < 2.f || (objects.empty() && children.empty()))
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

		int best_fit_child(vec3 p)
		{
			return (p.x <= center.x ? 0 : 1) + (p.y >= center.y ? 0 : 4) + (p.z <= center.z ? 0 : 2);
		}

		bool has_any_objects()
		{
			if (!objects.empty())
				return true;

			for (auto& c : children)
			{
				if (c->has_any_objects())
					return true;
			}

			return false;
		}

		void sub_add(cNodePtr n)
		{
			if (children.empty())
			{
				if (objects.size() < OCTREE_MAX_OBJECTS || (length / 2.f) < 1.f)
				{
					n->octnode = this;
					objects.push_back(n);
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
						obj->octnode = nullptr;
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
				n->octnode = this;
				objects.push_back(n);
			}
		}

		void split()
		{
			auto quarter = length / 4.f;
			auto hf_length = length / 2;
			children.resize(8);
			children[0].reset(new OctNode(hf_length, center + vec3(-quarter, quarter, -quarter), this));
			children[1].reset(new OctNode(hf_length, center + vec3(quarter, quarter, -quarter), this));
			children[2].reset(new OctNode(hf_length, center + vec3(-quarter, quarter, quarter), this));
			children[3].reset(new OctNode(hf_length, center + vec3(quarter, quarter, quarter), this));
			children[4].reset(new OctNode(hf_length, center + vec3(-quarter, -quarter, -quarter), this));
			children[5].reset(new OctNode(hf_length, center + vec3(quarter, -quarter, -quarter), this));
			children[6].reset(new OctNode(hf_length, center + vec3(-quarter, -quarter, quarter), this));
			children[7].reset(new OctNode(hf_length, center + vec3(quarter, -quarter, quarter), this));
		}

		void merge()
		{
			for (auto& c : children)
			{
				for (auto obj : c->objects)
				{
					objects.push_back(obj);
					obj->octnode = this;
				}
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
			return total <= OCTREE_MAX_OBJECTS;
		}
	};
}
