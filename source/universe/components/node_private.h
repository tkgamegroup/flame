#pragma once

#include <flame/universe/components/node.h>

namespace flame
{
	struct cNodePrivate : cNode // R ~ on_*
	{
		Vec3f pos = Vec3f(0.f);
		Vec4f quat = Vec4f(0.f, 0.f, 0.f, 1.f);
		Vec3f scale = Vec3f(1.f);
	};
}
