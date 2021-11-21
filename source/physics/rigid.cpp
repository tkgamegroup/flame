#include "physics_private.h"
#include "device_private.h"
#include "rigid_private.h"
#include "shape_private.h"

namespace flame
{
	namespace physics
	{
		RigidPrivate::RigidPrivate(DevicePrivate* _device, bool dynamic) :
			device(_device),
			dynamic(dynamic)
		{
			if (!device)
				device = current_device;

			if (dynamic)
			{
				px_rigid.reset(device->px_instance->createRigidDynamic(PxTransform(PxVec3(0.f))));
				PxRigidBodyExt::updateMassAndInertia(*(PxRigidDynamic*)px_rigid.get(), 10.0);
				//if (kinematic) 
				//	body->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, true);
			}
			else
				px_rigid.reset(device->px_instance->createRigidStatic(PxTransform(PxVec3(0.f))));

			px_rigid->setName("Rigid");
			px_rigid->userData = this;
		}

		bool RigidPrivate::is_sleeping() const
		{
			if (dynamic)
				return ((PxRigidDynamic*)px_rigid.get())->isSleeping();
			return false;
		}

		void RigidPrivate::get_pose(vec3& coord, quat& qut) const
		{
			auto trans = px_rigid->getGlobalPose();
			coord = cvt(trans.p);
			qut = cvt(trans.q);
		}

		void RigidPrivate::set_pose(const vec3& coord, const quat& qut)
		{
			px_rigid->setGlobalPose(PxTransform(cvt(coord), cvt(qut)));
		}

		void RigidPrivate::add_shape(ShapePtr s)
		{
			s->rigid = this;
			px_rigid->attachShape(*s->px_shape);
		}

		void RigidPrivate::remove_shape(ShapePtr s)
		{
			px_rigid->detachShape(*s->px_shape);
			s->rigid = nullptr;
		}

		void RigidPrivate::add_impulse(const vec3& v)
		{
			if (!dynamic)
				return;
			((PxRigidDynamic*)px_rigid.get())->addForce(cvt(v), PxForceMode::eIMPULSE);
		}

		void RigidPrivate::add_force(const vec3& v)
		{
			if (!dynamic)
				return;
			((PxRigidDynamic*)px_rigid.get())->addForce(cvt(v));
		}

		void RigidPrivate::clear_force()
		{
			if (!dynamic)
				return;
			((PxRigidDynamic*)px_rigid.get())->clearForce();
		}

		Rigid* Rigid::create(Device* device, bool dynamic)
		{
			return new RigidPrivate((DevicePrivate*)device, dynamic);
		}
	}
}

