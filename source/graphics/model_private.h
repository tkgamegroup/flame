#pragma once

#include "model.h"

namespace flame
{
	namespace graphics
	{
		struct MeshPrivate : Mesh
		{
			void save(const std::filesystem::path& filename) override;
		};

		struct ArmaturePrivate : Armature
		{
			void save(const std::filesystem::path& filename) override;
		};
	}
}
