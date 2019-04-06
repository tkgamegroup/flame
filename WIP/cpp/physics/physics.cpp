#include <flame/engine/physics/physics.h>

#if FLAME_ENABLE_PHYSICS != 0

namespace flame
{
	glm::vec3 physx_u32_to_vec3(const physx::PxU32 &src)
	{
		unsigned int t = src;
		float r = (t % 256) / 255.f;
		t /= 256;
		float g = (t % 256) / 255.f;
		t /= 256;
		float b = (t % 256) / 255.f;
		return glm::vec3(r, g, b);
	}

	std::string shapeTypeName(ShapeType t)
	{
		char *names[] = {
			"Box",
			"Sphere",
			"Capsule",
			"Plane",
			"Convex Mesh",
			"Triangle Mesh",
			"Height Field"
		};
		return names[(int)t];
	}

	float Shape::getVolume() const
	{
		switch (type)
		{
		case ShapeType::box:
			return scale.x * scale.y * scale.z * 8.f;
		case ShapeType::sphere:
			return 4.f * scale.x * scale.x * scale.x * M_PI / 3.f;
		case ShapeType::capsule:
			return 4.f * scale.x * scale.x * scale.x * M_PI / 3.f + M_PI * scale.x * scale.x * scale.y;
		}
		return 0.f;
	}

	Shape *Rigidbody::new_shape()
	{
		auto s = new Shape;
		shapes.emplace_back(s);
		return s;
	}

	void Rigidbody::remove_shape(Shape *s)
	{
		for (auto it = shapes.begin(); it != shapes.end(); it++)
		{
			if (it->get() == s)
			{
				shapes.erase(it);
				return;
			}
		}
	}

	physx::PxMaterial *pxDefaultMaterial = nullptr;

	void createPhysicsScene()
	{
		//	auto group1ID = 1;
		//	for (auto g : scene->pCollisionGroups)
		//	{
		//		for (int i = 0; i < 16; i++)
		//		{
		//			if ((g->originalmask & (1 << i)) == 0)
		//			{
		//				auto group2ID = 1;
		//				for (auto h : scene->pCollisionGroups)
		//				{
		//					if (h->originalID == i)
		//					{
		//						PxSetGroupCollisionFlag(group1ID, group2ID, false);
		//					}
		//					group2ID++;
		//				}
		//			}
		//		}
		//		group1ID++;
		//	}

		//	//PxSetGroupCollisionFlag(3, 3, false);
		//	//PxSetGroupCollisionFlag(3, 4, false);
		//	//PxSetGroupCollisionFlag(4, 4, false);
		//	//for (int i = 0; i < 100; i++)
		//	//	for (int j = 0; j < 100; j++)
		//	//		PxSetGroupCollisionFlag(i + 1, j + 1, false);
		//}
	}

	//void destoryPhysicsScene()
	//{
	//	pxControllerManager->release();
	//	pxScene->release();
	//}
}

#endif