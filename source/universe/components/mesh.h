//#pragma once
//
//#include "../component.h"
//
//namespace flame
//{
//	struct cMesh : Component
//	{
//		inline static auto type_name = "flame::cMesh";
//		inline static auto type_hash = sh(type_name);
//
//		std::filesystem::path model_name;
//		uint mesh_index = 0;
//		uint skin_index = 0;
//
//		bool cast_shadow = true;
//		ShadingFlags shading_flags = ShadingMaterial;
//
//		cMesh() :
//			Component(type_name, type_hash)
//		{
//		}
//
//		virtual void set_model_name(const std::filesystem::path& model_name) = 0;
//		virtual void set_mesh_index(uint idx) = 0;
//		virtual void set_skin_index(uint idx) = 0;
//
//		virtual void set_cast_shadow(bool v) = 0;
//		virtual void set_shading_flags(ShadingFlags flags) = 0;
//
//		struct Create
//		{
//			virtual cMeshPtr operator()() = 0;
//		};
//		FLAME_UNIVERSE_EXPORTS static Create& create;
//	};
//}
