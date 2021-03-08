#include <flame/graphics/device.h>
#include <flame/graphics/buffer.h>
#include <flame/graphics/image.h>
#include <flame/graphics/model.h>
#include "shape_private.h"
#include "device_private.h"
#include "material_private.h"

namespace flame
{
	namespace physics
	{
		const auto height_field_precision = 30000.f;

		TriangleMeshPrivate::TriangleMeshPrivate(DevicePrivate* device, graphics::Mesh* mesh)
		{
#ifdef USE_PHYSX
			PxTolerancesScale scale;
			PxCookingParams params(scale);

			device->px_cooking->setParams(params);

			std::vector<PxVec3> vertices;
			std::vector<PxU32> indices;
			{
				vertices.resize(mesh->get_vertices_count());
				indices.resize(mesh->get_indices_count());
				auto ps = mesh->get_positions();
				for (auto i = 0; i < vertices.size(); i++)
					vertices[i] = cvt(ps[i]);
				auto is = mesh->get_indices();
				for (auto i = 0; i < indices.size(); i++)
					indices[i] = is[i];
			}

			PxTriangleMeshDesc mesh_desc;
			mesh_desc.points.count = vertices.size();
			mesh_desc.points.stride = sizeof(PxVec3);
			mesh_desc.points.data = vertices.data();

			mesh_desc.triangles.count = indices.size() / 3;
			mesh_desc.triangles.stride = 3 * sizeof(PxU32);
			mesh_desc.triangles.data = indices.data();
			px_triangle_mesh.reset(device->px_cooking->createTriangleMesh(mesh_desc, device->px_instance->getPhysicsInsertionCallback()));
#endif
		}

		TriangleMesh* TriangleMesh::create(Device* device, graphics::Mesh* mesh)
		{
			return new TriangleMeshPrivate((DevicePrivate*)device, mesh);
		}

		HeightFieldPrivate::HeightFieldPrivate(DevicePrivate* device, graphics::Image* height_map, const uvec2& blocks, uint tess_levels) :
			tess_levels(tess_levels)
		{
#ifdef USE_PHYSX
			auto w = blocks.x * tess_levels;
			auto h = blocks.y * tess_levels;
			std::vector<uint> samples((w + 1) * (h + 1));
			std::vector<vec2> uvs(samples.size());
			std::vector<vec4> res(samples.size());
			auto idx = 0;
			for (auto x = 0; x <= w; x++)
			{
				for (auto y = 0; y <= h; y++)
					uvs[idx++] = vec2(x / (float)w, y / (float)h);
			}
			height_map->get_samples(uvs.size(), uvs.data(), res.data());
			auto dst = (PxHeightFieldSample*)samples.data();
			auto lvhf = tess_levels >> 1;
			idx = 0;
			for (auto x = 0; x <= w; x++)
			{
				for (auto y = 0; y <= h; y++)
				{
					dst->height = res[idx++][0] * height_field_precision;

					dst->materialIndex0 = 0;
					dst->materialIndex1 = 0;
					auto s1 = x % tess_levels < lvhf ? 1 : -1;
					auto s2 = y % tess_levels < lvhf ? 1 : -1;
					if (s1 * s2 > 0)
						dst->setTessFlag();
					else
						dst->clearTessFlag();
					dst++;
				}
			}

			PxHeightFieldDesc height_field_desc;
			height_field_desc.nbColumns = w + 1;
			height_field_desc.nbRows = h + 1;
			height_field_desc.samples.data = samples.data();
			height_field_desc.samples.stride = sizeof(uint);
			px_height_field.reset(device->px_cooking->createHeightField(height_field_desc, device->px_instance->getPhysicsInsertionCallback()));
#endif
		}

		HeightField* HeightField::create(Device* device, graphics::Image* height_map, const uvec2& blocks, uint tess_levels)
		{
			return new HeightFieldPrivate((DevicePrivate*)device, height_map, blocks, tess_levels);
		}

		ShapePrivate::ShapePrivate(DevicePrivate* device, MaterialPrivate* material, const vec3& hf_ext)
		{
			if (!material)
				material = device->mat.get();
#ifdef USE_PHYSX
			px_shape.reset(device->px_instance->createShape(PxBoxGeometry(hf_ext.x, hf_ext.y, hf_ext.z), *material->px_material));
#endif
			px_shape->userData = this;
		}

		ShapePrivate::ShapePrivate(DevicePrivate* device, MaterialPrivate* material, float radius)
		{
			if (!material)
				material = device->mat.get();
#ifdef USE_PHYSX
			px_shape.reset(device->px_instance->createShape(PxSphereGeometry(radius), *material->px_material));
#endif
			px_shape->userData = this;
		}

		ShapePrivate::ShapePrivate(DevicePrivate* device, MaterialPrivate* material, float radius, float height)
		{
			if (!material)
				material = device->mat.get();
#ifdef USE_PHYSX
			px_shape.reset(device->px_instance->createShape(PxCapsuleGeometry(radius, height), *material->px_material));
			px_shape->setLocalPose(PxTransform(PxQuat(PxHalfPi, PxVec3(0.f, 0.f, 1.f))));
#endif
			px_shape->userData = this;
		}

		ShapePrivate::ShapePrivate(DevicePrivate* device, MaterialPrivate* material, TriangleMeshPrivate* tri_mesh, float scale)
		{
			if (!material)
				material = device->mat.get();
#ifdef USE_PHYSX

			px_shape.reset(device->px_instance->createShape(PxTriangleMeshGeometry(tri_mesh->px_triangle_mesh.get(), PxMeshScale(scale)), *material->px_material));
#endif
			px_shape->userData = this;
		}

		ShapePrivate::ShapePrivate(DevicePrivate* device, MaterialPrivate* material, HeightFieldPrivate* height_field, const vec3& scale)
		{
			if (!material)
				material = device->mat.get();
#ifdef USE_PHYSX

			px_shape.reset(device->px_instance->createShape(PxHeightFieldGeometry(height_field->px_height_field.get(), PxMeshGeometryFlags(),
				scale.y / height_field_precision, scale.x / height_field->tess_levels, scale.z / height_field->tess_levels), *material->px_material));
#endif
			px_shape->userData = this;
		}

		void ShapePrivate::set_trigger(bool v)
		{
#ifdef USE_PHYSX
			px_shape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, false);
			px_shape->setFlag(PxShapeFlag::eTRIGGER_SHAPE, true);
#endif
		}

		void ShapePrivate::set_pose(const vec3& coord, const quat& qut)
		{
#ifdef USE_PHYSX
			px_shape->setLocalPose(PxTransform(cvt(coord), cvt(qut)));
#endif
		}

		Shape* Shape::create_box(Device* device, Material* material, const vec3& hf_ext)
		{
			return new ShapePrivate((DevicePrivate*)device, (MaterialPrivate*)material, hf_ext);
		}

		Shape* Shape::create_sphere(Device* device, Material* material, float radius)
		{
			return new ShapePrivate((DevicePrivate*)device, (MaterialPrivate*)material, radius);
		}

		Shape* Shape::create_triangle_mesh(Device* device, Material* material, TriangleMesh* tri_mesh, float scale)
		{
			return new ShapePrivate((DevicePrivate*)device, (MaterialPrivate*)material, (TriangleMeshPrivate*)tri_mesh, scale);
		}

		Shape* Shape::create_height_field(Device* device, Material* material, HeightField* height_field, const vec3& scale) 
		{
			return new ShapePrivate((DevicePrivate*)device, (MaterialPrivate*)material, (HeightFieldPrivate*)height_field, scale);
		}
	}
}

