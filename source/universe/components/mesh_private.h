#pragma once

#include "mesh.h"

namespace flame
{
	struct cMeshPrivate : cMesh
	{
		cArmaturePtr parmature = nullptr;
		bool dirty = true;

		~cMeshPrivate();
		void on_init() override;

		void set_mesh_name(const std::filesystem::path& mesh_name) override;
		void set_material_name(const std::filesystem::path& material_name) override;

		void set_cast_shadow(bool v) override;
		void set_enable_render(bool v) override;

		void on_active() override;
		void on_inactive() override;
	};
}
