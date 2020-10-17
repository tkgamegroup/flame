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
		struct Image;
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
			ShapeTriangleMesh,
			ShapeHeightField
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
			}triangle_mesh;
			struct
			{
				graphics::Image* height_map;
				Vec2u tess;
				Vec3f scale;
			}height_field;
		};
	}
}

