#pragma once

#include "../component.h"

// not physics, just a collider in logic

namespace flame
{
	enum ColliderType
	{
		ColliderCircle,
		ColliderSector
	};

	// Reflect ctor
	struct cCollider : Component
	{
		// Reflect requires
		cNodePtr node = nullptr;

		// Reflect
		ColliderType type = ColliderCircle;
		// Reflect
		float radius = -1.f; // -1 means auto
		// Reflect
		float radius_add = 0.f; // add to auto radius

		// Reflect
		uint any_filter = 0xffffffff;
		// Reflect
		uint all_filter = 0;
		// Reflect
		uint parent_search_times = 3;

		struct Create
		{
			virtual cColliderPtr operator()(EntityPtr e) = 0;
		};
		// Reflect static
		FLAME_UNIVERSE_API static Create& create;
	};
}
