#pragma once

#include "../../foundation/blueprint.h"
#include "../universe.h"

namespace flame
{
	extern thread_local EntityPtr bp_self;

	void init_library();
}
