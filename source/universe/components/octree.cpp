//#include "octree_private.h"
//
//namespace flame
//{
//	void cOctreePrivate::set_length(float len)
//	{
//		if (length == len)
//			return;
//		length = len;
//		data_changed("length"_h);
//	}
//
//	void cOctreePrivate::set_lod(uint _lod)
//	{
//		if (lod == _lod)
//			return;
//		lod = _lod;
//		data_changed("lod"_h);
//	}
//
//	bool cOctreePrivate::is_any_within_circle(const vec2& c, float r, uint filter_tag)
//	{
//		if (!octree)
//			return false;
//
//		return octree->is_colliding(c, r, filter_tag);
//	}
//
//	uint cOctreePrivate::get_within_circle(const vec2& c, float r, EntityPtr* dst, uint max_count, uint filter_tag)
//	{
//		if (!octree)
//			return 0;
//
//		std::vector<cNodePrivate*> res;
//		octree->get_colliding(c, r, res, filter_tag);
//		if (res.empty())
//			return 0;
//
//		std::vector<std::pair<EntityPrivate*, float>> vec;
//		vec.resize(res.size());
//		for (auto i = 0; i < res.size(); i++)
//		{
//			vec[i].first = res[i]->entity;
//			vec[i].second = distance(c, res[i]->g_pos.xz());
//		}
//		std::sort(vec.begin(), vec.end(), [](const auto& a, const auto& b) {
//			return a.second < b.second;
//			});
//		if (max_count > vec.size())
//			max_count = vec.size();
//		if (dst)
//		{
//			for (auto i = 0; i < max_count; i++)
//				dst[i] = vec[i].first;
//		}
//		return max_count;
//	}
//
//	struct cOctreeCreatePrivate : cOctree::Create
//	{
//		cOctreePtr operator()() override
//		{
//			return new cOctreePrivate();
//		}
//	}cOctree_create_private;
//	cOctree::Create& cOctree::create = cOctree_create_private;
//}
