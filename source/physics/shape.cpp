#include <flame/graphics/model.h>
#include "shape_private.h"
#include "device_private.h"
#include "material_private.h"

namespace flame
{
	namespace physics
	{
		static std::vector<std::pair<graphics::Model*, PxTriangleMesh*>> meshes;

		ShapePrivate::ShapePrivate(MaterialPrivate* m, ShapeType type, const ShapeDesc& desc, const Vec3f& coord, const Vec4f& quat)
		{
#ifdef USE_PHYSX
			switch (type)
			{
			case ShapeBox:
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
			case ShapeTriangles:
			{
				PxTriangleMesh* mesh = nullptr;
				for (auto& m : meshes)
				{
					if (m.first == desc.triangles.model)
						mesh = m.second;
				}
				if (!mesh)
				{
					PxTolerancesScale scale;
					PxCookingParams params(scale);

					DevicePrivate::get()->px_cooking->setParams(params);

					auto vtx_cnt = 0;
					auto idx_cnt = 0;
					std::vector<PxVec3> vertices;
					std::vector<PxU32> indices;
					auto model = desc.triangles.model;
					auto mesh_cnt = model->get_meshes_count();
					for (auto i = 0; i < mesh_cnt; i++)
					{
						auto m = model->get_mesh(i);
						vtx_cnt += m->get_vertices_count();
						idx_cnt += m->get_indices_count();
					}
					vertices.resize(vtx_cnt);
					indices.resize(idx_cnt);
					vtx_cnt = 0;
					idx_cnt = 0;
					for (auto i = 0; i < mesh_cnt; i++)
					{
						auto m = model->get_mesh(i);
						auto vc = m->get_vertices_count();
						auto vs = m->get_vertices();
						for (auto j = 0; j < vc; j++)
							vertices[vtx_cnt + j] = cvt(vs[j].pos);
						auto ic = m->get_indices_count();
						auto is = m->get_indices();
						for (auto j = 0; j < ic; j++)
							indices[idx_cnt + j] = is[j] + vtx_cnt;
						vtx_cnt += vc;
						idx_cnt += ic;
					}

					PxTriangleMeshDesc mesh_desc;
					mesh_desc.points.count = vertices.size();
					mesh_desc.points.stride = sizeof(PxVec3);
					mesh_desc.points.data = vertices.data();

					mesh_desc.triangles.count = indices.size() / 3;
					mesh_desc.triangles.stride = 3 * sizeof(PxU32);
					mesh_desc.triangles.data = indices.data();

					mesh = DevicePrivate::get()->px_cooking->createTriangleMesh(mesh_desc, DevicePrivate::get()->px_instance->getPhysicsInsertionCallback());
				}
				
				px_shape = DevicePrivate::get()->px_instance->createShape(PxTriangleMeshGeometry(mesh, PxMeshScale(cvt(desc.triangles.scale))), *m->px_material);
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

