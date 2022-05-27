#pragma once

#include "environment.h"

namespace flame
{
	struct cEnvironmentPrivate : cEnvironment
	{
		void set_sky_map_name(const std::filesystem::path& name) override;
	};
}
