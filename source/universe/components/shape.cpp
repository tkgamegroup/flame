#include <flame/graphics/canvas.h>
#include <flame/physics/material.h>
#include <flame/physics/rigid.h>
#include <flame/physics/shape.h>
#include "node_private.h"
#include "object_private.h"
#include "rigid_private.h"
#include "shape_private.h"

namespace flame
{
	static physics::Material* material = nullptr;
	physics::Material* get_material()
	{
		if (!material)
			material = physics::Material::create(0.5f, 0.5f, 0.6f);
		return material;
	}

	void cShapePrivate::set_type(physics::ShapeType t)
	{
		type = t;
	}

	void cShapePrivate::make_shape()
	{
		physics::ShapeDesc desc;
		switch (type)
		{
		case physics::ShapeBox:
			desc.box.hf_ext = Vec3f(0.5f) * node->scale;
			shape = physics::Shape::create(get_material(), physics::ShapeBox, desc);
			rigid->rigid->add_shape(shape);
			break;
		case physics::ShapeSphere:
			desc.sphere.radius = 0.5f * node->scale.x();
			shape = physics::Shape::create(get_material(), physics::ShapeSphere, desc);
			rigid->rigid->add_shape(shape);
			break;
		case physics::ShapeTriangles:
			if (object->model_idx != -1)
			{
				desc.triangles.model = object->canvas->get_model(object->model_idx);
				desc.triangles.scale = node->scale;
				shape = physics::Shape::create(get_material(), physics::ShapeTriangles, desc);
				rigid->rigid->add_shape(shape);
			}
			break;
		}
	}

	void cShapePrivate::on_gain_rigid()
	{
		if (type != physics::ShapeTriangles)
			make_shape();
	}

	void cShapePrivate::on_lost_rigid()
	{
		shape->release();
		shape = nullptr;
	}

	void cShapePrivate::on_local_data_changed(Component* t, uint64 h)
	{
		if (t == object && h == S<ch("model_idx")>::v)
		{
			if (type == physics::ShapeTriangles)
				make_shape();
		}
	}

	cShape* cShape::create()
	{
		return new cShapePrivate();
	}
}
