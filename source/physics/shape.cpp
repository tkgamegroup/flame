#include <flame/foundation/bitmap.h>
#include <flame/graphics/model.h>
#include "shape_private.h"
#include "device_private.h"
#include "material_private.h"

namespace flame
{
	namespace physics
	{
		static std::vector<std::pair<ShapeDesc, PxTriangleMesh*>> staging_triangle_meshes;

		ShapePrivate::ShapePrivate(MaterialPrivate* m, ShapeType type, const ShapeDesc& desc, const Vec3f& coord, const Vec4f& quat)
		{
#ifdef USE_PHYSX
			switch (type)
			{
			case ShapeCube:
				px_shape = DevicePrivate::get()->px_instance->createShape(PxBoxGeometry(desc.box.hf_ext.x(), desc.box.hf_ext.y(), desc.box.hf_ext.z()), *m->px_material);
				px_shape->setLocalPose(PxTransform(cvt(coord), cvt(quat)));
				break;
			case ShapeSphere:
				px_shape = DevicePrivate::get()->px_instance->createShape(PxSphereGeometry(desc.sphere.radius), *m->px_material);
				px_shape->setLocalPose(PxTransform(cvt(coord), cvt(quat)));
				break;
			case ShapeCapsule:
				px_shape = DevicePrivate::get()->px_instance->createShape(PxCapsuleGeometry(desc.capsule.radius, desc.capsule.height), *m->px_material);
				px_shape->setLocalPose(PxTransform(cvt(coord), cvt(quat) * PxQuat(PxHalfPi, PxVec3(0.f, 0.f, 1.f))));
				break;
			case ShapeTriangleMesh:
			{
				PxTriangleMesh* triangle_mesh = nullptr;
				for (auto& m : staging_triangle_meshes)
				{
					if (m.first.triangle_mesh.mesh == desc.triangle_mesh.mesh &&
						m.first.triangle_mesh.scale == desc.triangle_mesh.scale)
						triangle_mesh = m.second;
				}
				if (!triangle_mesh)
				{
					PxTolerancesScale scale;
					PxCookingParams params(scale);

					DevicePrivate::get()->px_cooking->setParams(params);

					std::vector<PxVec3> vertices;
					std::vector<PxU32> indices;
					{
						auto triangle_mesh = desc.triangle_mesh.mesh;
						vertices.resize(triangle_mesh->get_vertices_count());
						indices.resize(triangle_mesh->get_indices_count());
						auto ps = triangle_mesh->get_positions();
						for (auto i = 0; i < vertices.size(); i++)
							vertices[i] = cvt(ps[i]);
						auto is = triangle_mesh->get_indices();
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

					triangle_mesh = DevicePrivate::get()->px_cooking->createTriangleMesh(mesh_desc, DevicePrivate::get()->px_instance->getPhysicsInsertionCallback());
					staging_triangle_meshes.emplace_back(desc, triangle_mesh);
				}
				
				px_shape = DevicePrivate::get()->px_instance->createShape(PxTriangleMeshGeometry(triangle_mesh, PxMeshScale(cvt(desc.triangle_mesh.scale))), *m->px_material);
			}
				break;
			case ShapeHeightField:
			{
				auto w = desc.height_field.height_map->get_width();
				auto h = desc.height_field.height_map->get_height();
				PxHeightFieldDesc height_field_desc;
				height_field_desc.nbColumns = w;
				height_field_desc.nbRows = h;
				auto samples = new uint[w * h];
				height_field_desc.samples.data = samples;
				height_field_desc.samples.stride = sizeof(uint);
				auto data = desc.height_field.height_map->get_data();
				auto pitch = desc.height_field.height_map->get_pitch();
				auto bpp = desc.height_field.height_map->get_byte_per_channel() * desc.height_field.height_map->get_channel();
				auto sample = (PxHeightFieldSample*)samples;
				for (auto y = 0; y < h; y++)
				{
					auto line = &data[y * pitch];
					for (auto x = 0; x < w; x++)
					{
						sample->height = line[x * bpp];
						sample->materialIndex0 = 0;
						sample->materialIndex1 = 0;
						sample->clearTessFlag();
						sample++;
					}
				}
				auto height_field = DevicePrivate::get()->px_cooking->createHeightField(height_field_desc, DevicePrivate::get()->px_instance->getPhysicsInsertionCallback());

				px_shape = DevicePrivate::get()->px_instance->createShape(PxHeightFieldGeometry(height_field, PxMeshGeometryFlags(), 
					desc.height_field.scale.y() / 255.f, desc.height_field.scale.x() / w, desc.height_field.scale.z() / h), *m->px_material);
				delete[]samples;
			}
				break;
			default:
				assert(0);
			}
			px_shape->userData = this;
#endif
		}

		ShapePrivate::~ShapePrivate()
		{
#ifdef USE_PHYSX
			px_shape->release();
#endif
		}

		void ShapePrivate::set_trigger(bool v)
		{
#ifdef USE_PHYSX
			px_shape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, false);
			px_shape->setFlag(PxShapeFlag::eTRIGGER_SHAPE, true);
#endif
		}

		Shape* Shape::create(Material* m, ShapeType type, const ShapeDesc& desc, const Vec3f& coord, const Vec4f& quat)
		{
			return new ShapePrivate((MaterialPrivate*)m, type, desc, coord, quat);
		}
	}
}

