#pragma once

#include "procedure_mesh.h"

namespace flame
{
	struct cProcedureMeshPrivate : cProcedureMesh
	{
		~cProcedureMeshPrivate();

		void start() override;
	};
}
