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
			px_triangle_mesh = device->px_cooking->createTriangleMesh(mesh_desc, device->px_instance->getPhysicsInsertionCallback());
#endif
		}

		TriangleMeshPrivate::~TriangleMeshPrivate()
		{
#ifdef USE_PHYSX
			px_triangle_mesh->release();
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
			std::vector<uint> samples;
			samples.resize((w + 1) * (h + 1));
			{
				auto dev = graphics::Device::get_default();
				auto img_size = height_map->get_size();
				auto buf = graphics::Buffer::create(dev, img_size.x * img_size.y, graphics::BufferUsageTransferDst, graphics::MemoryPropertyHost | graphics::MemoryPropertyCoherent);
				auto cb = graphics::CommandBuffer::create(graphics::CommandPool::get(dev));
				cb->begin(true);
				cb->image_barrier(height_map, {}, graphics::ImageLayoutShaderReadOnly, graphics::ImageLayoutTransferSrc);
				graphics::BufferImageCopy cpy;
				cpy.image_extent = img_size;
				cb->copy_image_to_buffer(height_map, buf, 1, &cpy);
				cb->image_barrier(height_map, {}, graphics::ImageLayoutTransferSrc, graphics::ImageLayoutShaderReadOnly);
				cb->end();
				auto que = graphics::Queue::get(dev);
				que->submit(1, &cb, nullptr, nullptr, nullptr);
				que->wait_idle();
				buf->map();
				auto src = (uchar*)buf->get_mapped();
				auto dst = (PxHeightFieldSample*)samples.data();
				auto sample = [&](int x, int y) {
					if (x < 0)
						x = 0;
					else if (x >= img_size.x)
						x = img_size.x - 1;
					if (y < 0)
						y = 0;
					else if (y >= img_size.y)
						y = img_size.y - 1;
					return src[y * img_size.x + x] / 255.f;
				};
				auto lvhf = tess_levels >> 1;
				for (auto x = 0; x <= w; x++)
				{
					for (auto y = 0; y <= h; y++)
					{
						auto tc = vec2(x / (float)w, y / (float)h) * vec2(img_size);
						auto itc = ivec2(tc);
						auto ftc = tc - vec2(itc);
						float height;
						if (ftc.x > 0.5f && ftc.y > 0.5f)
						{
							ftc.x -= 0.5f;
							ftc.y -= 0.5f;
							height =
								(sample(itc.x, itc.y) * (1.f - ftc.x) + sample(itc.x + 1, itc.y) * ftc.x) * (1.f - ftc.y) +
								(sample(itc.x, itc.y + 1) * (1.f - ftc.x) + sample(itc.x + 1, itc.y + 1) * ftc.x) * ftc.y;
						}
						else if (ftc.x > 0.5f && ftc.y < 0.5f)
						{
							ftc.x -= 0.5f;
							ftc.y += 0.5f;
							height =
								(sample(itc.x, itc.y - 1) * (1.f - ftc.x) + sample(itc.x + 1, itc.y - 1) * ftc.x) * (1.f - ftc.y) +
								(sample(itc.x, itc.y) * (1.f - ftc.x) + sample(itc.x + 1, itc.y) * ftc.x) * ftc.y;
						}
						else if (ftc.x < 0.5f && ftc.y > 0.5f)
						{
							ftc.x += 0.5f;
							ftc.y -= 0.5f;
							height =
								(sample(itc.x - 1, itc.y) * (1.f - ftc.x) + sample(itc.x, itc.y) * ftc.x) * (1.f - ftc.y) +
								(sample(itc.x - 1, itc.y + 1) * (1.f - ftc.x) + sample(itc.x, itc.y + 1) * ftc.x) * ftc.y;
						}
						else
						{
							ftc.x += 0.5f;
							ftc.y += 0.5f;
							height =
								(sample(itc.x - 1, itc.y - 1) * (1.f - ftc.x) + sample(itc.x, itc.y - 1) * ftc.x) * (1.f - ftc.y) +
								(sample(itc.x - 1, itc.y) * (1.f - ftc.x) + sample(itc.x, itc.y) * ftc.x) * ftc.y;
						}
						dst->height = height * height_field_precision;

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
				cb->release();
				buf->release();
			}

			PxHeightFieldDesc height_field_desc;
			height_field_desc.nbColumns = w + 1;
			height_field_desc.nbRows = h + 1;
			height_field_desc.samples.data = samples.data();
			height_field_desc.samples.stride = sizeof(uint);
			px_height_field = device->px_cooking->createHeightField(height_field_desc, device->px_instance->getPhysicsInsertionCallback());
#endif
		}

		HeightFieldPrivate::~HeightFieldPrivate()
		{
#ifdef USE_PHYSX
			px_height_field->release();
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
			px_shape = device->px_instance->createShape(PxBoxGeometry(hf_ext.x, hf_ext.y, hf_ext.z), *material->px_material);
#endif
			px_shape->userData = this;
		}

		ShapePrivate::ShapePrivate(DevicePrivate* device, MaterialPrivate* material, float radius)
		{
			if (!material)
				material = device->mat.get();
#ifdef USE_PHYSX
			px_shape = device->px_instance->createShape(PxSphereGeometry(radius), *material->px_material);
#endif
			px_shape->userData = this;
		}

		ShapePrivate::ShapePrivate(DevicePrivate* device, MaterialPrivate* material, float radius, float height)
		{
			if (!material)
				material = device->mat.get();
#ifdef USE_PHYSX
			px_shape = device->px_instance->createShape(PxCapsuleGeometry(radius, height), *material->px_material);
			px_shape->setLocalPose(PxTransform(PxQuat(PxHalfPi, PxVec3(0.f, 0.f, 1.f))));
#endif
			px_shape->userData = this;
		}

		ShapePrivate::ShapePrivate(DevicePrivate* device, MaterialPrivate* material, TriangleMeshPrivate* tri_mesh, float scale)
		{
			if (!material)
				material = device->mat.get();
#ifdef USE_PHYSX

			px_shape = device->px_instance->createShape(PxTriangleMeshGeometry(tri_mesh->px_triangle_mesh, PxMeshScale(scale)), *material->px_material);
#endif
			px_shape->userData = this;
		}

		ShapePrivate::ShapePrivate(DevicePrivate* device, MaterialPrivate* material, HeightFieldPrivate* height_field, const vec3& scale)
		{
			if (!material)
				material = device->mat.get();
#ifdef USE_PHYSX

			px_shape = device->px_instance->createShape(PxHeightFieldGeometry(height_field->px_height_field, PxMeshGeometryFlags(),
				scale.y / height_field_precision, scale.x / height_field->tess_levels, scale.z / height_field->tess_levels), *material->px_material);
#endif
			px_shape->userData = this;
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

