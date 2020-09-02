#include "shape_private.h"
#include "device_private.h"
#include "material_private.h"

namespace flame
{
	namespace physics
	{
		ShapePrivate::ShapePrivate(DevicePrivate* d, MaterialPrivate* m, ShapeType type, const ShapeDesc& desc, const Vec3f& coord)
		{
#ifdef USE_PHYSX
			switch (type)
			{
			case ShapeBox:
				px_shape = d->px_instance->createShape(physx::PxBoxGeometry(desc.box.hf_ext.x(), desc.box.hf_ext.y(), desc.box.hf_ext.z()), *m->px_material);
				px_shape->setLocalPose(physx::PxTransform(physx::PxVec3(coord.x(), coord.y(), coord.z())));
				break;
			case ShapeSphere:
				px_shape = d->px_instance->createShape(physx::PxSphereGeometry(desc.sphere.radius), *m->px_material);
				px_shape->setLocalPose(physx::PxTransform(physx::PxVec3(coord.x(), coord.y(), coord.z())));
				break;
			case ShapeCapsule:
				px_shape = d->px_instance->createShape(physx::PxCapsuleGeometry(desc.capsule.radius, desc.capsule.height), *m->px_material);
				px_shape->setLocalPose(physx::PxTransform(physx::PxVec3(coord.x(), coord.y(), coord.z()), physx::PxQuat(physx::PxHalfPi, physx::PxVec3(0, 0, 1))));
				break;
			default:
				assert(0);
			}
			px_shape->userData = this;
#endif
		}

		ShapePrivate::~ShapePrivate()
		{
#ifdef USE_PHYSX
			px_shape->release();
#endif
		}

		void ShapePrivate::set_trigger(bool v)
		{
#ifdef USE_PHYSX
			px_shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, false);
			px_shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, true);
#endif
		}

		Shape* Shape::create(Device* d, Material* m, ShapeType type, const ShapeDesc& desc, const Vec3f& coord)
		{
			return new ShapePrivate((DevicePrivate*)d, (MaterialPrivate*)m, type, desc, coord);
		}
	}
}

