#include <flame/graphics/canvas.h>
#include <flame/graphics/model.h>
#include <flame/physics/material.h>
#include <flame/physics/rigid.h>
#include <flame/physics/shape.h>
#include "../entity_private.h"
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
			material = physics::Material::create(0.2f, 0.2f, 0.3f);
		return material;
	}

	void cShapePrivate::set_type(physics::ShapeType t)
	{
		type = t;
	}

	void cShapePrivate::set_size(const Vec3f& s)
	{
		size = s;
	}

	void cShapePrivate::set_trigger(bool v)
	{
		trigger = v;
	}

	void cShapePrivate::on_local_message(Message msg, void* p)
	{
		switch (msg)
		{
		case MessageEnteredWorld:
			node->update_transform();

			physics::ShapeDesc desc;
			switch (type)
			{
			case physics::ShapeCube:
				desc.box.hf_ext = size * Vec3f(0.5f) * node->global_scale;
				phy_shape = physics::Shape::create(get_material(), physics::ShapeCube, desc);
				break;
			case physics::ShapeSphere:
				desc.sphere.radius = size.x() * 0.5f * node->global_scale.x();
				phy_shape = physics::Shape::create(get_material(), physics::ShapeSphere, desc);
				break;
			case physics::ShapeMesh:
				if (mesh && mesh->mesh)
				{
					desc.mesh.mesh = mesh->mesh;
					desc.mesh.scale = size * node->global_scale;
					phy_shape = physics::Shape::create(get_material(), physics::ShapeMesh, desc);
				}
				break;
			}

			if (phy_shape)
			{
				phy_shape->user_data = this;
				if (trigger)
					phy_shape->set_trigger(true);
				rigid->phy_shapes.push_back(phy_shape);
				rigid->phy_rigid->add_shape(phy_shape);
			}
			break;
		case MessageLeftWorld:
			if (phy_shape)
			{
				std::erase_if(rigid->phy_shapes, [&](const auto& i) {
					return i == phy_shape;
				});
				phy_shape->release();
				phy_shape = nullptr;
			}
			break;
		}
	}

	cShape* cShape::create()
	{
		return new cShapePrivate();
	}
}
