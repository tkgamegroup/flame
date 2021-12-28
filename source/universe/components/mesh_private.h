//#pragma once
//
//#include "mesh.h"
//#include "node_private.h"
//
//namespace flame
//{
//	struct cMeshPrivate : cMesh
//	{
//		cNodePrivate* node = nullptr;
//		cArmaturePrivate* parmature = nullptr;
//		sNodeRendererPrivate* s_renderer = nullptr;
//
//		void* drawer_lis = nullptr;
//		void* measurer_lis = nullptr;
//
//		int mesh_id = -1;
//		graphics::Mesh* mesh = nullptr;
//
//		int transform_id = -1;
//		uint frame = 0;
//
//		void set_model_name(const std::filesystem::path& model_name) override;
//		void set_mesh_index(uint idx) override;
//		void set_skin_index(uint idx) override;
//
//		void set_cast_shadow(bool v) override;
//		void set_shading_flags(ShadingFlags flags) override;
//
//		void apply_src();
//
//		void on_added() override;
//		void on_removed() override;
//		void on_entered_world() override;
//		void on_left_world() override;
//	};
//}
