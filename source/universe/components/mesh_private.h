#pragma once

#include "mesh.h"
#include "node_private.h"

namespace flame
{
	struct cMeshPrivate : cMesh
	{
		cArmaturePtr parmature = nullptr;

		void* drawer_lis = nullptr;
		void* measurer_lis = nullptr;

		int mesh_id = -1;
		graphics::MeshPtr mesh = nullptr;

		int transform_id = -1;
		uint frame = 0;

		~cMeshPrivate();

		void on_init() override;

		void set_model_name(const std::filesystem::path& model_name) override;
		void set_mesh_index(uint idx) override;
		void set_skin_index(uint idx) override;

		void set_cast_shadow(bool v) override;
		void set_shading_flags(ShadingFlags flags) override;

		void apply_src();

		void draw(sNodeRendererPtr renderer, bool shadow_pass);

		void on_active() override;
		void on_inactive() override;
	};
}
