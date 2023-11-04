#pragma once

#include "procedure_mesh.h"

namespace flame
{
	struct cProcedureMeshPrivate : cProcedureMesh
	{
		~cProcedureMeshPrivate();

		void set_blueprint_name(const std::filesystem::path& name) override;
	};
}
