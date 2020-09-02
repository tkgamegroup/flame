#include "physics_private.h"
#include "device_private.h"
#include "rigid_private.h"
#include "shape_private.h"

namespace flame
{
	namespace physics
	{
		RigidPrivate::RigidPrivate(DevicePrivate* d, const Vec3f& coord, bool dynamic)
		{
#ifdef USE_PHYSX
			if (dynamic)
			{
				px_rigid = d->px_instance->createRigidDynamic(physx::PxTransform(physx::PxVec3(coord.x(), coord.y(), coord.z())));
				physx::PxRigidBodyExt::updateMassAndInertia(*(physx::PxRigidDynamic*)px_rigid, 10.0);
				//if (kinematic) 
				//	body->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, true);
			}
			else
				px_rigid = d->px_instance->createRigidStatic(physx::PxTransform(physx::PxVec3(coord.x(), coord.y(), coord.z())));

			px_rigid->userData = this;
#endif
		}

		RigidPrivate::~RigidPrivate()
		{
#ifdef USE_PHYSX
			px_rigid->release();
#endif
		}

		void RigidPrivate::add_shape(ShapePrivate* s)
		{
#ifdef USE_PHYSX
			px_rigid->attachShape(*s->px_shape);
			s->rigid = this;
#endif
		}

		void RigidPrivate::remove_shape(ShapePrivate* s)
		{
#ifdef USE_PHYSX
			s->rigid = nullptr;
			px_rigid->detachShape(*s->px_shape);
#endif
		}

		void RigidPrivate::get_pose(Vec3f& out_coord, Vec4f& out_quat)
		{
#ifdef USE_PHYSX
			auto trans = px_rigid->getGlobalPose();
			out_coord.x() = trans.p.x;
			out_coord.y() = trans.p.y;
			out_coord.z() = trans.p.z;
			out_quat.x() = trans.q.x;
			out_quat.y() = trans.q.y;
			out_quat.z() = trans.q.z;
			out_quat.w() = trans.q.w;
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

		Rigid* Rigid::create(Device* d, const Vec3f& coord, bool dynamic)
		{
			return new RigidPrivate((DevicePrivate*)d, coord, dynamic);
		}
	}
}

