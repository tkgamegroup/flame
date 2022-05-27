#include "environment_private.h"

namespace flame
{
	void cEnvironmentPrivate::set_sky_map_name(const std::filesystem::path& name)
	{

	}

	struct cEnvironmentCreate : cEnvironment::Create
	{
		cEnvironmentPtr operator()() override
		{
			return new cEnvironmentPrivate();
		}
	}cEnvironment_create;
	cEnvironment::Create& cEnvironment::create = cEnvironment_create;
}
