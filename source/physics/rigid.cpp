#include "physics_private.h"
#include "device_private.h"
#include "rigid_private.h"
#include "shape_private.h"

namespace flame
{
	namespace physics
	{
		RigidPrivate::RigidPrivate(bool dynamic)
		{
#ifdef USE_PHYSX
			if (dynamic)
			{
				px_rigid = DevicePrivate::get()->px_instance->createRigidDynamic(physx::PxTransform(physx::PxVec3(0.f)));
				physx::PxRigidBodyExt::updateMassAndInertia(*(physx::PxRigidDynamic*)px_rigid, 10.0);
				//if (kinematic) 
				//	body->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, true);
			}
			else
				px_rigid = DevicePrivate::get()->px_instance->createRigidStatic(physx::PxTransform(physx::PxVec3(0.f)));

			px_rigid->userData = this;
#endif
		}

		RigidPrivate::~RigidPrivate()
		{
#ifdef USE_PHYSX
			px_rigid->release();
#endif
		}

		void RigidPrivate::get_pose(Vec3f& coord, Vec4f& quat) const
		{
#ifdef USE_PHYSX
			auto trans = px_rigid->getGlobalPose();
			coord = Vec3f(trans.p.x, trans.p.y, trans.p.z);
			quat = Vec4f(trans.q.x, trans.q.y, trans.q.z, trans.q.w);
#endif
		}

		void RigidPrivate::set_pose(const Vec3f& coord, const Vec4f& quat)
		{
#ifdef USE_PHYSX
			px_rigid->setGlobalPose(physx::PxTransform(
				physx::PxVec3(coord.x(), coord.y(), coord.z()),
				physx::PxQuat(quat.x(), quat.y(), quat.z(), quat.w())));
#endif
		}

		void RigidPrivate::add_shape(ShapePrivate* s)
		{
#ifdef USE_PHYSX
			s->rigid = this;
			px_rigid->attachShape(*s->px_shape);
#endif
		}

		void RigidPrivate::remove_shape(ShapePrivate* s)
		{
#ifdef USE_PHYSX
			px_rigid->detachShape(*s->px_shape);
			s->rigid = nullptr;
#endif
		}

		void RigidPrivate::add_force(const Vec3f& v)
		{
#ifdef USE_PHYSX
			((physx::PxRigidDynamic*)px_rigid)->addForce(physx::PxVec3(v.x(), v.y(), v.z()));
#endif
		}

		void RigidPrivate::clear_force()
		{
#ifdef USE_PHYSX
			((physx::PxRigidDynamic*)px_rigid)->clearForce();
#endif
		}

		Rigid* Rigid::create(bool dynamic)
		{
			return new RigidPrivate(dynamic);
		}
	}
}

