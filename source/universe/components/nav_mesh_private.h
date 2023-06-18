#pragma once

#include "nav_mesh.h"

namespace flame
{
	struct cNavMeshPrivate : cNavMesh
	{
		void set_filename(const std::filesystem::path& filename) override;
	};
}
