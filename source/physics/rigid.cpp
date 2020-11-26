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
				px_rigid = device->px_instance->createRigidDynamic(PxTransform(PxVec3(0.f)));
				PxRigidBodyExt::updateMassAndInertia(*(PxRigidDynamic*)px_rigid, 10.0);
				//if (kinematic) 
				//	body->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, true);
			}
			else
				px_rigid = device->px_instance->createRigidStatic(PxTransform(PxVec3(0.f)));

			px_rigid->userData = this;
#endif
		}

		RigidPrivate::~RigidPrivate()
		{
#ifdef USE_PHYSX
			px_rigid->release();
#endif
		}

		void RigidPrivate::get_pose(vec3& coord, vec4& quat) const
		{
#ifdef USE_PHYSX
			auto trans = px_rigid->getGlobalPose();
			coord = cvt(trans.p);
			quat = cvt(trans.q);
#endif
		}

		void RigidPrivate::set_pose(const vec3& coord, const vec4& quat)
		{
#ifdef USE_PHYSX
			px_rigid->setGlobalPose(PxTransform(cvt(coord), cvt(quat)));
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

		void RigidPrivate::add_impulse(const vec3& v)
		{
			if (!dynamic)
				return;
#ifdef USE_PHYSX
			((PxRigidDynamic*)px_rigid)->addForce(cvt(v), PxForceMode::eIMPULSE);
#endif
		}

		void RigidPrivate::add_force(const vec3& v)
		{
			if (!dynamic)
				return;
#ifdef USE_PHYSX
			((PxRigidDynamic*)px_rigid)->addForce(cvt(v));
#endif
		}

		void RigidPrivate::clear_force()
		{
			if (!dynamic)
				return;
#ifdef USE_PHYSX
			((PxRigidDynamic*)px_rigid)->clearForce();
#endif
		}

		Rigid* Rigid::create(Device* device, bool dynamic)
		{
			return new RigidPrivate((DevicePrivate*)device, dynamic);
		}
	}
}

