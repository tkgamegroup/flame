#include <flame/graphics/device.h>
#include <flame/graphics/buffer.h>
#include <flame/graphics/image.h>
#include <flame/graphics/model.h>
#include <flame/graphics/command.h>
#include "shape_private.h"
#include "device_private.h"
#include "material_private.h"

namespace flame
{
	namespace physics
	{
		static std::vector<std::pair<ShapeDesc, PxTriangleMesh*>> staging_triangle_meshes;
		static std::vector<std::pair<ShapeDesc, PxHeightField*>> staging_height_fields;
		const auto height_field_precision = 30000.f;

		ShapePrivate::ShapePrivate(DevicePrivate* device, MaterialPrivate* m, ShapeType type, const ShapeDesc& desc, const Vec3f& coord, const Vec4f& quat)
		{
#ifdef USE_PHYSX
			switch (type)
			{
			case ShapeCube:
				px_shape = device->px_instance->createShape(PxBoxGeometry(desc.box.hf_ext.x(), desc.box.hf_ext.y(), desc.box.hf_ext.z()), *m->px_material);
				px_shape->setLocalPose(PxTransform(cvt(coord), cvt(quat)));
				break;
			case ShapeSphere:
				px_shape = device->px_instance->createShape(PxSphereGeometry(desc.sphere.radius), *m->px_material);
				px_shape->setLocalPose(PxTransform(cvt(coord), cvt(quat)));
				break;
			case ShapeCapsule:
				px_shape = device->px_instance->createShape(PxCapsuleGeometry(desc.capsule.radius, desc.capsule.height), *m->px_material);
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

					device->px_cooking->setParams(params);

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

					triangle_mesh = device->px_cooking->createTriangleMesh(mesh_desc, device->px_instance->getPhysicsInsertionCallback());
					staging_triangle_meshes.emplace_back(desc, triangle_mesh);
				}
				
				px_shape = device->px_instance->createShape(PxTriangleMeshGeometry(triangle_mesh, PxMeshScale(cvt(desc.triangle_mesh.scale))), *m->px_material);
			}
				break;
			case ShapeHeightField:
			{
				PxHeightField* height_field = nullptr;
				for (auto& m : staging_height_fields)
				{
					if (m.first.height_field.height_map == desc.height_field.height_map &&
						m.first.height_field.blocks == desc.height_field.blocks &&
						m.first.height_field.tess_levels == desc.height_field.tess_levels)
						height_field = m.second;
				}
				auto blocks = desc.height_field.blocks;
				auto lv = desc.height_field.tess_levels;
				auto w = blocks.x() * lv;
				auto h = blocks.y() * lv;
				if (!height_field)
				{
					std::vector<uint> samples;
					samples.resize((w + 1) * (h + 1));
					{
						auto dev = graphics::Device::get();
						auto img = desc.height_field.height_map;
						auto img_size = img->get_size();
						auto buf = graphics::Buffer::create(dev, img_size.x() * img_size.y(), graphics::BufferUsageTransferDst, graphics::MemoryPropertyHost | graphics::MemoryPropertyCoherent);
						auto cb = graphics::CommandBuffer::create(dev->get_command_pool(graphics::QueueGraphics));
						cb->begin(true);
						cb->image_barrier(img, {}, graphics::ImageLayoutShaderReadOnly, graphics::ImageLayoutTransferSrc);
						graphics::BufferImageCopy cpy;
						cpy.image_extent = img_size;
						cb->copy_image_to_buffer(img, buf, 1, &cpy);
						cb->image_barrier(img, {}, graphics::ImageLayoutTransferSrc, graphics::ImageLayoutShaderReadOnly);
						cb->end();
						auto que = dev->get_queue(graphics::QueueGraphics);
						que->submit(1, &cb, nullptr, nullptr, nullptr);
						que->wait_idle();
						buf->map();
						auto src = (uchar*)buf->get_mapped();
						auto dst = (PxHeightFieldSample*)samples.data();
						auto sample = [&](int x, int y) {
							if (x < 0)
								//x = img_size.x() - 1;
								x = 0;
							else if (x >= img_size.x())
								//x = 0;
								x = img_size.x() - 1;
							if (y < 0)
								//y = img_size.y() - 1;
								y = 0;
							else if (y >= img_size.y())
								//y = 0;
								y = img_size.y() - 1;
							return src[y * img_size.x() + x] / 255.f;
						};
						auto lvhf = lv >> 1;
						for (auto x = 0; x <= w; x++)
						{
							for (auto y = 0; y <= h; y++)
							{
								auto tc = Vec2f(x / (float)w, y / (float)h) * img_size;
								auto itc = Vec2i(tc);
								auto ftc = tc - itc;
								float height;
								if (ftc.x() > 0.5f && ftc.y() > 0.5f)
								{
									ftc.x() -= 0.5f;
									ftc.y() -= 0.5f;
									height =
										(sample(itc.x(), itc.y()) * (1.f - ftc.x()) + sample(itc.x() + 1, itc.y()) * ftc.x()) * (1.f - ftc.y()) +
										(sample(itc.x(), itc.y() + 1) * (1.f - ftc.x()) + sample(itc.x() + 1, itc.y() + 1) * ftc.x()) * ftc.y();
								}
								else if (ftc.x() > 0.5f && ftc.y() < 0.5f)
								{
									ftc.x() -= 0.5f;
									ftc.y() += 0.5f;
									height =
										(sample(itc.x(), itc.y() - 1) * (1.f - ftc.x()) + sample(itc.x() + 1, itc.y() - 1) * ftc.x()) * (1.f - ftc.y()) +
										(sample(itc.x(), itc.y()) * (1.f - ftc.x()) + sample(itc.x() + 1, itc.y()) * ftc.x()) * ftc.y();
								}
								else if (ftc.x() < 0.5f && ftc.y() > 0.5f)
								{
									ftc.x() += 0.5f;
									ftc.y() -= 0.5f;
									height =
										(sample(itc.x() - 1, itc.y()) * (1.f - ftc.x()) + sample(itc.x(), itc.y()) * ftc.x()) * (1.f - ftc.y()) +
										(sample(itc.x() - 1, itc.y() + 1) * (1.f - ftc.x()) + sample(itc.x(), itc.y() + 1) * ftc.x()) * ftc.y();
								}
								else
								{
									ftc.x() += 0.5f;
									ftc.y() += 0.5f;
									height =
										(sample(itc.x() - 1, itc.y() - 1) * (1.f - ftc.x()) + sample(itc.x(), itc.y() - 1) * ftc.x()) * (1.f - ftc.y()) +
										(sample(itc.x() - 1, itc.y()) * (1.f - ftc.x()) + sample(itc.x(), itc.y()) * ftc.x()) * ftc.y();
								}
								dst->height = height * height_field_precision;

								dst->materialIndex0 = 0;
								dst->materialIndex1 = 0;
								auto s1 = x % lv < lvhf ? 1 : -1;
								auto s2 = y % lv < lvhf ? 1 : -1;
								if (s1 * s2 > 0)
									dst->setTessFlag();
								else
									dst->clearTessFlag();
								dst++;
							}
						}
						cb->release();
						buf->release();
					}

					PxHeightFieldDesc height_field_desc;
					height_field_desc.nbColumns = w + 1;
					height_field_desc.nbRows = h + 1;
					height_field_desc.samples.data = samples.data();
					height_field_desc.samples.stride = sizeof(uint);

					height_field = device->px_cooking->createHeightField(height_field_desc, device->px_instance->getPhysicsInsertionCallback());
					staging_height_fields.emplace_back(desc, height_field);
				}

				px_shape = device->px_instance->createShape(PxHeightFieldGeometry(height_field, PxMeshGeometryFlags(),
					desc.height_field.scale.y() / height_field_precision, desc.height_field.scale.x() / lv, desc.height_field.scale.z() / lv), *m->px_material);
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

		Shape* Shape::create(Device* device, Material* m, ShapeType type, const ShapeDesc& desc, const Vec3f& coord, const Vec4f& quat)
		{
			return new ShapePrivate((DevicePrivate*)device, (MaterialPrivate*)m, type, desc, coord, quat);
		}
	}
}

