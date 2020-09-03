#include <flame/physics/material.h>
#include <flame/physics/rigid.h>
#include <flame/physics/shape.h>
#include "node_private.h"
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

	void cShapePrivate::on_gain_rigid()
	{
		physics::ShapeDesc desc;
		desc.box.hf_ext = Vec3f(0.5f) * node->scale;
		shape = physics::Shape::create(get_material(), physics::ShapeBox, desc, Vec3f(0.f));
		rigid->rigid->add_shape(shape);
	}

	void cShapePrivate::on_lost_rigid()
	{
		shape->release();
		shape = nullptr;
	}

	cShape* cShape::create()
	{
		return new cShapePrivate();
	}
}
