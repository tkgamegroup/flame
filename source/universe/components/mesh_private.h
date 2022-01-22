#pragma once

#include "mesh.h"
#include "node_private.h"

namespace flame
{
	struct cMeshPrivate : cMesh
	{
		void* drawer_lis = nullptr;
		void* measurer_lis = nullptr;

		int mesh_id = -1;
		graphics::MeshPtr mesh = nullptr;

		cArmaturePtr parmature = nullptr;
		int object_id = -1;
		int frame = -1;

		~cMeshPrivate();

		void on_init() override;

		void set_model_name(const std::filesystem::path& model_name) override;
		void set_mesh_index(uint idx) override;
		void set_skin_index(uint idx) override;

		void set_cast_shadow(bool v) override;

		void apply_src();

		void draw(sNodeRendererPtr renderer, bool shadow_pass);

		void on_active() override;
		void on_inactive() override;
	};
}
