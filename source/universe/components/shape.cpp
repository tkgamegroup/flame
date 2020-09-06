#include <flame/graphics/canvas.h>
#include <flame/graphics/model.h>
#include <flame/physics/material.h>
#include <flame/physics/rigid.h>
#include <flame/physics/shape.h>
#include "node_private.h"
#include "mesh_instance_private.h"
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

	void cShapePrivate::on_gain_rigid()
	{
		physics::ShapeDesc desc;
		switch (type)
		{
		case physics::ShapeBox:
			desc.box.hf_ext = Vec3f(0.5f) * node->scale;
			phy_shape = physics::Shape::create(get_material(), physics::ShapeBox, desc);
			rigid->phy_rigid->add_shape(phy_shape);
			break;
		case physics::ShapeSphere:
			desc.sphere.radius = 0.5f * node->scale.x();
			phy_shape = physics::Shape::create(get_material(), physics::ShapeSphere, desc);
			rigid->phy_rigid->add_shape(phy_shape);
			break;
		case physics::ShapeTriangles:
			//if (rigid->mesh->model_idx != -1)
			//{
			//	desc.triangles.model = object->canvas->get_model(object->model_idx);
			//	desc.triangles.scale = node->scale;
			//	phy_shapes[0] = physics::Shape::create(get_material(), physics::ShapeTriangles, desc);
			//	rigid->phy_rigid->add_shape(phy_shapes[0]);

			//	if (has_triggers)
			//	{
			//		phy_shapes[1]->set_trigger(true);
			//		rigid->phy_rigid->add_shape(phy_shapes[1]);
			//	}
			//}
			break;
		}
	}

	void cShapePrivate::on_lost_rigid()
	{
		phy_shape->release();
		phy_shape = nullptr;
	}

	cShape* cShape::create()
	{
		return new cShapePrivate();
	}
}
