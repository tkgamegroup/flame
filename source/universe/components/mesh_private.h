#pragma once

#include "mesh.h"

namespace flame
{
	struct cMeshPrivate : cMesh
	{
		graphics::MeshPtr mesh = nullptr;

		cArmaturePtr parmature = nullptr;

		int frame = -1;

		~cMeshPrivate();
		void on_init() override;

		void set_model_name(const std::filesystem::path& model_name) override;
		void set_mesh_index(uint idx) override;
		void set_skin_index(uint idx) override;

		void set_cast_shadow(bool v) override;

		void apply_src();

		void draw(sRendererPtr renderer, bool shadow_pass);

		void on_active() override;
		void on_inactive() override;
	};
}
