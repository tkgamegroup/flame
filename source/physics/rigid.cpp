//MIT License
//
//Copyright (c) 2018 wjs
//
//Permission is hereby granted, free of charge, to any person obtaining a copy
//of this software and associated documentation files (the "Software"), to deal
//in the Software without restriction, including without limitation the rights
//to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//copies of the Software, and to permit persons to whom the Software is
//furnished to do so, subject to the following conditions:
//
//The above copyright notice and this permission notice shall be included in all
//copies or substantial portions of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//SOFTWARE.

#include "physics_private.h"
#include "device_private.h"
#include "rigid_private.h"
#include "shape_private.h"

namespace flame
{
	namespace physics
	{
		void Rigid::attach_shape(Shape *s)
		{
			_priv->v->attachShape(*s->_priv->v);
		}

		void Rigid::detach_shape(Shape *s)
		{
			_priv->v->detachShape(*s->_priv->v);
		}

		void Rigid::get_pose(Vec3 &out_coord, Vec4 &out_quat)
		{
			auto trans = _priv->v->getGlobalPose();
			out_coord.x() = trans.p.x();
			out_coord.y() = trans.p.y();
			out_coord.z() = trans.p.z();
			out_quat.x() = trans.q.x();
			out_quat.y() = trans.q.y();
			out_quat.z() = trans.q.z();
			out_quat.w() = trans.q.w();
		}

		void Rigid::add_force(const Vec3 &v)
		{
			auto d = (PxRigidDynamic*)_priv->v;
			d->addForce(Z(v));
		}

		void Rigid::clear_force()
		{
			auto d = (PxRigidDynamic*)_priv->v;
			d->clearForce();
		}
		
		Rigid *create_static_rigid(Device *d, const Vec3 &coord)
		{
			auto r = new Rigid;
			
			r->_priv = new RigidPrivate;
			r->_priv->v = d->_priv->inst->createRigidStatic(
				Z(coord, Vec4(0.f, 0.f, 0.f, 1.f)));
			r->_priv->v->userData = r;

			return r;
		}

		Rigid *create_dynamic_rigid(Device *d, const Vec3 &coord)
		{
			auto r = new Rigid;

			r->_priv = new RigidPrivate;
			r->_priv->v = d->_priv->inst->createRigidDynamic(
				Z(coord, Vec4(0.f, 0.f, 0.f, 1.f)));
			//PxRigidBodyExt::updateMassAndInertia(*body, density);
			//if (kinematic) body->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, true);
			r->_priv->v->userData = r;

			return r;
		}

		void destroy_rigid(Rigid *r)
		{
			r->_priv->v->release();
			delete r->_priv;
			delete r;
		}
	}
}

