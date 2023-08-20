#pragma once

#include "../universe_private.h"
#include "bp_instance.h"

namespace flame
{
	struct cBpInstancePrivate : cBpInstance
	{
		void set_bp_name(const std::filesystem::path& bp_name) override;
	};
}
