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

#include "shape_private.h"
#include "device_private.h"
#include "material_private.h"

namespace flame
{
	namespace physics
	{
		void Shape::set_trigger(bool v)
		{
			auto s = _priv->v;
			s->setFlag(PxShapeFlag::eSIMULATION_SHAPE, false);
			s->setFlag(PxShapeFlag::eTRIGGER_SHAPE, true);
		}

		Shape *create_box_shape(Device *d, Material *m, const Vec3f &coord,
			float x_hf_ext, float y_hf_ext, float z_hf_ext)
		{
			auto s = new Shape;

			s->_priv = new ShapePrivate;

			s->_priv->v = d->_priv->inst->createShape(PxBoxGeometry(x_hf_ext, y_hf_ext, z_hf_ext),
				*m->_priv->v);
			s->_priv->v->setLocalPose(Z(coord, Vec4f(0.f, 0.f, 0.f, 1.f)));
			s->_priv->v->userData = s;

			return s;
		}

		//void create_sphere_shape(Material *m, const glm::vec3 &coord,
		//	float radius)
		//{
		//	_priv->v->createShape(physx::PxSphereGeometry(scale[0]),
		//		*pxDefaultMaterial, trans);
		//}

		//void create_capsule_shape(Material *m, const glm::vec3 &coord,
		//	float radius, float height)
		//{
		//	_priv->v->createShape(physx::PxCapsuleGeometry(scale[0], scale[1]),
		//		*pxDefaultMaterial, trans * physx::PxTransform(physx::PxQuat(physx::PxHalfPi, physx::PxVec3(0, 0, 1))));
		//}

		void destroy_shape(Shape *s)
		{
			s->_priv->v->release();

			delete s->_priv;
			delete s;
		}
	}
}

