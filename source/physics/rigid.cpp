#include "physics_private.h"
#include "device_private.h"
#include "rigid_private.h"
#include "shape_private.h"

namespace flame
{
	namespace physics
	{
		RigidPrivate::RigidPrivate(DevicePrivate* device, bool dynamic) :
			dynamic(dynamic)
		{
#ifdef USE_PHYSX
			if (dynamic)
			{
				px_rigid.reset(device->px_instance->createRigidDynamic(PxTransform(PxVec3(0.f))));
				PxRigidBodyExt::updateMassAndInertia(*(PxRigidDynamic*)px_rigid.get(), 10.0);
				//if (kinematic) 
				//	body->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, true);
			}
			else
				px_rigid.reset(device->px_instance->createRigidStatic(PxTransform(PxVec3(0.f))));

			px_rigid->userData = this;
#endif
		}

		void RigidPrivate::get_pose(vec3& coord, quat& qut) const
		{
#ifdef USE_PHYSX
			auto trans = px_rigid->getGlobalPose();
			coord = cvt(trans.p);
			qut = cvt(trans.q);
#endif
		}

		void RigidPrivate::set_pose(const vec3& coord, const quat& qut)
		{
#ifdef USE_PHYSX
			px_rigid->setGlobalPose(PxTransform(cvt(coord), cvt(qut)));
#endif
		}

		void RigidPrivate::add_shape(ShapePtr s)
		{
#ifdef USE_PHYSX
			s->rigid = this;
			px_rigid->attachShape(*s->px_shape);
#endif
		}

		void RigidPrivate::remove_shape(ShapePtr s)
		{
#ifdef USE_PHYSX
			px_rigid->detachShape(*s->px_shape);
			s->rigid = nullptr;
#endif
		}

		void RigidPrivate::add_impulse(const vec3& v)
		{
			if (!dynamic)
				return;
#ifdef USE_PHYSX
			((PxRigidDynamic*)px_rigid.get())->addForce(cvt(v), PxForceMode::eIMPULSE);
#endif
		}

		void RigidPrivate::add_force(const vec3& v)
		{
			if (!dynamic)
				return;
#ifdef USE_PHYSX
			((PxRigidDynamic*)px_rigid.get())->addForce(cvt(v));
#endif
		}

		void RigidPrivate::clear_force()
		{
			if (!dynamic)
				return;
#ifdef USE_PHYSX
			((PxRigidDynamic*)px_rigid.get())->clearForce();
#endif
		}

		Rigid* Rigid::create(Device* device, bool dynamic)
		{
			return new RigidPrivate((DevicePrivate*)device, dynamic);
		}
	}
}

