#pragma once

#include "../component.h"

namespace flame
{
	struct cOctree : Component
	{
		inline static auto type_name = "flame::cOctree";
		inline static auto type_hash = ch(type_name);

		float octree_length = 0.f;
		uint octree_lod = 0;
	};
}
