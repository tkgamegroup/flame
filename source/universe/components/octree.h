//#pragma once
//
//#include "../component.h"
//
//namespace flame
//{
//	struct cOctree : Component
//	{
//		inline static auto type_name = "flame::cOctree";
//		inline static auto type_hash = sh(type_name);
//
//		float length = 0.f;
//		uint lod = 0;
//
//		cOctree() :
//			Component(type_name, type_hash)
//		{
//		}
//
//		virtual void set_length(float len) = 0;
//		virtual void set_lod(uint lod) = 0;
//
//		virtual bool is_any_within_circle(const vec2& c, float r, uint filter_tag = 0xffffffff) = 0;
//		virtual uint get_within_circle(const vec2& c, float r, EntityPtr* dst, uint max_count, uint filter_tag = 0xffffffff) = 0;
//
//		struct Create
//		{
//			virtual cOctreePtr operator()() = 0;
//		};
//		FLAME_UNIVERSE_EXPORTS static Create& create;
//	};
//}
