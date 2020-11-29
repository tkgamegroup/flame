#include <flame/foundation/bitmap.h>
#include <flame/graphics/image.h>
#include <flame/graphics/model.h>
#include <flame/graphics/canvas.h>
#include <flame/physics/device.h>
#include <flame/physics/material.h>
#include <flame/physics/rigid.h>
#include <flame/physics/shape.h>
#include "../entity_private.h"
#include "node_private.h"
#include "mesh_private.h"
#include "terrain_private.h"
#include "rigid_private.h"
#include "shape_private.h"

namespace flame
{
	static std::vector<std::tuple<graphics::Mesh*, physics::TriangleMesh*, uint>> triangle_meshes;
	static std::vector<std::tuple<graphics::Image*, physics::HeightField*, uint>> height_fields;

	void cShapePrivate::set_type(physics::ShapeType t)
	{
		type = t;
	}

	void cShapePrivate::set_size(const vec3& s)
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

			switch (type)
			{
			case physics::ShapeCube:
				phy_shape = physics::Shape::create_box(physics::Device::get_default(), nullptr, size * vec3(0.5f) * node->g_scl);
				break;
			case physics::ShapeSphere:
				phy_shape = physics::Shape::create_sphere(physics::Device::get_default(), nullptr, size.x * 0.5f * node->g_scl.x);
				break;
			case physics::ShapeTriangleMesh:
				if (mesh && mesh->mesh)
				{
					auto m = mesh->mesh;
					physics::TriangleMesh* triangle_mesh = nullptr;
					for (auto& t : triangle_meshes)
					{
						if (std::get<0>(t) == m)
						{
							triangle_mesh = std::get<1>(t);
							std::get<2>(t)++;
							break;
						}
					}
					if (!triangle_mesh)
					{
						triangle_mesh = physics::TriangleMesh::create(physics::Device::get_default(), m);
						triangle_meshes.emplace_back(m, triangle_mesh, 1);
					}
					phy_shape = physics::Shape::create_triangle_mesh(physics::Device::get_default(), nullptr, triangle_mesh, size.x * node->g_scl.x);
					phy_triangle_mesh = triangle_mesh;
				}
				break;
			case physics::ShapeHeightField:
				if (terrain && terrain->height_map_id != -1)
				{
					auto t = terrain->canvas->get_texture_resource(terrain->height_map_id)->get_image();
					physics::HeightField* height_field = nullptr;
					for (auto& h : height_fields)
					{
						if (std::get<0>(h) == t)
						{
							height_field = std::get<1>(h);
							std::get<2>(h)++;
							break;
						}
					}
					if (!height_field)
					{
						height_field = physics::HeightField::create(physics::Device::get_default(), t, terrain->blocks, terrain->tess_levels);
						height_fields.emplace_back(t, height_field, 1);
					}
					phy_shape = physics::Shape::create_height_field(physics::Device::get_default(), nullptr, height_field, node->g_scl * terrain->scale);
					phy_height_field = height_field;
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
				if (phy_triangle_mesh)
				{
					for (auto it = triangle_meshes.begin(); it != triangle_meshes.end(); it++)
					{
						if (std::get<1>(*it) == phy_triangle_mesh)
						{
							std::get<2>(*it)--;
							if (!std::get<2>(*it))
								triangle_meshes.erase(it);
							break;
						}
					}
					phy_triangle_mesh = nullptr;
				}
				if (phy_height_field)
				{
					for (auto it = height_fields.begin(); it != height_fields.end(); it++)
					{
						if (std::get<1>(*it) == phy_height_field)
						{
							std::get<2>(*it)--;
							if (!std::get<2>(*it))
								height_fields.erase(it);
							break;
						}
					}
					phy_height_field = nullptr;
				}
			}
			break;
		}
	}

	cShape* cShape::create()
	{
		return new cShapePrivate();
	}
}
