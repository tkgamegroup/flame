#pragma once

#ifdef FLAME_PHYSICS_MODULE
#define FLAME_PHYSICS_EXPORTS __declspec(dllexport)
#else
#define FLAME_PHYSICS_EXPORTS __declspec(dllimport)
#endif

#include <flame/math.h>

namespace flame
{
	namespace graphics
	{
		struct Mesh;
	}

	namespace physics
	{
		enum TouchType
		{
			TouchFound,
			TouchLost
		};

		enum ShapeType
		{
			ShapeCube,
			ShapeSphere,
			ShapeCapsule,
			ShapeMesh
		};

		union ShapeDesc
		{
			struct
			{
				Vec3f hf_ext;
			}box;
			struct
			{
				float radius;
			}sphere;
			struct
			{
				float radius;
				float height;
			}capsule;
			struct
			{
				graphics::Mesh* mesh;
				Vec3f scale;
			}mesh;
		};
	}
}

