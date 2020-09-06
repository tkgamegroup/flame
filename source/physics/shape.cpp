#include <flame/graphics/model.h>
#include "shape_private.h"
#include "device_private.h"
#include "material_private.h"

namespace flame
{
	namespace physics
	{
		static std::vector<std::pair<ShapeDesc, PxTriangleMesh*>> meshes;

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
			case ShapeMesh:
			{
				PxTriangleMesh* mesh = nullptr;
				for (auto& m : meshes)
				{
					if (m.first.mesh.mesh == desc.mesh.mesh &&
						m.first.mesh.scale == desc.mesh.scale)
						mesh = m.second;
				}
				if (!mesh)
				{
					PxTolerancesScale scale;
					PxCookingParams params(scale);

					DevicePrivate::get()->px_cooking->setParams(params);

					std::vector<PxVec3> vertices;
					std::vector<PxU32> indices;
					{
						auto mesh = desc.mesh.mesh;
						vertices.resize(mesh->get_vertices_count_1());
						indices.resize(mesh->get_indices_count());
						auto vs = mesh->get_vertices_1();
						for (auto i = 0; i < vertices.size(); i++)
							vertices[i] = cvt(vs[i].pos);
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

					mesh = DevicePrivate::get()->px_cooking->createTriangleMesh(mesh_desc, DevicePrivate::get()->px_instance->getPhysicsInsertionCallback());
					meshes.emplace_back(desc, mesh);
				}
				
				px_shape = DevicePrivate::get()->px_instance->createShape(PxTriangleMeshGeometry(mesh, PxMeshScale(cvt(desc.mesh.scale))), *m->px_material);
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

