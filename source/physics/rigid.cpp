#include "physics_private.h"
#include "device_private.h"
#include "rigid_private.h"
#include "shape_private.h"

namespace flame
{
	namespace physics
	{
		RigidPrivate::~RigidPrivate()
		{
#ifdef USE_PHYSX
			v->release();
#endif
		}

		void Rigid::attach_shape(Shape* s)
		{
			v->attachShape(*s->v);
		}

		void Rigid::detach_shape(Shape* s)
		{
			v->detachShape(*s->v);
		}

		void Rigid::get_pose(Vec3f& out_coord, Vec4f& out_quat)
		{
			auto trans = v->getGlobalPose();
			out_coord.x() = trans.p.x();
			out_coord.y() = trans.p.y();
			out_coord.z() = trans.p.z();
			out_quat.x() = trans.q.x();
			out_quat.y() = trans.q.y();
			out_quat.z() = trans.q.z();
			out_quat.w() = trans.q.w();
		}

		void Rigid::add_force(const Vec3f& v)
		{
			auto d = (PxRigidDynamic*)v;
			d->addForce(Z(v));
		}

		void Rigid::clear_force()
		{
			auto d = (PxRigidDynamic*)v;
			d->clearForce();
		}

		Rigid* create_static_rigid(Device* d, const Vec3f& coord)
		{
			r->v = d->inst->createRigidStatic(
				Z(coord, Vec4f(0.f, 0.f, 0.f, 1.f)));
			r->v->userData = r;

			return r;
		}

		Rigid* create_dynamic_rigid(Device* d, const Vec3f& coord)
		{
			r->v = d->inst->createRigidDynamic(
				Z(coord, Vec4f(0.f, 0.f, 0.f, 1.f)));
			//PxRigidBodyExt::updateMassAndInertia(*body, density);
			//if (kinematic) body->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, true);
			r->v->userData = r;

			return r;
		}
	}
}

