#ifdef USE_PHYSICS_MODULE

#include "../../foundation/bitmap.h"
#include "../../graphics/image.h"
#include "../../graphics/model.h"
#include "../../graphics/model.h"
#include "../../physics/device.h"
#include "../../physics/material.h"
#include "../../physics/rigid.h"
#include "../../physics/shape.h"
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

	void cShapePrivate::set_static_friction(float v)
	{
		static_friction = v;
	}

	void cShapePrivate::set_dynamic_friction(float v)
	{
		dynamic_friction = v;
	}

	void cShapePrivate::set_restitution(float v)
	{
		restitution = v;
	}

	void cShapePrivate::set_trigger(bool v)
	{
		trigger = v;
	}

	void cShapePrivate::on_added()
	{
		node = entity->get_component_i<cNodePrivate>(0);
		assert(node);

		c_mesh = entity->get_component_t<cMeshPrivate>();
		c_terrain = entity->get_component_t<cTerrainPrivate>();
	}

	void cShapePrivate::on_removed()
	{
		node = nullptr;
	}

	void cShapePrivate::on_entered_world()
	{
		auto e = entity;
		while (e)
		{
			rigid = e->get_component_t<cRigidPrivate>();
			if (rigid)
				break;
			e = e->parent;
		}
		assert(rigid);

		node->update_transform();

		auto material = physics::Material::get(nullptr, static_friction, dynamic_friction, restitution);
		phy_shape = nullptr;
		switch (type)
		{
		case physics::ShapeBox:
			phy_shape = physics::Shape::create_box(nullptr, material, size * node->g_scl);
			break;
		case physics::ShapeSphere:
			phy_shape = physics::Shape::create_sphere(nullptr, material, size.x * node->g_scl.x);
			break;
		case physics::ShapeCapsule:
			phy_shape = physics::Shape::create_capsule(nullptr, material, size.x * node->g_scl.x, size.y * 0.5f * node->g_scl.y);
			break;
		case physics::ShapeTriangleMesh:
			if (c_mesh && c_mesh->mesh)
			{
				auto m = c_mesh->mesh;
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
					triangle_mesh = physics::TriangleMesh::create(nullptr, m);
					triangle_meshes.emplace_back(m, triangle_mesh, 1);
				}
				phy_shape = physics::Shape::create_triangle_mesh(nullptr, material, triangle_mesh, node->g_scl.x);
				phy_triangle_mesh = triangle_mesh;
			}
			break;
		case physics::ShapeHeightField:
			if (c_terrain && c_terrain->height_texture)
			{
				auto t = c_terrain->height_texture;
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
					height_field = physics::HeightField::create(nullptr, t, uvec2(c_terrain->blocks), c_terrain->tess_levels);
					height_fields.emplace_back(t, height_field, 1);
				}
				auto ext = c_terrain->extent;
				phy_shape = physics::Shape::create_height_field(nullptr, material, height_field, vec3(ext.x, ext.y, ext.x));
				phy_height_field = height_field;
			}
			break;
		}

		if (phy_shape)
		{
			phy_shape->set_pose(node->pos, node->get_quat());
			phy_shape->user_data = entity;
			if (trigger)
				phy_shape->set_trigger(true);
			rigid->phy_shapes.push_back(phy_shape);
			rigid->phy_rigid->add_shape(phy_shape);
		}
	}

	void cShapePrivate::on_left_world()
	{
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

		rigid = nullptr;
	}

	cShape* cShape::create()
	{
		return new cShapePrivate();
	}
}

#endif
