#include "../../graphics/device.h"
#include "../../graphics/buffer.h"
#include "../../graphics/image.h"
#include "../../graphics/renderpass.h"
#include "../../graphics/shader.h"
#include "../../graphics/font.h"
#include "../../graphics/material.h"
#include "../../graphics/model.h"
#include "../world_private.h"
#include "../components/element_private.h"
#include "../components/node_private.h"
#include "../components/camera_private.h"
#include "renderer_private.h"

namespace element
{
#include <element/element.dsl.h>
#include <element/element.pll.h>
}
#include <render_data.dsl.h>
#include <material.dsl.h>
#include <light.dsl.h>
namespace mesh
{
#include <mesh/mesh.dsl.h>
#include <mesh/forward.pll.h>
#include <mesh/gbuffer.pll.h>
}
namespace terrain
{
#include <terrain/terrain.dsl.h>
#include <terrain/forward.pll.h>
#include <terrain/gbuffer.pll.h>
}
#include <deferred/deferred.dsl.h>
#include <deferred/deferred.pll.h>
#include <particle/particle.pll.h>
#include <post/post.dsl.h>
#include <post/post.pll.h>

namespace flame
{
	static graphics::AccessFlags access_from_usage(graphics::BufferUsageFlags usage)
	{
		switch (usage)
		{
		case graphics::BufferUsageVertex:
			return graphics::AccessVertexAttributeRead;
		case graphics::BufferUsageIndex:
			return graphics::AccessIndexRead;
		case graphics::BufferUsageIndirect:
			return graphics::AccessIndirectCommandRead;
		}
		return graphics::AccessNone;
	}

	template <class T>
	struct SequentialBuffer
	{
		uint capacity;
		graphics::AccessFlags access;
		T* pstag = nullptr;
		uint stag_num = 0;

		UniPtr<graphics::Buffer> buf;
		UniPtr<graphics::Buffer> stagbuf;

		void rebuild()
		{
			T* temp = nullptr;
			auto n = 0;
			if (stag_num > 0)
			{
				n = stag_num;
				temp = new T[n];
				memcpy(temp, stagbuf->get_mapped(), sizeof(T) * n);
			}
			auto size = capacity * sizeof(T);
			buf->recreate(size);
			stagbuf->recreate(size);
			stagbuf->map();
			pstag = (T*)stagbuf->get_mapped();
			if (temp)
			{
				push(n, temp);
				delete[]temp;
			}
		}

		void create(graphics::Device* device, graphics::BufferUsageFlags usage, uint _capacity)
		{
			capacity = _capacity;
			access = access_from_usage(usage);
			auto size = capacity * sizeof(T);
			buf.reset(graphics::Buffer::create(device, size, graphics::BufferUsageTransferDst | usage, graphics::MemoryPropertyDevice));
			stagbuf.reset(graphics::Buffer::create(device, size, graphics::BufferUsageTransferSrc, graphics::MemoryPropertyHost | graphics::MemoryPropertyCoherent));
			stagbuf->map();
			pstag = (T*)stagbuf->get_mapped();
		}

		void push(uint cnt, const T* p)
		{
			fassert(stag_num + cnt <= capacity);
			//if (stag_num + cnt > capacity)
			//{
			//	capacity = (stag_num + cnt) * 2;
			//	rebuild();
			//}

			memcpy(pstag + stag_num, p, sizeof(T) * cnt);
			stag_num += cnt;
		}

		T* stag(uint cnt)
		{
			fassert(stag_num + cnt <= capacity);
			//if (stag_num + cnt > capacity)
			//{
			//	capacity = (stag_num + cnt) * 2;
			//	rebuild();
			//}

			auto dst = pstag + stag_num;
			stag_num += cnt;
			return dst;
		}

		void upload(graphics::CommandBuffer* cb)
		{
			if (stag_num == 0)
				return;
			graphics::BufferCopy cpy;
			cpy.size = stag_num * sizeof(T);
			cb->copy_buffer(stagbuf.get(), buf.get(), 1, &cpy);
			cb->buffer_barrier(buf.get(), graphics::AccessTransferWrite, access);
			stag_num = 0;
		}
	};

	template <class T>
	struct SparseBuffer
	{
		uint capacity;
		graphics::AccessFlags access;
		uint n0 = 0;
		uint n1 = 0;

		UniPtr<graphics::Buffer> buf;
		UniPtr<graphics::Buffer> stagbuf;
		uint stag_capacity;
		T* pstag = nullptr;

		void create(graphics::Device* device, graphics::BufferUsageFlags usage, uint _capacity)
		{
			capacity = _capacity;
			access = access_from_usage(usage);
			auto size = capacity * sizeof(T);
			buf.reset(graphics::Buffer::create(device, size, graphics::BufferUsageTransferDst | usage, graphics::MemoryPropertyDevice));
			stag_capacity = 100;
			stagbuf.reset(graphics::Buffer::create(device, sizeof(T) * stag_capacity, graphics::BufferUsageTransferSrc, graphics::MemoryPropertyHost | 
				graphics::MemoryPropertyCoherent));
			pstag = (T*)stagbuf->map();
		}

		T* alloc(uint n)
		{
			fassert(n0 == n1);
			fassert(n1 + n <= capacity);
			if (stag_capacity < n)
			{
				stag_capacity = n;
				stagbuf->recreate(n * sizeof(T));
				pstag = (T*)stagbuf->map();
			}
			n1 += n;
			return pstag;
		}

		void free(T* p)
		{
			// TODO
		}

		void upload(graphics::CommandBuffer* cb)
		{
			if (n1 > n0)
			{
				graphics::BufferCopy cpy;
				cpy.size = (n1 - n0) * sizeof(T);
				cpy.dst_off = n0 * sizeof(T);
				cb->copy_buffer(stagbuf.get(), buf.get(), 1, &cpy);
				cb->buffer_barrier(buf.get(), graphics::AccessTransferWrite, access);
				n0 = n1;
			}
		}
	};

	template <class T>
	struct StorageBuffer
	{
		T* pstag = nullptr;

		UniPtr<graphics::Buffer> buf;
		UniPtr<graphics::Buffer> stagbuf;

		std::vector<graphics::BufferCopy> cpies;

		void create(graphics::Device* device, graphics::BufferUsageFlags usage)
		{
			buf.reset(graphics::Buffer::create(device, sizeof(T), graphics::BufferUsageTransferDst | usage, graphics::MemoryPropertyDevice));
			stagbuf.reset(graphics::Buffer::create(device, sizeof(T), graphics::BufferUsageTransferSrc, graphics::MemoryPropertyHost | graphics::MemoryPropertyCoherent));
			pstag = (T*)stagbuf->map();
		}

		void cpy_whole()
		{
			fassert(cpies.empty());
			graphics::BufferCopy cpy;
			cpy.size = sizeof(T);
			cpies.push_back(cpy);
		}

		void upload(graphics::CommandBuffer* cb)
		{
			if (cpies.empty())
				return;
			cb->copy_buffer(stagbuf.get(), buf.get(), cpies.size(), cpies.data());
			cb->buffer_barrier(buf.get(), graphics::AccessTransferWrite, graphics::AccessShaderRead);
			cpies.clear();
		}
	};

	template <class T>
	struct ArrayStorageBuffer : StorageBuffer<T>
	{
		using StorageBuffer<T>::pstag;
		using StorageBuffer<T>::cpies;

		auto& item(uint idx, bool mark_cpy = true)
		{
			auto& [items] = *pstag;
			fassert(idx < _countof(items));

			auto& item = items[idx];

			if (mark_cpy)
			{
				graphics::BufferCopy cpy;
				cpy.src_off = cpy.dst_off = idx * sizeof(item);
				cpy.size = sizeof(item);
				cpies.push_back(cpy);
			}

			return item;
		}
	};

	template <class T>
	struct SequentialArrayStorageBuffer : ArrayStorageBuffer<T>
	{
		using StorageBuffer<T>::pstag;
		using StorageBuffer<T>::cpies;
		using ArrayStorageBuffer<T>::item;

		uint n = 0;

		auto& add_item()
		{
			auto& i = item(n, false);

			if (cpies.empty())
				cpies.emplace_back();
			cpies.back().size += sizeof(i);

			n++;
			return i;
		}

		auto& add_item_overwrite_size(uint overwrite_size)
		{
			auto& i = item(n, false);

			{
				graphics::BufferCopy cpy;
				cpy.src_off = cpy.dst_off = n * sizeof(i);
				cpy.size = overwrite_size;
				cpies.push_back(cpy);
			}

			n++;
			return i;
		}

		void upload(graphics::CommandBuffer* cb)
		{
			StorageBuffer<T>::upload(cb);
			n = 0;
		}
	};

	struct ElementVertex
	{
		vec2 pos;
		vec2 uv;
		cvec4 col;
	};

	struct MeshVertex
	{
		vec3 pos;
		vec2 uv;
		vec3 normal;
	};

	struct ArmMeshVertex : MeshVertex
	{
		ivec4 ids;
		vec4 weights;
	};

	struct ParticleVertex
	{
		vec3 pos;
		vec2 uv;
		cvec4 col;
	};

	struct ElementDrawCmd
	{
		uint res;
		std::vector<ElementVertex> vertices;
		std::vector<uint> indices;
		Rect scissor;
	};

	struct MaterialPipeline
	{
		std::filesystem::path mat;
		std::vector<std::string> defines;
		uint ref_count = 1;
		UniPtr<graphics::Pipeline> pipeline;
	};

	struct MaterialRes
	{
		graphics::Material* mat;
		std::filesystem::path pipeline_file;
		std::string pipeline_defines;
		int texs[4];
		graphics::Pipeline* pls[MaterialUsageCount] = {};

		graphics::Pipeline* get_pl(sRendererPrivate* thiz, MaterialUsage u);

	};

	struct MeshRes
	{
		graphics::Mesh* mesh = nullptr;
		bool arm;
		uint vtx_off;
		uint vtx_cnt;
		uint idx_off;
		uint idx_cnt;
		uint mat_id;
	};

	struct ElemenetRenderData
	{
		bool should_render;

		std::vector<graphics::ImageView*> reses;

		Rect						scissor;
		std::vector<ElementDrawCmd> layers[128];
		uint						max_layer = 0;

		SequentialBuffer<ElementVertex>	buf_vtx;
		SequentialBuffer<uint>			buf_idx;
		UniPtr<graphics::DescriptorSet>	ds_element;

		graphics::Pipeline* pl_element;
	};

	enum MaterialType
	{
		MaterialWireframe,
		MaterialOutline,
		MaterialPickup,
		MaterialNormalData,
		MaterialCustom,
		MaterialTypeCount
	};

	struct NodeRenderData
	{
		bool should_render;

		uint dir_shadow_levels = 3U;
		float dir_shadow_dist = 100.f;
		float pt_shadow_dist = 20.f;
		float pt_shadow_near = 0.1f;

		std::vector<graphics::ImageView*> tex_reses;
		std::vector<MaterialRes> mat_reses;
		uint max_mat_id = 0;
		std::vector<MeshRes> mesh_reses;

		std::vector<std::pair<uint, mat3>>				dir_shadows;
		std::vector<std::pair<uint, vec3>>				pt_shadows;
		std::vector<std::vector<std::pair<uint, uint>>>	meshes[MaterialMeshUsageCount];
		std::vector<std::pair<uint, uint>>				terrains[MaterialTypeCount];
		std::vector<std::pair<uint, uint>>				particles;

		SequentialBuffer<graphics::DrawIndexedIndirectCommand>	buf_mesh_indirs[MaterialMeshUsageCount];

		SparseBuffer<MeshVertex>	buf_mesh_vtx;
		SparseBuffer<uint>			buf_mesh_idx;
		SparseBuffer<ArmMeshVertex>	buf_arm_mesh_vtx;
		SparseBuffer<uint>			buf_arm_mesh_idx;

		StorageBuffer<DSL_render_data::RenderData>							buf_render_data;
		UniPtr<graphics::DescriptorSet>										ds_render_data;
		ArrayStorageBuffer<DSL_material::MaterialInfos>						buf_materials;
		UniPtr<graphics::DescriptorSet>										ds_material;
		SequentialArrayStorageBuffer<mesh::DSL_mesh::Transforms>			buf_mesh_transforms;
		SequentialArrayStorageBuffer<mesh::DSL_mesh::Armatures>				buf_mesh_armatures;
		UniPtr<graphics::DescriptorSet>										ds_mesh;
		SequentialArrayStorageBuffer<terrain::DSL_terrain::TerrainInfos>	buf_terrain;
		UniPtr<graphics::DescriptorSet>										ds_terrain;

		UniPtr<graphics::Image> img_dep;
		UniPtr<graphics::Image> img_col_met; // color, metallic
		UniPtr<graphics::Image> img_nor_rou; // normal, roughness

		SequentialArrayStorageBuffer<DSL_light::LightInfos>			buf_light_infos;
		ArrayStorageBuffer<DSL_light::TileLightsMap>				buf_tile_lights;
		SequentialArrayStorageBuffer<DSL_light::DirShadows>			buf_dir_shadows;
		SequentialArrayStorageBuffer<DSL_light::PtShadows>			buf_pt_shadows;
		std::vector<UniPtr<graphics::Image>>						img_dir_shadow_maps;
		std::vector<UniPtr<graphics::Image>>						img_pt_shadow_maps;
		UniPtr<graphics::DescriptorSet>								ds_light;

		graphics::PipelineLayout* pll_mesh_fwd;
		graphics::PipelineLayout* pll_mesh_gbuf;
		graphics::PipelineLayout* pll_terrain_fwd;
		graphics::PipelineLayout* pll_terrain_gbuf;

		UniPtr<graphics::Framebuffer> fb_gbuf;
		UniPtr<graphics::Framebuffer> fb_fwd;
		UniPtr<graphics::Framebuffer> fb_fwd_dc;

		std::vector<MaterialPipeline> pl_mats[MaterialUsageCount];

		graphics::Pipeline* pl_def;
		UniPtr<graphics::DescriptorSet>	ds_def;

		SequentialBuffer<ParticleVertex>	buf_ptc_vtx;
		SequentialBuffer<uint>				buf_ptc_idx;
		graphics::Pipeline*					pl_ptc;

		SequentialBuffer<graphics::Line>	buf_lines;
		graphics::Pipeline*					pl_line;

		graphics::Pipeline* pl_downsample;
		graphics::Pipeline* pl_upsample;
		graphics::Pipeline* pl_add;
		graphics::Pipeline* pl_gamma;

		UniPtr<graphics::Image> img_back;
	};

	sRendererPrivate::sRendererPrivate(sRendererParms* _parms)
	{
		sRendererParms parms;
		if (_parms)
			parms = *_parms;

		_ed.reset(new ElemenetRenderData);
		_nd.reset(new NodeRenderData);
	}

	sRendererPrivate::~sRendererPrivate()
	{
		graphics::Queue::get(device)->wait_idle();
	}

	uint sRendererPrivate::element_render(uint layer, cElementPrivate* element)
	{
		auto& ed = *_ed;

		auto e = element->entity;

		if (element->layer_policy == 2)
			element->layer_policy = 0;

		element->parent_scissor = ed.scissor;
		element->update_transform();
		auto culled = !ed.scissor.overlapping(element->bounds);
		if (element->culled != culled)
		{
			element->culled = culled;
			e->component_data_changed(element, S<"culled"_h>);
		}
		if (culled)
			return layer;

		auto self_transparent = true;
		if (!element->draw(layer))
			self_transparent = false;

		auto clipping = false;
		Rect last_scissor;
		if (element->clipping && !(ed.scissor == element->bounds))
		{
			element->layer_policy = 2;

			clipping = true;
			last_scissor = ed.scissor;
			ed.scissor = element->bounds;
			layer = ed.max_layer;
			auto& info = ed.layers[layer].emplace_back();
			info.scissor = ed.scissor;
		}

		if (!element->drawers.empty())
		{
			auto l = layer;
			for (auto d : element->drawers)
				layer = max(layer, d->draw(l, this));
			self_transparent = false;
		}

		if (!self_transparent)
			layer++;
		ed.max_layer = max(ed.max_layer, layer);
		auto children_max_layer = layer;
		for (auto& c : e->children)
		{
			if (!c->global_visibility)
				continue;

			auto celement = c->get_component_i<cElementPrivate>(0);
			if (celement)
			{
				children_max_layer = max(children_max_layer, element_render(layer, celement));
				if (celement->layer_policy > 0)
				{
					layer = children_max_layer;
					if (celement->layer_policy == 2)
						element->layer_policy = 2;
				}
			}
		}

		if (clipping)
		{
			ed.scissor = last_scissor;
			auto& info = ed.layers[children_max_layer].emplace_back();
			info.scissor = last_scissor;
		}

		return max(layer, children_max_layer);
	}

	void sRendererPrivate::node_render(cNodePrivate* node, Frustum* lod_frustums)
	{
		auto e = node->entity;

		node->update_transform();
		node->draw(frame, false);

		if (node->octree)
		{
			std::vector<cNodePrivate*> objs;
			node->octree->get_within_frustum(lod_frustums[node->octree_lod], objs);
			for (auto obj : objs)
				node_render(obj, lod_frustums);
		}
		else
		{
			for (auto& c : e->children)
			{
				if (!c->global_visibility)
					continue;

				auto cnode = c->get_component_i<cNodePrivate>(0);
				if (cnode)
					node_render(cnode, lod_frustums);
			}
		}
	}

	void sRendererPrivate::get_shadow_props(uint* dir_levels, float* dir_dist, float* pt_dist)
	{
		auto& nd = *_nd;

		if (dir_levels)
			*dir_levels = nd.dir_shadow_levels;
		if (dir_dist)
			*dir_dist = nd.dir_shadow_dist;
		if (pt_dist)
			*pt_dist = nd.pt_shadow_dist;
	}

	void sRendererPrivate::set_shadow_props(uint dir_levels, float dir_dist, float pt_dist)
	{
		auto& nd = *_nd;

		nd.dir_shadow_levels = dir_levels;
		nd.dir_shadow_dist = dir_dist;
		nd.pt_shadow_dist = pt_dist;
	}

	graphics::ImageView* sRendererPrivate::get_element_res(uint idx) const
	{
		return _ed->reses[idx];
	}

	int sRendererPrivate::set_element_res(int idx, graphics::ImageView* iv)
	{
		auto iv_white = img_white->get_view();
		auto& ed = *_ed;

		if (!iv)
			iv = iv_white;

		if (idx == -1)
		{
			for (auto i = 1; i < ed.reses.size(); i++)
			{
				if (ed.reses[i] == iv_white)
				{
					idx = i;
					break;
				}
			}
		}
		if (idx == -1)
			return -1;

		ed.reses[idx] = iv;
		ed.ds_element->set_image(graphics::DescriptorSetLayout::get(device, L"element/element.dsl")
			->find_binding("images"), idx, iv, sp_linear);
		ed.ds_element->update();

		return idx;
	}

	int sRendererPrivate::find_element_res(graphics::ImageView* iv) const
	{
		auto& ed = *_ed;
		for (auto i = 0; i < ed.reses.size(); i++)
		{
			if (ed.reses[i] == iv)
				return i;
		}
		return -1;
	}

	void sRendererPrivate::fill(uint layer, uint pt_cnt, const vec2* pts, const cvec4& color)
	{
		if (pt_cnt == 0)
			return;

		auto& ed = *_ed;

		auto vtx_cnt = 3 * (pt_cnt - 2);
		auto idx_cnt = vtx_cnt;
		ed.buf_vtx.stag_num += vtx_cnt;
		ed.buf_idx.stag_num += idx_cnt;

		auto& info = ed.layers[layer].emplace_back();
		info.res = 0;
		info.vertices.resize(vtx_cnt);
		info.indices.resize(idx_cnt);

		auto off = 0;
		for (auto i = 0; i < pt_cnt - 2; i++)
		{
			info.vertices[off + 0] = { pts[0],		vec2(0.5f), color };
			info.vertices[off + 1] = { pts[i + 1],	vec2(0.5f), color };
			info.vertices[off + 2] = { pts[i + 2],	vec2(0.5f), color };

			info.indices[off + 0] = off + 0;
			info.indices[off + 1] = off + 2;
			info.indices[off + 2] = off + 1;

			off += 3;
		}
	}

	void sRendererPrivate::stroke(uint layer, uint pt_cnt, const vec2* pts, float thickness, const cvec4& color, bool closed)
	{
		if (pt_cnt == 0)
			return;

		auto& ed = *_ed;

		auto vtx_cnt = 2 * pt_cnt;
		auto idx_cnt = 6 * (pt_cnt - 1);
		if (closed)
			idx_cnt += 6;
		ed.buf_vtx.stag_num += vtx_cnt;
		ed.buf_idx.stag_num += idx_cnt;

		auto& info = ed.layers[layer].emplace_back();
		info.res = 0;
		info.vertices.resize(vtx_cnt);
		info.indices.resize(idx_cnt);

		auto get_normal = [](const vec2& p1, const vec2& p2) {
			auto d = normalize(p2 - p1);
			return vec2(d.y, -d.x);
		};

		auto vtx_off = 0;
		auto idx_off = 0;
		vec2 first_normal;
		vec2 last_normal;

		first_normal = last_normal = get_normal(pts[0], pts[1]);
		info.vertices[0] = { pts[0] + first_normal * thickness, vec2(0.5f), color };
		info.vertices[1] = { pts[0] - first_normal * thickness, vec2(0.5f), color };

		vtx_off += 2;
		for (auto i = 1; i < pt_cnt - 1; i++)
		{
			auto _n = get_normal(pts[i], pts[i + 1]);
			auto n = (last_normal + _n) * 0.5f;
			last_normal = _n;
			info.vertices[vtx_off + 0] = { pts[i] + n * thickness, vec2(0.5f), color };
			info.vertices[vtx_off + 1] = { pts[i] - n * thickness, vec2(0.5f), color };

			info.indices[idx_off + 0] = vtx_off - 2;
			info.indices[idx_off + 1] = vtx_off - 1;
			info.indices[idx_off + 2] = vtx_off + 1;
			info.indices[idx_off + 3] = vtx_off - 2;
			info.indices[idx_off + 4] = vtx_off + 1;
			info.indices[idx_off + 5] = vtx_off + 0;

			vtx_off += 2;
			idx_off += 6;
		}

		if (closed)
		{
			auto _n = get_normal(pts[pt_cnt - 1], pts[0]);
			auto n = (last_normal + _n) * 0.5f;

			info.vertices[vtx_off + 0] = { pts[pt_cnt - 1] + n * thickness, vec2(0.5f), color };
			info.vertices[vtx_off + 1] = { pts[pt_cnt - 1] - n * thickness, vec2(0.5f), color };

			info.indices[idx_off + 0] = vtx_off - 2;
			info.indices[idx_off + 1] = vtx_off - 1;
			info.indices[idx_off + 2] = vtx_off + 1;
			info.indices[idx_off + 3] = vtx_off - 2;
			info.indices[idx_off + 4] = vtx_off + 1;
			info.indices[idx_off + 5] = vtx_off + 0;

			vtx_off += 2;
			idx_off += 6;

			n = (_n + first_normal) * 0.5f;

			info.vertices[0] = { pts[0] + n * thickness, vec2(0.5f), color };
			info.vertices[1] = { pts[0] - n * thickness, vec2(0.5f), color };

			info.indices[idx_off + 0] = vtx_off - 2;
			info.indices[idx_off + 1] = vtx_off - 1;
			info.indices[idx_off + 2] = 1;
			info.indices[idx_off + 3] = vtx_off - 2;
			info.indices[idx_off + 4] = 1;
			info.indices[idx_off + 5] = 0;
		}
		else
		{
			info.vertices[vtx_off + 0] = { pts[pt_cnt - 1] + last_normal * thickness, vec2(0.5f), color };
			info.vertices[vtx_off + 1] = { pts[pt_cnt - 1] - last_normal * thickness, vec2(0.5f), color };

			info.indices[idx_off + 0] = vtx_off - 2;
			info.indices[idx_off + 1] = vtx_off - 1;
			info.indices[idx_off + 2] = vtx_off + 1;
			info.indices[idx_off + 3] = vtx_off - 2;
			info.indices[idx_off + 4] = vtx_off + 1;
			info.indices[idx_off + 5] = vtx_off + 0;

			vtx_off += 2;
			idx_off += 6;
		}
	}

	void sRendererPrivate::draw_glyphs(uint layer, uint cnt, const graphics::GlyphDraw* glyphs, uint res_id, const cvec4& color)
	{
		if (cnt == 0)
			return;

		auto& ed = *_ed;

		auto vtx_cnt = 4 * cnt;
		auto idx_cnt = 6 * cnt;
		ed.buf_vtx.stag_num += vtx_cnt;
		ed.buf_idx.stag_num += idx_cnt;

		auto& info = ed.layers[layer].emplace_back();
		info.res = res_id;
		info.vertices.resize(vtx_cnt);
		info.indices.resize(idx_cnt);

		auto vtx_off = 0;
		auto idx_off = 0;
		for (auto i = 0; i < cnt; i++)
		{
			auto& g = glyphs[i];

			info.vertices[vtx_off + 0] = { g.points[0],	g.uvs.xy(), color };
			info.vertices[vtx_off + 1] = { g.points[1],	g.uvs.xw(), color };
			info.vertices[vtx_off + 2] = { g.points[2],	g.uvs.zw(), color };
			info.vertices[vtx_off + 3] = { g.points[3],	g.uvs.zy(), color };

			info.indices[idx_off + 0] = vtx_off + 0;
			info.indices[idx_off + 1] = vtx_off + 2;
			info.indices[idx_off + 2] = vtx_off + 1;
			info.indices[idx_off + 3] = vtx_off + 0;
			info.indices[idx_off + 4] = vtx_off + 3;
			info.indices[idx_off + 5] = vtx_off + 2;

			vtx_off += 4;
			idx_off += 6;
		}
	}

	void sRendererPrivate::draw_image(uint layer, const vec2* pts, uint res_id, const vec4& uvs, const cvec4& tint_color)
	{
		auto& ed = *_ed;

		ed.buf_vtx.stag_num += 4;
		ed.buf_idx.stag_num += 6;

		auto& info = ed.layers[layer].emplace_back();
		info.res = res_id;
		info.vertices.resize(4);
		info.indices.resize(6);

		info.vertices[0] = { pts[0], uvs.xy(), tint_color };
		info.vertices[1] = { pts[1], uvs.zy(), tint_color };
		info.vertices[2] = { pts[2], uvs.zw(), tint_color };
		info.vertices[3] = { pts[3], uvs.xw(), tint_color };

		info.indices[0] = 0;
		info.indices[1] = 2;
		info.indices[2] = 1;
		info.indices[3] = 0;
		info.indices[4] = 3;
		info.indices[5] = 2;
	}

	int sRendererPrivate::set_texture_res(int idx, graphics::ImageView* tex, graphics::Sampler* sp)
	{
		auto& nd = *_nd;

		if (idx == -1)
		{
			for (auto i = 0; i < nd.tex_reses.size(); i++)
			{
				if (!nd.tex_reses[i])
				{
					idx = i;
					break;
				}
			}
		}
		if (idx == -1)
			return -1;

		nd.tex_reses[idx] = tex;
		nd.ds_material->set_image(DSL_material::maps_binding, idx, tex ? tex : img_white->get_view(), sp ? sp : sp_linear);
		nd.ds_material->update();

		return idx;
	}

	int sRendererPrivate::find_texture_res(graphics::ImageView* tex) const
	{
		auto& nd = *_nd;
		for (auto i = 0; i < nd.tex_reses.size(); i++)
		{
			if (nd.tex_reses[i] == tex)
				return i;
		}
		return -1;
	}

	graphics::Pipeline* MaterialRes::get_pl(sRendererPrivate* thiz, MaterialUsage u)
	{
		if (pls[u])
			return pls[u];
		pls[u] = thiz->get_material_pipeline(u, pipeline_file, pipeline_defines);
		return pls[u];
	}

	int sRendererPrivate::set_material_res(int idx, graphics::Material* mat)
	{
		auto& nd = *_nd;

		if (idx == -1)
		{
			for (auto i = (uint)MaterialCustom; i < nd.mat_reses.size(); i++)
			{
				if (!nd.mat_reses[i].mat)
				{
					idx = i;
					break;
				}
			}
		}
		if (idx == -1)
			return -1;

		auto& dst = nd.mat_reses[idx];
		if (dst.mat)
		{
			for (auto i = 0; i < 4; i++)
			{
				if (dst.texs[i] != -1)
					set_texture_res(dst.texs[i], nullptr, nullptr);
			}
			for (auto i = 0; i < _countof(dst.pls); i++)
			{
				if (dst.pls[i])
					release_material_pipeline((MaterialUsage)i, dst.pls[i]);
			}
		}
		dst.mat = mat;
		{
			wchar_t buf[260];
			mat->get_pipeline_file(buf);
			dst.pipeline_file = buf;
		}
		dst.pipeline_defines = mat->get_pipeline_defines();
		if (mat)
		{
			graphics::InstanceCB cb(device);

			auto& data = nd.buf_materials.item(idx);
			data.color = mat->get_color();
			data.metallic = mat->get_metallic();
			data.roughness = mat->get_roughness();
			data.alpha_test = mat->get_alpha_test();
			for (auto i = 0; i < 4; i++)
			{
				wchar_t buf[260]; buf[0] = 0;
				mat->get_texture_file(i, buf);
				auto fn = std::filesystem::path(buf);
				if (fn.empty())
				{
					dst.texs[i] = -1;
					data.map_indices[i] = -1;
				}
				else
				{
					auto img = graphics::Image::get(device, fn.c_str(), mat->get_texture_srgb(i));
					auto iv = img->get_view();
					auto id = find_texture_res(iv);
					if (id == -1)
						id = set_texture_res(-1, iv, mat->get_texture_sampler(device, i));
					dst.texs[i] = id;
					data.map_indices[i] = id;
				}
			}

			nd.buf_materials.upload(cb.get());
		}

		nd.max_mat_id = max((uint)idx, nd.max_mat_id);
		return idx;
	}

	int sRendererPrivate::find_material_res(graphics::Material* mat) const
	{
		auto& nd = *_nd;
		for (auto i = 0; i < nd.mat_reses.size(); i++)
		{
			if (nd.mat_reses[i].mat == mat)
				return i;
		}
		return -1;
	}

	int sRendererPrivate::set_mesh_res(int idx, graphics::Mesh* mesh)
	{
		auto& nd = *_nd;

		if (idx == -1)
		{
			for (auto i = 0; i < nd.mesh_reses.size(); i++)
			{
				if (!nd.mesh_reses[i].mesh)
				{
					idx = i;
					break;
				}
			}
		}
		if (idx == -1)
			return -1;

		auto& dst = nd.mesh_reses[idx];
		if (dst.mesh)
		{
			// TODO: mark uploaded vertexs invalid
		}
		dst.mesh = mesh;
		if (mesh)
		{
			graphics::InstanceCB cb(device);

			dst.vtx_cnt = mesh->get_vertices_count();
			dst.idx_cnt = mesh->get_indices_count();
			auto apos = mesh->get_positions();
			auto auv = mesh->get_uvs();
			auto anormal = mesh->get_normals();
			auto abids = mesh->get_bone_ids();
			auto abwgts = mesh->get_bone_weights();
			auto aidx = mesh->get_indices();

			if (!abids)
			{
				dst.arm = false;

				dst.vtx_off = nd.buf_mesh_vtx.n1;
				auto pvtx = nd.buf_mesh_vtx.alloc(dst.vtx_cnt);
				for (auto i = 0; i < dst.vtx_cnt; i++)
				{
					auto& vtx = pvtx[i];
					vtx.pos = apos[i];
					vtx.uv = auv ? auv[i] : vec2(0.f);
					vtx.normal = anormal ? anormal[i] : vec3(1.f, 0.f, 0.f);
				}

				dst.idx_off = nd.buf_mesh_idx.n1;
				memcpy(nd.buf_mesh_idx.alloc(dst.idx_cnt), aidx, sizeof(uint) * dst.idx_cnt);

				nd.buf_mesh_vtx.upload(cb.get());
				nd.buf_mesh_idx.upload(cb.get());
			}
			else
			{
				dst.arm = true;

				dst.vtx_off = nd.buf_arm_mesh_vtx.n1;
				auto pvtx = nd.buf_arm_mesh_vtx.alloc(dst.vtx_cnt);

				for (auto i = 0; i < dst.vtx_cnt; i++)
				{
					auto& vtx = pvtx[i];
					vtx.pos = apos[i];
					vtx.uv = auv ? auv[i] : vec2(0.f);
					vtx.normal = anormal ? anormal[i] : vec3(1.f, 0.f, 0.f);
					vtx.ids = abids ? abids[i] : ivec4(-1);
					vtx.weights = abwgts ? abwgts[i] : vec4(0.f);
				}

				dst.idx_off = nd.buf_arm_mesh_idx.n1;
				memcpy(nd.buf_arm_mesh_idx.alloc(dst.idx_cnt), aidx, sizeof(uint) * dst.idx_cnt);

				nd.buf_arm_mesh_vtx.upload(cb.get());
				nd.buf_arm_mesh_idx.upload(cb.get());
			}

			auto mat = mesh->get_material();
			auto mid = find_material_res(mat);
			if (mid == -1)
				mid = set_material_res(-1, mat);
			dst.mat_id = mid;
		}

		return idx;
	}

	int sRendererPrivate::find_mesh_res(graphics::Mesh* mesh) const
	{
		auto& nd = *_nd;
		for (auto i = 0; i < nd.mesh_reses.size(); i++)
		{
			if (nd.mesh_reses[i].mesh == mesh)
				return i;
		}
		return -1;
	}

	graphics::Pipeline* sRendererPrivate::get_material_pipeline(MaterialUsage usage, const std::filesystem::path& mat, const std::string& _defines)
	{
		auto& nd = *_nd;

		auto defines = graphics::Shader::format_defines(_defines);

		for (auto& p : nd.pl_mats[usage])
		{
			if (p.mat == mat && p.defines == defines)
			{
				p.ref_count++;
				return p.pipeline.get();
			}
		}

		graphics::Pipeline* ret = nullptr;

		std::vector<std::pair<std::string, std::string>> substitutes;
		auto polygon_mode = graphics::PolygonModeFill;
		auto cull_mode = graphics::CullModeBack;
		auto depth_test = true;
		auto depth_write = true;
		auto use_mat = true;
		auto deferred = true;

		graphics::Renderpass* rp = nullptr;

		auto find_define = [&](const std::string& s) {
			for (auto& d : defines)
			{
				if (d == s)
					return true;
			}
			return false;
		};
		if (find_define("WIREFRAME"))
		{
			use_mat = false;
			polygon_mode = graphics::PolygonModeLine;
			depth_test = false;
			depth_write = false;
			deferred = false;
			rp = graphics::Renderpass::get(device, L"rgba16.rp");
		}
		else if (find_define("PICKUP"))
		{
			use_mat = false;
			deferred = false;
			rp = graphics::Renderpass::get(device, L"rgba16.rp");
		}
		else if (find_define("OUTLINE"))
		{
			use_mat = false;
			depth_test = false;
			depth_write = false;
			deferred = false;
			rp = graphics::Renderpass::get(device, L"rgba16.rp");
		}
		else if (find_define("NORMAL_DATA"))
		{
			use_mat = false;
			deferred = false;
			rp = graphics::Renderpass::get(device, L"rgba16c_d16c.rp");
		}
		if (use_mat && !mat.empty())
		{
			defines.push_back("MAT");
			substitutes.emplace_back("MAT_FILE", mat.string());
		}

		auto get_defines_str = [&]() {
			std::string ret;
			for (auto& t : defines)
			{
				ret += t;
				ret += ' ';
			}
			return ret;
		};

		auto get_substitutes_str = [&]() {
			std::string ret;
			for (auto& t : substitutes)
			{
				ret += t.first;
				ret += ' ';
				ret += t.second;
				ret += ' ';
			}
			return ret;
		};

		switch (usage)
		{
		case MaterialForMeshShadow:
			deferred = false;
			defines.push_back("SHADOW_PASS");
			rp = graphics::Renderpass::get(device, L"d16.rp");
		case MaterialForMesh:
		{
			if (deferred)
				defines.push_back("DEFERRED");
			auto defines_str = get_defines_str();
			auto substitutes_str = get_substitutes_str();
			graphics::GraphicsPipelineInfo info;
			graphics::Shader* shaders[] = {
				graphics::Shader::get(device, L"mesh/mesh.vert", defines_str.c_str(), substitutes_str.c_str()),
				graphics::Shader::get(device, L"mesh/mesh.frag", defines_str.c_str(), substitutes_str.c_str())
			};
			info.shaders_count = _countof(shaders);
			info.shaders = shaders;
			info.layout = graphics::PipelineLayout::get(device, deferred ? L"mesh/gbuffer.pll" : L"mesh/forward.pll");
			if (!rp)
				rp = graphics::Renderpass::get(device, L"gbuffer.rp");
			info.renderpass = rp;
			info.subpass_index = 0;
			graphics::VertexAttributeInfo vias[3];
			vias[0].location = 0;
			vias[0].format = graphics::Format_R32G32B32_SFLOAT;
			vias[1].location = 1;
			vias[1].format = graphics::Format_R32G32_SFLOAT;
			vias[2].location = 2;
			vias[2].format = graphics::Format_R32G32B32_SFLOAT;
			graphics::VertexBufferInfo vib;
			vib.attributes_count = _countof(vias);
			vib.attributes = vias;
			info.vertex_buffers_count = 1;
			info.vertex_buffers = &vib;
			info.polygon_mode = polygon_mode;
			info.cull_mode = cull_mode;
			info.depth_test = depth_test;
			info.depth_write = depth_write;
			ret = graphics::Pipeline::create(device, info);
		}
			break;
		case MaterialForMeshShadowArmature:
			deferred = false;
			defines.push_back("SHADOW_PASS");
			rp = graphics::Renderpass::get(device, L"d16.rp");
		case MaterialForMeshArmature:
		{
			if (deferred)
				defines.push_back("DEFERRED");
			defines.push_back("ARMATURE");
			auto defines_str = get_defines_str();
			auto substitutes_str = get_substitutes_str();
			graphics::GraphicsPipelineInfo info;
			graphics::Shader* shaders[] = {
				graphics::Shader::get(device, L"mesh/mesh.vert", defines_str.c_str(), substitutes_str.c_str()),
				graphics::Shader::get(device, L"mesh/mesh.frag", defines_str.c_str(), substitutes_str.c_str())
			};
			info.shaders_count = _countof(shaders);
			info.shaders = shaders;
			info.layout = graphics::PipelineLayout::get(device, deferred ? L"mesh/gbuffer.pll" : L"mesh/forward.pll");
			if (!rp)
				rp = graphics::Renderpass::get(device, L"gbuffer.rp");
			info.renderpass = rp;
			info.subpass_index = 0;
			graphics::VertexAttributeInfo vias[5];
			vias[0].location = 0;
			vias[0].format = graphics::Format_R32G32B32_SFLOAT;
			vias[1].location = 1;
			vias[1].format = graphics::Format_R32G32_SFLOAT;
			vias[2].location = 2;
			vias[2].format = graphics::Format_R32G32B32_SFLOAT;
			vias[3].location = 3;
			vias[3].format = graphics::Format_R32G32B32A32_INT;
			vias[4].location = 4;
			vias[4].format = graphics::Format_R32G32B32A32_SFLOAT;
			graphics::VertexBufferInfo vib;
			vib.attributes_count = _countof(vias);
			vib.attributes = vias;
			info.vertex_buffers_count = 1;
			info.vertex_buffers = &vib;
			info.polygon_mode = polygon_mode;
			info.cull_mode = cull_mode;
			info.depth_test = depth_test;
			info.depth_write = depth_write;
			ret = graphics::Pipeline::create(device, info);
		}
			break;
		case MaterialForTerrain:
		{
			if (deferred)
				defines.push_back("DEFERRED");
			auto defines_str = get_defines_str();
			auto substitutes_str = get_substitutes_str();
			graphics::GraphicsPipelineInfo info;
			graphics::Shader* shaders[] = {
				graphics::Shader::get(device, L"terrain/terrain.vert", defines_str.c_str(), substitutes_str.c_str()),
				graphics::Shader::get(device, L"terrain/terrain.tesc", defines_str.c_str(), substitutes_str.c_str()),
				graphics::Shader::get(device, L"terrain/terrain.tese", defines_str.c_str(), substitutes_str.c_str()),
				graphics::Shader::get(device, L"terrain/terrain.frag", defines_str.c_str(), substitutes_str.c_str())
			};
			info.shaders_count = _countof(shaders);
			info.shaders = shaders;
			info.layout = graphics::PipelineLayout::get(device, deferred ? L"terrain/gbuffer.pll" : L"terrain/forward.pll");
			info.renderpass = graphics::Renderpass::get(device, deferred ? L"gbuffer.rp" : L"forward.rp");
			info.subpass_index = 0;
			info.primitive_topology = graphics::PrimitiveTopologyPatchList;
			info.patch_control_points = 4;
			info.polygon_mode = polygon_mode;
			info.cull_mode = cull_mode;
			info.depth_test = depth_test;
			info.depth_write = depth_write;
			ret = graphics::Pipeline::create(device, info);
		}
			break;
		}

		MaterialPipeline mp;
		mp.mat = mat;
		mp.defines = defines;
		mp.pipeline.reset(ret);
		nd.pl_mats[usage].push_back(std::move(mp));
		return ret;
	}

	void sRendererPrivate::release_material_pipeline(MaterialUsage usage, graphics::Pipeline* pl)
	{
		fassert(0);

		auto& nd = *_nd;
		for (auto it = nd.pl_mats[usage].begin(); it != nd.pl_mats[usage].end(); it++)
		{
			// TODO: fix this (requires that all kinds of graphics resources are able to destroy when ref_count==0)
			//if (it->pipeline.get() == pl)
			//{
			//	for (auto s : p->shaders)
			//		s->release();
			//	if (it->ref_count == 1)
			//		material_pipelines[usage].erase(it);
			//	else
			//		it->ref_count--;
			//	break;
			//}
		}
	}

	void sRendererPrivate::set_sky(graphics::ImageView* box, graphics::ImageView* irr,
		graphics::ImageView* rad, graphics::ImageView* lut, const vec3& fog_color, float intensity, void* id)
	{
		sky_id = id;

		auto& nd = *_nd;

		auto iv_black = img_black->get_view();
		auto iv_black_cube = img_black_cube->get_view({ 0, 1, 0, 6 });
		nd.ds_light->set_image(DSL_light::sky_box_binding, 0, box ? box : iv_black_cube, sp_linear);
		nd.ds_light->set_image(DSL_light::sky_irr_binding, 0, irr ? irr : iv_black_cube, sp_linear);
		nd.ds_light->set_image(DSL_light::sky_rad_binding, 0, rad ? rad : iv_black_cube, sp_linear);
		nd.ds_light->set_image(DSL_light::sky_lut_binding, 0, lut ? lut : iv_black, sp_linear);
		nd.ds_light->update();

		auto& data = *nd.buf_render_data.pstag;
		data.fog_color = fog_color;
		data.sky_intensity = intensity;
		data.sky_rad_levels = rad->get_sub().level_count;
	}

	void sRendererPrivate::add_light(const mat4& mat, LightType type, const vec3& color, bool cast_shadow)
	{
		auto& nd = *_nd;

		auto idx = nd.buf_light_infos.n;

		// TODO
		{
			auto& data = nd.buf_light_infos.add_item();
			data.color = color;
			data.shadow_index = -1;

			auto& tile = nd.buf_tile_lights.item(0, false);
			switch (type)
			{
			case LightDirectional:
			{
				auto rot = mat3(mat);
				data.pos = -rot[2];
				if (tile.dir_count < _countof(tile.dir_indices))
				{
					tile.dir_indices[tile.dir_count] = idx;
					tile.dir_count++;
				}
				if (cast_shadow)
				{
					if (nd.dir_shadows.size() < 4)
					{
						data.shadow_index = nd.dir_shadows.size();
						nd.dir_shadows.emplace_back(idx, rot);
					}
				}
			}
				break;
			case LightPoint:
			{
				auto pos = vec3(mat[3]);
				data.pos = pos;
				if (tile.pt_count < _countof(tile.pt_indices))
				{
					tile.pt_indices[tile.pt_count] = idx;
					tile.pt_count++;
				}
				if (cast_shadow)
				{
					if (nd.pt_shadows.size() < 4)
					{
						data.shadow_index = nd.pt_shadows.size();
						nd.pt_shadows.emplace_back(idx, pos);
					}
				}
			}
				break;
			}
		}
	}

	uint sRendererPrivate::add_mesh_transform(const mat4& mat, const mat3& nor)
	{
		auto& nd = *_nd;

		auto idx = nd.buf_mesh_transforms.n;

		auto& data = nd.buf_mesh_transforms.add_item();
		data.mat = mat;
		data.nor = nor;

		return idx;
	}

	uint sRendererPrivate::add_mesh_armature(uint bones_count, const mat4* bones)
	{
		auto& nd = *_nd;

		auto idx = nd.buf_mesh_armatures.n;
		auto& data = nd.buf_mesh_armatures.add_item_overwrite_size(sizeof(mat4) * bones_count);
		memcpy(data.bones, bones, sizeof(mat4) * bones_count);
		return idx;
	}

	void sRendererPrivate::draw_mesh(uint idx, uint mesh_id, ShadingFlags flags)
	{
		auto& nd = *_nd;

		if (render_type != ShadingMaterial)
			flags = (ShadingFlags)(flags & ~ShadingMaterial);

		auto& mesh = nd.mesh_reses[mesh_id];
		auto usage = mesh.arm ? MaterialForMeshArmature : MaterialForMesh;

		if (flags & ShadingMaterial)
			nd.meshes[usage][mesh.mat_id].emplace_back(idx, mesh_id);
		if (render_type == RenderWireframe || (flags & ShadingWireframe))
			nd.meshes[usage][MaterialWireframe].emplace_back(idx, mesh_id);
		if (flags & ShadingOutline)
			nd.meshes[usage][MaterialOutline].emplace_back(idx, mesh_id);
		if (render_type == RenderNormalData)
			nd.meshes[usage][MaterialNormalData].emplace_back(idx, mesh_id);
	}

	void sRendererPrivate::add_mesh_occluder(uint idx, uint mesh_id)
	{
		auto& nd = *_nd;

		auto& mesh = nd.mesh_reses[mesh_id];
		nd.meshes[mesh.arm ? MaterialForMeshShadowArmature : MaterialForMeshShadow][mesh.mat_id].emplace_back(idx, mesh_id);
	}

	void sRendererPrivate::draw_terrain(const vec3& coord, const vec3& extent, const uvec2& blocks, uint tess_levels, uint height_map_id, uint normal_map_id,
		uint material_id, ShadingFlags flags)
	{
		auto& nd = *_nd;

		if (render_type != ShadingMaterial)
			flags = (ShadingFlags)(flags & ~ShadingMaterial);

		auto& data = nd.buf_terrain.add_item();
		data.coord = coord;
		data.extent = extent;
		data.blocks = blocks;
		data.tess_levels = tess_levels;
		data.height_map_id = height_map_id;
		data.normal_map_id = normal_map_id;
		data.material_id = material_id;

		auto dispatch_count = blocks.x * blocks.y;
		if (flags & ShadingMaterial)
			nd.terrains[MaterialCustom].emplace_back(dispatch_count, material_id);
		if (render_type == RenderWireframe || (flags & ShadingWireframe))
			nd.terrains[MaterialWireframe].emplace_back(dispatch_count, material_id);
		if (flags & ShadingOutline)
			nd.terrains[MaterialOutline].emplace_back(dispatch_count, material_id);
		if (render_type == RenderNormalData)
			nd.terrains[MaterialNormalData].emplace_back(dispatch_count, material_id);
	}

	void sRendererPrivate::draw_particles(uint count, graphics::Particle* partcles, uint res_id)
	{
		auto& nd = *_nd;

		if (nd.particles.back().first != res_id)
			nd.particles.emplace_back(res_id, 0);

		auto base = nd.particles.back().second;

		auto pvtx = nd.buf_ptc_vtx.stag(count * 4);
		for (auto i = 0; i < count; i++)
		{
			auto& p = partcles[i];
			pvtx[i * 4 + 0] = { p.pos - p.xext - p.yext, p.uvs.xy(), p.col };
			pvtx[i * 4 + 1] = { p.pos + p.xext - p.yext, p.uvs.zy(), p.col };
			pvtx[i * 4 + 2] = { p.pos + p.xext + p.yext, p.uvs.zw(), p.col };
			pvtx[i * 4 + 3] = { p.pos - p.xext + p.yext, p.uvs.xw(), p.col };
		}

		auto pidx = nd.buf_ptc_idx.stag(count * 6);
		for (auto i = 0; i < count; i++)
		{
			pidx[i * 6 + 0] = base + i * 4 + 0;
			pidx[i * 6 + 1] = base + i * 4 + 2;
			pidx[i * 6 + 2] = base + i * 4 + 1;
			pidx[i * 6 + 3] = base + i * 4 + 0;
			pidx[i * 6 + 4] = base + i * 4 + 3;
			pidx[i * 6 + 5] = base + i * 4 + 2;
		}

		nd.particles.back().second += count;
	}

	void sRendererPrivate::draw_lines(uint count, graphics::Line* lines)
	{
		auto& nd = *_nd;

		memcpy(nd.buf_lines.stag(count), lines, count * sizeof(graphics::Line));
	}

	void sRendererPrivate::set_targets(uint tar_cnt, graphics::ImageView* const* ivs)
	{
		iv_tars.resize(tar_cnt);
		for (auto i = 0; i < tar_cnt; i++)
			iv_tars[i] = ivs[i];

		fb_tars.clear();
		fb_tars.resize(tar_cnt);
		for (auto i = 0; i < tar_cnt; i++)
			fb_tars[i].reset(graphics::Framebuffer::create(device, rp_bgra8, 1, &ivs[i]));

		if (tar_cnt == 0)
			return;

		tar_sz = ivs[0]->get_image()->get_size();

		img_dst.reset(graphics::Image::create(device, graphics::Format_R16G16B16A16_SFLOAT, tar_sz, 1, 1,
			graphics::SampleCount_1, graphics::ImageUsageSampled | graphics::ImageUsageAttachment));

		auto& nd = *_nd;

		nd.img_dep.reset(graphics::Image::create(device, graphics::Format_Depth16, tar_sz, 1, 1,
			graphics::SampleCount_1, graphics::ImageUsageSampled | graphics::ImageUsageAttachment));
		nd.img_col_met.reset(graphics::Image::create(device, graphics::Format_R8G8B8A8_UNORM, tar_sz, 1, 1,
			graphics::SampleCount_1, graphics::ImageUsageSampled | graphics::ImageUsageAttachment));
		nd.img_nor_rou.reset(graphics::Image::create(device, graphics::Format_R8G8B8A8_UNORM, tar_sz, 1, 1,
			graphics::SampleCount_1, graphics::ImageUsageSampled | graphics::ImageUsageAttachment));

		{
			graphics::ImageView* vs[] = {
				nd.img_col_met->get_view(),
				nd.img_nor_rou->get_view(),
				nd.img_dep->get_view()
			};
			nd.fb_gbuf.reset(graphics::Framebuffer::create(device, graphics::Renderpass::get(device, L"gbuffer.rp"), _countof(vs), vs));
		}

		{
			graphics::ImageView* vs[] = {
				img_dst->get_view(),
				nd.img_dep->get_view()
			};
			nd.fb_fwd.reset(graphics::Framebuffer::create(device, graphics::Renderpass::get(device, L"rgba16c_d16c.rp"), _countof(vs), vs));
			nd.fb_fwd_dc.reset(graphics::Framebuffer::create(device, graphics::Renderpass::get(device, L"rgba16_d16.rp"), _countof(vs), vs));
		}

		nd.ds_def->set_image(DSL_deferred::img_col_met_binding, 0, nd.img_col_met->get_view(), sp_nearest);
		nd.ds_def->set_image(DSL_deferred::img_nor_rou_binding, 0, nd.img_nor_rou->get_view(), sp_nearest);
		nd.ds_def->set_image(DSL_deferred::img_dep_binding, 0, nd.img_dep->get_view(), sp_nearest);
		nd.ds_def->update();

		nd.img_back.reset(graphics::Image::create(device, graphics::Format_R16G16B16A16_SFLOAT, tar_sz, 0, 1,
			graphics::SampleCount_1, graphics::ImageUsageSampled | graphics::ImageUsageAttachment));
	}

	const auto shadow_map_size = uvec2(1024);

	void sRendererPrivate::record(uint tar_idx, graphics::CommandBuffer* cb)
	{
		auto& nd = *_nd;
		if (nd.should_render)
		{
			{
				camera->set_screen_size(tar_sz);
				camera->update_view();
				camera->update_proj();

				auto& data = *(nd.buf_render_data.pstag);
				data.zNear = camera->near;
				data.zFar = camera->far;
				data.camera_coord = camera->node->g_pos;
				data.view = camera->view;
				data.view_inv = camera->view_inv;
				data.proj = camera->proj;
				data.proj_inv = camera->proj_inv;
				data.proj_view = data.proj * data.view;
				auto frustum = camera->get_frustum();
				memcpy(data.frustum_planes, frustum.planes, sizeof(vec4) * 6);
			}
			nd.buf_render_data.cpy_whole();
			nd.buf_render_data.upload(cb);

			typedef std::vector<std::pair<uint, uint>> DrawIndirs;

			auto pack_mesh_indirs = [&](MaterialUsage u, bool upload = true) {
				DrawIndirs ret;
				for (auto mat_id = (uint)MaterialCustom; mat_id <= nd.max_mat_id; mat_id++)
				{
					auto& vec = nd.meshes[u][mat_id];
					if (!vec.empty())
					{
						auto indirs = nd.buf_mesh_indirs[u].stag(vec.size());
						for (auto i = 0; i < vec.size(); i++)
						{
							auto& src = nd.mesh_reses[vec[i].second];
							auto& dst = indirs[i];
							dst.vertex_offset = src.vtx_off;
							dst.first_index = src.idx_off;
							dst.index_count = src.idx_cnt;
							dst.first_instance = (vec[i].first << 16) + mat_id;
							dst.instance_count = 1;
						}
						ret.emplace_back(mat_id, (uint)vec.size());
						vec.clear();
					}
				}
				if (upload)
					nd.buf_mesh_indirs[u].upload(cb);
				return ret;
			};

			auto bind_mesh_fwd_res = [&]() {
				cb->bind_pipeline_layout(nd.pll_mesh_fwd);
				graphics::DescriptorSet* sets[mesh::PLL_forward::Binding_Max];
				sets[mesh::PLL_forward::Binding_render_data] = nd.ds_render_data.get();
				sets[mesh::PLL_forward::Binding_material] = nd.ds_material.get();
				sets[mesh::PLL_forward::Binding_light] = nd.ds_light.get();
				sets[mesh::PLL_forward::Binding_mesh] = nd.ds_mesh.get();
				cb->bind_descriptor_sets(0, _countof(sets), sets);
			};

			auto bind_mesh_def_res = [&]() {
				cb->bind_pipeline_layout(nd.pll_mesh_gbuf);
				graphics::DescriptorSet* sets[mesh::PLL_gbuffer::Binding_Max];
				sets[mesh::PLL_gbuffer::Binding_render_data] = nd.ds_render_data.get();
				sets[mesh::PLL_gbuffer::Binding_material] = nd.ds_material.get();
				sets[mesh::PLL_gbuffer::Binding_mesh] = nd.ds_mesh.get();
				cb->bind_descriptor_sets(0, _countof(sets), sets);
			};

			auto draw_meshes = [&](MaterialUsage u, const std::vector<std::pair<uint, uint>>& indirs, uint off = 0) {
				auto buf = nd.buf_mesh_indirs[u].buf.get();
				for (auto& i : indirs)
				{
					cb->bind_pipeline(nd.mat_reses[i.first].get_pl(this, u));
					cb->draw_indexed_indirect(buf, off, i.second);
					off += i.second;
				}
				return off;
			};

			auto bind_terrain_fwd_res = [&]() {
				cb->bind_pipeline_layout(nd.pll_terrain_fwd);
				graphics::DescriptorSet* sets[mesh::PLL_forward::Binding_Max];
				sets[terrain::PLL_forward::Binding_render_data] = nd.ds_render_data.get();
				sets[terrain::PLL_forward::Binding_material] = nd.ds_material.get();
				sets[terrain::PLL_forward::Binding_light] = nd.ds_light.get();
				sets[terrain::PLL_forward::Binding_terrain] = nd.ds_terrain.get();
				cb->bind_descriptor_sets(0, _countof(sets), sets);
			};

			auto bind_terrain_def_res = [&]() {
				cb->bind_pipeline_layout(nd.pll_terrain_gbuf);
				graphics::DescriptorSet* sets[mesh::PLL_gbuffer::Binding_Max];
				sets[terrain::PLL_gbuffer::Binding_render_data] = nd.ds_render_data.get();
				sets[terrain::PLL_gbuffer::Binding_material] = nd.ds_material.get();
				sets[terrain::PLL_gbuffer::Binding_terrain] = nd.ds_terrain.get();
				cb->bind_descriptor_sets(0, _countof(sets), sets);
			};

			auto draw_terrains = [&]() {
				auto& vec = nd.terrains[MaterialCustom];
				for (auto i = 0; i < vec.size(); i++)
				{
					cb->bind_pipeline(nd.mat_reses[vec[i].second].get_pl(this, MaterialForTerrain));
					cb->draw(4, vec[i].first, 0, i << 16);
				}
				vec.clear();
			};

			std::function<void(cNodePrivate*, const Frustum&)> collect_occluders;
			collect_occluders = [&](cNodePrivate* node, const Frustum& frustum) {
				auto e = node->entity;

				node->update_transform();
				if (!node->bounds_invalid && AABB_frustum_check(frustum, node->bounds))
					node->draw(frame, true);

				if (node->octree)
				{
					std::vector<cNodePrivate*> objs;
					node->octree->get_within_frustum(frustum, objs);
					for (auto obj : objs)
						collect_occluders(obj, frustum);
				}
				else
				{
					for (auto& c : e->children)
					{
						if (!c->global_visibility)
							continue;

						auto cnode = c->get_component_i<cNodePrivate>(0);
						if (cnode)
							collect_occluders(cnode, frustum);
					}
				}
			};

			std::vector<std::vector<DrawIndirs>> dir_shadow_mesh_indirs;
			std::vector<std::vector<DrawIndirs>> dir_shadow_mesh_arm_indirs;
			std::vector<std::vector<DrawIndirs>> pt_shadow_mesh_indirs;
			std::vector<std::vector<DrawIndirs>> pt_shadow_mesh_arm_indirs;
			if (render_type == RenderShaded)
			{
				for (auto& s : nd.dir_shadows)
				{
					auto rot = s.second;
					rot[2] *= -1.f;
					auto inv = inverse(rot);

					auto& data = nd.buf_dir_shadows.add_item();
					data.far = nd.dir_shadow_dist;

					auto& mesh_indirs_vec = dir_shadow_mesh_indirs.emplace_back();
					auto& mesh_arm_indirs_vec = dir_shadow_mesh_arm_indirs.emplace_back();
					for (auto i = 0; i < 4; i++)
					{
						if (i < nd.dir_shadow_levels)
						{
							auto n = i / (float)nd.dir_shadow_levels;
							n = n * n * data.far;
							auto f = (i + 1) / (float)nd.dir_shadow_levels;
							f = f * f * data.far;

							AABB b; b.reset();
							vec3 ps[8];
							camera->get_points(ps, n, f);
							for (auto k = 0; k < 8; k++)
								b.expand(inv * ps[k]);
							auto hf_xlen = (b.b.x - b.a.x) * 0.5f;
							auto hf_ylen = (b.b.y - b.a.y) * 0.5f;
							auto c = rot * b.center();

							auto proj = orthoRH(-hf_xlen, +hf_xlen, -hf_ylen, +hf_ylen, 0.f, data.far);
							proj[1][1] *= -1.f;
							auto view = lookAt(c + rot[2] * data.far * 0.5f, c, rot[1]);

							auto mat = proj * view;
							data.mats[i] = mat;
							data.splits[i] = f;

							collect_occluders(world->first_node->get_component_i<cNodePrivate>(0), Frustum(inverse(mat)));
							mesh_indirs_vec.push_back(pack_mesh_indirs(MaterialForMeshShadow, false));
							mesh_arm_indirs_vec.push_back(pack_mesh_indirs(MaterialForMeshShadowArmature, false));
						}
						else
							data.splits[i] = 0.f;
					}
				}

				for (auto& s : nd.pt_shadows)
				{
					auto near = 0.1f;

					auto proj = perspective(radians(90.f), near, 1.f, nd.pt_shadow_dist);
					proj[1][1] *= -1.f;

					auto& data = nd.buf_pt_shadows.add_item();

					auto& mesh_indirs_vec = pt_shadow_mesh_indirs.emplace_back();
					auto& mesh_arm_indirs_vec = pt_shadow_mesh_arm_indirs.emplace_back();
					for (auto i = 0; i < 6; i++)
					{
						auto& matrix = data.mats[i];
						switch (i)
						{
						case 0:
							matrix[0][0] = -1.f;
							matrix = matrix * proj * lookAt(s.second, s.second + vec3(1.f, 0.f, 0.f), vec3(0.f, 1.f, 0.f));
							break;
						case 1:
							matrix[0][0] = -1.f;
							matrix = matrix * proj * lookAt(s.second, s.second + vec3(-1.f, 0.f, 0.f), vec3(0.f, 1.f, 0.f));
							break;
						case 2:
							matrix[1][1] = -1.f;
							matrix = matrix * proj * lookAt(s.second, s.second + vec3(0.f, 1.f, 0.f), vec3(1.f, 0.f, 0.f));
							break;
						case 3:
							matrix[1][1] = -1.f;
							matrix = matrix * proj * lookAt(s.second, s.second + vec3(0.f, -1.f, 0.f), vec3(0.f, 0.f, -1.f));
							break;
						case 4:
							matrix[0][0] = -1.f;
							matrix = matrix * proj * lookAt(s.second, s.second + vec3(0.f, 0.f, 1.f), vec3(0.f, 1.f, 0.f));
							break;
						case 5:
							matrix[0][0] = -1.f;
							matrix = matrix * proj * lookAt(s.second, s.second + vec3(0.f, 0.f, -1.f), vec3(0.f, 1.f, 0.f));
							break;
						}

						collect_occluders(world->first_node->get_component_i<cNodePrivate>(0), Frustum(inverse(matrix)));
						mesh_indirs_vec.push_back(pack_mesh_indirs(MaterialForMeshShadow, false));
						mesh_arm_indirs_vec.push_back(pack_mesh_indirs(MaterialForMeshShadowArmature, false));

					}
					data.near = near;
					data.far = nd.pt_shadow_dist;
				}
			}

			nd.buf_mesh_transforms.upload(cb);
			nd.buf_mesh_armatures.upload(cb);
			nd.buf_terrain.upload(cb);
			nd.buf_light_infos.upload(cb);
			nd.buf_tile_lights.upload(cb);
			nd.buf_dir_shadows.upload(cb);
			nd.buf_pt_shadows.upload(cb);

			if (render_type == RenderShaded)
			{
				nd.buf_mesh_indirs[MaterialForMeshShadow].upload(cb);
				nd.buf_mesh_indirs[MaterialForMeshShadowArmature].upload(cb);

				cb->set_viewport(Rect(0.f, 0.f, shadow_map_size.x, shadow_map_size.y));
				cb->set_scissor(Rect(0.f, 0.f, shadow_map_size.x, shadow_map_size.y));

				auto dir_mesh_indirs_off = 0;
				auto dir_mesh_arm_indirs_off = 0;
				for (auto i = 0; i < nd.dir_shadows.size(); i++)
				{
					auto& mesh_indirs_vec = dir_shadow_mesh_indirs[i];
					auto& mesh_arm_indirs_vec = dir_shadow_mesh_arm_indirs[i];
					for (auto lv = 0; lv < nd.dir_shadow_levels; lv++)
					{
						auto cv = vec4(1.f, 0.f, 0.f, 0.f);
						cb->begin_renderpass(nullptr, nd.img_dir_shadow_maps[i]->get_shader_write_dst(0, lv, true), &cv);
						bind_mesh_fwd_res();
						cb->push_constant_t(mesh::PLL_forward::PushConstant{ .i = ivec4(0, i, lv, 0) });
						cb->bind_vertex_buffer(nd.buf_mesh_vtx.buf.get(), 0);
						cb->bind_index_buffer(nd.buf_mesh_idx.buf.get(), graphics::IndiceTypeUint);
						dir_mesh_indirs_off = draw_meshes(MaterialForMeshShadow, mesh_indirs_vec[lv], dir_mesh_indirs_off);
						cb->bind_vertex_buffer(nd.buf_arm_mesh_vtx.buf.get(), 0);
						cb->bind_index_buffer(nd.buf_arm_mesh_idx.buf.get(), graphics::IndiceTypeUint);
						dir_mesh_arm_indirs_off = draw_meshes(MaterialForMeshShadowArmature, mesh_arm_indirs_vec[lv], dir_mesh_arm_indirs_off);
						cb->end_renderpass();
					}

					cb->image_barrier(nd.img_dir_shadow_maps[i].get(), { 0U, 1U, 0U, nd.dir_shadow_levels }, graphics::ImageLayoutAttachment, graphics::ImageLayoutShaderReadOnly);
				}

				cb->set_viewport(Rect(0.f, 0.f, shadow_map_size.x * 0.5f, shadow_map_size.y * 0.5f));
				cb->set_scissor(Rect(0.f, 0.f, shadow_map_size.x * 0.5f, shadow_map_size.y * 0.5f));

				auto pt_mesh_indirs_off = 0;
				auto pt_mesh_arm_indirs_off = 0;
				for (auto i = 0; i < nd.pt_shadows.size(); i++)
				{
					auto& mesh_indirs_vec = pt_shadow_mesh_indirs[i];
					auto& mesh_arm_indirs_vec = pt_shadow_mesh_arm_indirs[i];
					for (auto ly = 0; ly < 6; ly++)
					{
						auto cv = vec4(1.f, 0.f, 0.f, 0.f);
						cb->begin_renderpass(nullptr, nd.img_pt_shadow_maps[i]->get_shader_write_dst(0, ly, true), &cv);
						bind_mesh_fwd_res();
						cb->push_constant_t(mesh::PLL_forward::PushConstant{ .i = ivec4(1, i, ly, 0) });
						cb->bind_vertex_buffer(nd.buf_mesh_vtx.buf.get(), 0);
						cb->bind_index_buffer(nd.buf_mesh_idx.buf.get(), graphics::IndiceTypeUint);
						pt_mesh_indirs_off = draw_meshes(MaterialForMeshShadow, mesh_indirs_vec[ly], pt_mesh_indirs_off);
						cb->bind_vertex_buffer(nd.buf_arm_mesh_vtx.buf.get(), 0);
						cb->bind_index_buffer(nd.buf_arm_mesh_idx.buf.get(), graphics::IndiceTypeUint);
						pt_mesh_arm_indirs_off = draw_meshes(MaterialForMeshShadowArmature, mesh_arm_indirs_vec[ly], pt_mesh_arm_indirs_off);
						cb->end_renderpass();
					}

					cb->image_barrier(nd.img_pt_shadow_maps[i].get(), { 0U, 1U, 0U, 6U }, graphics::ImageLayoutAttachment, graphics::ImageLayoutShaderReadOnly);
				}
			}

			nd.dir_shadows.clear();
			nd.pt_shadows.clear();

			auto vp = Rect(vec2(0.f), tar_sz);
			cb->set_viewport(vp);
			cb->set_scissor(vp);

			if (render_type == RenderShaded)
			{
				auto mesh_indirs = pack_mesh_indirs(MaterialForMesh);
				auto mesh_arm_indirs = pack_mesh_indirs(MaterialForMeshArmature);

				vec4 cvs[] = {
					vec4(0.f, 0.f, 0.f, 0.f),
					vec4(0.f, 0.f, 0.f, 0.f),
					vec4(1.f, 0.f, 0.f, 0.f),
					vec4(0.f, 0.f, 0.f, 0.f)
				};
				cb->begin_renderpass(nullptr, nd.fb_gbuf.get(), cvs);

				bind_mesh_def_res();
				cb->bind_vertex_buffer(nd.buf_mesh_vtx.buf.get(), 0);
				cb->bind_index_buffer(nd.buf_mesh_idx.buf.get(), graphics::IndiceTypeUint);
				draw_meshes(MaterialForMesh, mesh_indirs);
				cb->bind_vertex_buffer(nd.buf_arm_mesh_vtx.buf.get(), 0);
				cb->bind_index_buffer(nd.buf_arm_mesh_idx.buf.get(), graphics::IndiceTypeUint);
				draw_meshes(MaterialForMeshArmature, mesh_arm_indirs);
				bind_terrain_def_res();
				draw_terrains();

				cb->end_renderpass();

				cb->image_barrier(nd.img_col_met.get(), {}, graphics::ImageLayoutAttachment, graphics::ImageLayoutShaderReadOnly);
				cb->image_barrier(nd.img_nor_rou.get(), {}, graphics::ImageLayoutAttachment, graphics::ImageLayoutShaderReadOnly);
				cb->image_barrier(nd.img_dep.get(), {}, graphics::ImageLayoutAttachment, graphics::ImageLayoutShaderReadOnly);

				cb->begin_renderpass(nullptr, img_dst->get_shader_write_dst());
				cb->bind_pipeline(nd.pl_def);
				graphics::DescriptorSet* sets[PLL_deferred::Binding_Max];
				sets[PLL_deferred::Binding_deferred] = nd.ds_def.get();
				sets[PLL_deferred::Binding_render_data] = nd.ds_render_data.get();
				sets[PLL_deferred::Binding_light] = nd.ds_light.get();
				cb->bind_descriptor_sets(0, _countof(sets), sets);
				cb->draw(3, 1, 0, 0);
				cb->end_renderpass();
			}
			else
			{
				vec4 cvs[] = {
					vec4(0.f, 0.f, 0.f, 0.f),
					vec4(1.f, 0.f, 0.f, 0.f)
				};
				cb->begin_renderpass(nullptr, nd.fb_fwd.get(), cvs);

				switch (render_type)
				{
				case RenderNormalData:
				{
					auto& meshes = nd.meshes[MaterialForMesh][MaterialNormalData];
					auto& arm_meshes = nd.meshes[MaterialForMeshArmature][MaterialNormalData];
					auto& terrains = nd.terrains[MaterialNormalData];
					if (!meshes.empty() || !arm_meshes.empty() || !terrains.empty())
					{
						if (!meshes.empty())
						{
							cb->bind_vertex_buffer(nd.buf_mesh_vtx.buf.get(), 0);
							cb->bind_index_buffer(nd.buf_mesh_idx.buf.get(), graphics::IndiceTypeUint);
							bind_mesh_fwd_res();
							cb->bind_pipeline(nd.mat_reses[MaterialNormalData].get_pl(this, MaterialForMesh));
							for (auto& m : meshes)
							{
								auto& mr = nd.mesh_reses[m.second];
								cb->draw_indexed(mr.idx_cnt, mr.idx_off, mr.vtx_off, 1, (m.first << 16) + mr.mat_id);
							}
							meshes.clear();
						}
						if (!arm_meshes.empty())
						{
							cb->bind_vertex_buffer(nd.buf_arm_mesh_vtx.buf.get(), 0);
							cb->bind_index_buffer(nd.buf_arm_mesh_idx.buf.get(), graphics::IndiceTypeUint);
							bind_mesh_fwd_res();
							cb->push_constant_t(vec4(0.f, 1.f, 0.f, 1.f));
							cb->bind_pipeline(nd.mat_reses[MaterialNormalData].get_pl(this, MaterialForMeshArmature));
							for (auto& m : arm_meshes)
							{
								auto& mr = nd.mesh_reses[m.second];
								cb->draw_indexed(mr.idx_cnt, mr.idx_off, mr.vtx_off, 1, (m.first << 16) + mr.mat_id);
							}
							arm_meshes.clear();
						}
						if (!terrains.empty())
						{
							bind_terrain_fwd_res();
							cb->push_constant_t(vec4(0.f, 1.f, 0.f, 1.f));
							for (auto i = 0; i < terrains.size(); i++)
							{
								cb->bind_pipeline(nd.mat_reses[MaterialNormalData].get_pl(this, MaterialForTerrain));
								cb->draw(4, terrains[i].first, 0, i << 16);
							}
							terrains.clear();
						}
					}
				}
					break;
				}

				cb->end_renderpass();
			}

			auto& wireframe_meshes = nd.meshes[MaterialForMesh][MaterialWireframe];
			auto& wireframe_arm_meshes = nd.meshes[MaterialForMeshArmature][MaterialWireframe];
			auto& wireframe_terrains = nd.terrains[MaterialWireframe];
			if (!wireframe_meshes.empty() || !wireframe_arm_meshes.empty() || !wireframe_terrains.empty())
			{
				cb->begin_renderpass(nullptr, img_dst->get_shader_write_dst());
				if (!wireframe_meshes.empty())
				{
					cb->bind_vertex_buffer(nd.buf_mesh_vtx.buf.get(), 0);
					cb->bind_index_buffer(nd.buf_mesh_idx.buf.get(), graphics::IndiceTypeUint);
					bind_mesh_fwd_res();
					cb->push_constant_t(vec4(0.f, 1.f, 0.f, 1.f));
					cb->bind_pipeline(nd.mat_reses[MaterialWireframe].get_pl(this, MaterialForMesh));
					for (auto& m : wireframe_meshes)
					{
						auto& mr = nd.mesh_reses[m.second];
						cb->draw_indexed(mr.idx_cnt, mr.idx_off, mr.vtx_off, 1, (m.first << 16) + mr.mat_id);
					}
					wireframe_meshes.clear();
				}
				if (!wireframe_arm_meshes.empty())
				{
					cb->bind_vertex_buffer(nd.buf_arm_mesh_vtx.buf.get(), 0);
					cb->bind_index_buffer(nd.buf_arm_mesh_idx.buf.get(), graphics::IndiceTypeUint);
					bind_mesh_fwd_res();
					cb->push_constant_t(vec4(0.f, 1.f, 0.f, 1.f));
					cb->bind_pipeline(nd.mat_reses[MaterialWireframe].get_pl(this, MaterialForMeshArmature));
					for (auto& m : wireframe_arm_meshes)
					{
						auto& mr = nd.mesh_reses[m.second];
						cb->draw_indexed(mr.idx_cnt, mr.idx_off, mr.vtx_off, 1, (m.first << 16) + mr.mat_id);
					}
					wireframe_arm_meshes.clear();
				}
				if (!wireframe_terrains.empty())
				{
					bind_terrain_fwd_res();
					cb->push_constant_t(vec4(0.f, 1.f, 0.f, 1.f));
					for (auto i = 0; i < wireframe_terrains.size(); i++)
					{
						cb->bind_pipeline(nd.mat_reses[MaterialWireframe].get_pl(this, MaterialForTerrain));
						cb->draw(4, wireframe_terrains[i].first, 0, i << 16);
					}
					wireframe_terrains.clear();
				}
				cb->end_renderpass();
			}

			if (nd.particles.size() > 1)
			{
				nd.buf_ptc_vtx.upload(cb);
				nd.buf_ptc_idx.upload(cb);

				cb->begin_renderpass(nullptr, nd.fb_fwd_dc.get());
				cb->bind_vertex_buffer(nd.buf_ptc_vtx.buf.get(), 0);
				cb->bind_index_buffer(nd.buf_ptc_idx.buf.get(), graphics::IndiceTypeUint);
				cb->bind_pipeline(nd.pl_ptc);
				{
					graphics::DescriptorSet* sets[PLL_particle::Binding_Max];
					sets[PLL_particle::Binding_render_data] = nd.ds_render_data.get();
					sets[PLL_particle::Binding_material] = nd.ds_material.get();
					cb->bind_descriptor_sets(0, _countof(sets), sets);
				}
				auto cnt = 0;
				for (auto& vec : nd.particles)
				{
					if (vec.second == 0)
						continue;
					cb->draw_indexed(vec.second * 6, cnt * 4, cnt * 6, 1, vec.first);
					cnt += vec.second;
				}
				cb->end_renderpass();

				nd.particles.clear();
				nd.particles.emplace_back(0xffff, 0);
			}

			auto& outline_meshes = nd.meshes[MaterialForMesh][MaterialOutline];
			auto& outline_arm_meshes = nd.meshes[MaterialForMeshArmature][MaterialOutline];
			auto& outline_terrains = nd.terrains[MaterialOutline];
			if (!outline_meshes.empty() || !outline_arm_meshes.empty() || !outline_terrains.empty())
			{
				auto cv = vec4(0.f);
				cb->begin_renderpass(nullptr, nd.img_back->get_shader_write_dst(0, 0, true), &cv);
				if (!outline_meshes.empty())
				{
					cb->bind_vertex_buffer(nd.buf_mesh_vtx.buf.get(), 0);
					cb->bind_index_buffer(nd.buf_mesh_idx.buf.get(), graphics::IndiceTypeUint);
					bind_mesh_fwd_res();
					cb->push_constant_t(vec4(1.f, 1.f, 0.f, 1.f));
					cb->bind_pipeline(nd.mat_reses[MaterialOutline].get_pl(this, MaterialForMesh));
					for (auto& m : outline_meshes)
					{
						auto& mr = nd.mesh_reses[m.second];
						cb->draw_indexed(mr.idx_cnt, mr.idx_off, mr.vtx_off, 1, (m.first << 16) + mr.mat_id);
					}
				}
				if (!outline_arm_meshes.empty())
				{
					cb->bind_vertex_buffer(nd.buf_arm_mesh_vtx.buf.get(), 0);
					cb->bind_index_buffer(nd.buf_arm_mesh_idx.buf.get(), graphics::IndiceTypeUint);
					bind_mesh_fwd_res();
					cb->push_constant_t(vec4(1.f, 1.f, 0.f, 1.f));
					cb->bind_pipeline(nd.mat_reses[MaterialOutline].get_pl(this, MaterialForMeshArmature));
					for (auto& m : outline_arm_meshes)
					{
						auto& mr = nd.mesh_reses[m.second];
						cb->draw_indexed(mr.idx_cnt, mr.idx_off, mr.vtx_off, 1, (m.first << 16) + mr.mat_id);
					}
				}
				if (!outline_terrains.empty())
				{
					bind_terrain_fwd_res();
					cb->push_constant_t(vec4(1.f, 1.f, 0.f, 1.f));
					for (auto i = 0; i < outline_terrains.size(); i++)
					{
						cb->bind_pipeline(nd.mat_reses[MaterialOutline].get_pl(this, MaterialForTerrain));
						cb->draw(4, outline_terrains[i].first, 0, i << 16);
					}
				}
				cb->end_renderpass();

				auto lvs = min(nd.img_back->get_levels(), 3U);
				for (auto i = 0U; i < lvs - 1; i++)
				{
					cb->image_barrier(nd.img_back.get(), { i }, graphics::ImageLayoutAttachment, graphics::ImageLayoutShaderReadOnly);
					cb->set_viewport(Rect(vec2(0.f), nd.img_back->get_size(i + 1)));
					cb->begin_renderpass(nullptr, nd.img_back->get_shader_write_dst(i + 1, 0));
					cb->bind_pipeline(nd.pl_downsample);
					cb->bind_descriptor_set(0, nd.img_back->get_shader_read_src(i, 0, sp_linear));
					cb->push_constant_t(1.f / vec2(nd.img_back->get_size(i)));
					cb->draw(3, 1, 0, 0);
					cb->end_renderpass();	
				}
				for (auto i = lvs - 1; i > 0; i--)
				{
					cb->image_barrier(nd.img_back.get(), { i }, graphics::ImageLayoutAttachment, graphics::ImageLayoutShaderReadOnly);
					cb->set_viewport(Rect(vec2(0.f), nd.img_back->get_size(i - 1)));
					cb->begin_renderpass(nullptr, nd.img_back->get_shader_write_dst(i - 1, 0));
					cb->bind_pipeline(nd.pl_upsample);
					cb->bind_descriptor_set(0, nd.img_back->get_shader_read_src(i, 0, sp_linear));
					cb->push_constant_t(1.f / vec2(nd.img_back->get_size(i)));
					cb->draw(3, 1, 0, 0);
					cb->end_renderpass();
				}

				cb->image_barrier(nd.img_back.get(), {}, graphics::ImageLayoutAttachment, graphics::ImageLayoutShaderReadOnly);
				cb->set_viewport(vp);
				cb->begin_renderpass(nullptr, nd.img_back->get_shader_write_dst());
				if (!outline_meshes.empty())
				{
					cb->bind_vertex_buffer(nd.buf_mesh_vtx.buf.get(), 0);
					cb->bind_index_buffer(nd.buf_mesh_idx.buf.get(), graphics::IndiceTypeUint);
					bind_mesh_fwd_res();
					cb->push_constant_t(vec4(0.f, 0.f, 0.f, 1.f));
					cb->bind_pipeline(nd.mat_reses[MaterialOutline].get_pl(this, MaterialForMesh));
					for (auto& m : outline_meshes)
					{
						auto& mr = nd.mesh_reses[m.second];
						cb->draw_indexed(mr.idx_cnt, mr.idx_off, mr.vtx_off, 1, (m.first << 16) + mr.mat_id);
					}
					outline_meshes.clear();
				}
				if (!outline_arm_meshes.empty())
				{
					cb->bind_vertex_buffer(nd.buf_arm_mesh_vtx.buf.get(), 0);
					cb->bind_index_buffer(nd.buf_arm_mesh_idx.buf.get(), graphics::IndiceTypeUint);
					bind_mesh_fwd_res();
					cb->push_constant_t(vec4(0.f, 0.f, 0.f, 1.f));
					cb->bind_pipeline(nd.mat_reses[MaterialOutline].get_pl(this, MaterialForMeshArmature));
					for (auto& m : outline_arm_meshes)
					{
						auto& mr = nd.mesh_reses[m.second];
						cb->draw_indexed(mr.idx_cnt, mr.idx_off, mr.vtx_off, 1, (m.first << 16) + mr.mat_id);
					}
					outline_arm_meshes.clear();
				}
				if (!outline_terrains.empty())
				{
					bind_terrain_fwd_res();
					cb->push_constant_t(vec4(0.f, 0.f, 0.f, 1.f));
					for (auto i = 0; i < outline_terrains.size(); i++)
					{
						cb->bind_pipeline(nd.mat_reses[MaterialOutline].get_pl(this, MaterialForTerrain));
						cb->draw(4, outline_terrains[i].first, 0, i << 16);
					}
					outline_terrains.clear();
				}
				cb->end_renderpass();

				cb->image_barrier(nd.img_back.get(), {}, graphics::ImageLayoutAttachment, graphics::ImageLayoutShaderReadOnly);
				cb->begin_renderpass(nullptr, img_dst->get_shader_write_dst());
				cb->bind_pipeline(nd.pl_add);
				cb->bind_descriptor_set(0, nd.img_back->get_shader_read_src(0, 0, sp_nearest));
				cb->draw(3, 1, 0, 0);
				cb->end_renderpass();
			}

			cb->image_barrier(img_dst.get(), {}, graphics::ImageLayoutAttachment, graphics::ImageLayoutShaderReadOnly);
			cb->begin_renderpass(nullptr, fb_tars[tar_idx].get());
			cb->bind_pipeline(nd.pl_gamma);
			cb->bind_descriptor_set(0, img_dst->get_shader_read_src(0, 0, sp_nearest));
			cb->push_constant_t(PLL_post::PushConstant{ .f = vec4(render_type == RenderShaded ? 2.2 : 1.0, 0.f, 0.f, 0.f) });
			cb->draw(3, 1, 0, 0);
			cb->end_renderpass();

			if (nd.buf_lines.stag_num > 0)
			{
				auto count = nd.buf_lines.stag_num;
				nd.buf_lines.upload(cb);

				cb->begin_renderpass(nullptr, fb_tars[tar_idx].get());
				cb->bind_vertex_buffer(nd.buf_lines.buf.get(), 0);
				cb->bind_pipeline(nd.pl_line);
				auto& data = *(nd.buf_render_data.pstag);
				cb->push_constant_t(data.proj_view);
				cb->draw(count * 2, 1, 0, 0);
				cb->end_renderpass();
			}
		}

		auto& ed = *_ed;
		if (ed.should_render)
		{
			auto scissor = Rect(vec2(0.f), tar_sz);
			cb->set_viewport(scissor);
			cb->set_scissor(scissor);

			auto pvtx = ed.buf_vtx.pstag;
			auto pidx = ed.buf_idx.pstag;
			ed.buf_vtx.upload(cb);
			ed.buf_idx.upload(cb);

			if (nd.should_render)
				cb->begin_renderpass(rp_bgra8, fb_tars[tar_idx].get());
			else
			{
				auto cv = vec4(0.6f, 0.7f, 0.8f, 1.f);
				cb->begin_renderpass(rp_bgra8c, fb_tars[tar_idx].get(), &cv);
			}

			cb->bind_pipeline(ed.pl_element);
			cb->bind_vertex_buffer(ed.buf_vtx.buf.get(), 0);
			cb->bind_index_buffer(ed.buf_idx.buf.get(), graphics::IndiceTypeUint);
			cb->bind_descriptor_set(element::PLL_element::Binding_element, ed.ds_element.get());
			cb->push_constant_t(element::PLL_element::PushConstant{ 2.f / vec2(tar_sz) });
			auto vtx_off = 0;
			auto idx_off = 0;
			auto vtx_cnt = 0;
			auto idx_cnt = 0;
			auto res = -1;

			auto emit_draw = [&]() {
				if (vtx_cnt > 0)
				{
					cb->draw_indexed(idx_cnt, idx_off, vtx_off, 1, res);
					vtx_off += vtx_cnt;
					idx_off += idx_cnt;
					vtx_cnt = 0;
					idx_cnt = 0;
				}
			};

			for (auto i = 0; i < ed.max_layer; i++)
			{
				for (auto& info : ed.layers[i])
				{
					if (info.vertices.empty())
					{
						emit_draw();
						if (!(scissor == info.scissor))
						{
							scissor = info.scissor;
							cb->set_scissor(scissor);
						}
					}
					else
					{
						if (res != info.res)
						{
							emit_draw();
							res = info.res;
						}

						memcpy(pvtx + vtx_off + vtx_cnt, info.vertices.data(), sizeof(ElementVertex) * info.vertices.size());
						for (auto i = 0; i < info.indices.size(); i++)
							pidx[idx_off + idx_cnt + i] = vtx_cnt + info.indices[i];
						vtx_cnt += info.vertices.size();
						idx_cnt += info.indices.size();
					}
				}
				ed.layers[i].clear();
			}

			emit_draw();

			cb->end_renderpass();
		}

		nd.should_render = false;
		ed.should_render = false;
	}

	uint sRendererPrivate::get_shadow_count(LightType t)
	{
		auto& nd = *_nd;
		switch (t)
		{
		case LightDirectional:
			return nd.dir_shadows.size();
		case LightPoint:
			return nd.pt_shadows.size();
		}
		return 0;
	}

	void sRendererPrivate::get_shadow_matrices(LightType t, uint idx, mat4* dst)
	{
		auto& nd = *_nd;

		switch (t)
		{
		case LightDirectional:
			memcpy(dst, nd.buf_dir_shadows.item(idx, false).mats, 4 * sizeof(mat4));
			break;
		case LightPoint:
			memcpy(dst, nd.buf_pt_shadows.item(idx, false).mats, 6 * sizeof(mat4));
			break;
		}
	}

	void sRendererPrivate::on_added()
	{
		device = graphics::Device::get_default();
		dsp = graphics::DescriptorPool::get_default(device);
		sp_nearest = graphics::Sampler::get(device, graphics::FilterNearest, graphics::FilterNearest, false, graphics::AddressClampToEdge);
		sp_linear = graphics::Sampler::get(device, graphics::FilterLinear, graphics::FilterLinear, false, graphics::AddressClampToEdge);

		rp_rgba8 = graphics::Renderpass::get(device, L"rgba8.rp");
		rp_rgba8c = graphics::Renderpass::get(device, L"rgba8c.rp");
		rp_bgra8 = graphics::Renderpass::get(device, L"bgra8.rp");
		rp_bgra8c = graphics::Renderpass::get(device, L"bgra8c.rp");

		img_white.reset(graphics::Image::create(device, graphics::Format_R8G8B8A8_UNORM, uvec2(1), 1, 1,
			graphics::SampleCount_1, graphics::ImageUsageTransferDst | graphics::ImageUsageSampled));
		img_white->clear(graphics::ImageLayoutUndefined, graphics::ImageLayoutShaderReadOnly, cvec4(255));
		img_black.reset(graphics::Image::create(device, graphics::Format_R8G8B8A8_UNORM, uvec2(1), 1, 1,
			graphics::SampleCount_1, graphics::ImageUsageTransferDst | graphics::ImageUsageSampled));
		img_black->clear(graphics::ImageLayoutUndefined, graphics::ImageLayoutShaderReadOnly, cvec4(0, 0, 0, 255));
		img_black_cube.reset(graphics::Image::create(device, graphics::Format_R8G8B8A8_UNORM, uvec2(1), 1, 6,
			graphics::SampleCount_1, graphics::ImageUsageTransferDst | graphics::ImageUsageSampled, true));
		img_black_cube->clear(graphics::ImageLayoutUndefined, graphics::ImageLayoutShaderReadOnly, cvec4(0, 0, 0, 255));

		auto iv_white = img_white->get_view();
		auto iv_black = img_black->get_view();
		auto iv_black_cube = img_black_cube->get_view({ 0, 1, 0, 6 });

		auto& ed = *_ed;

		ed.pl_element = graphics::Pipeline::get(device, L"element/element.pl");

		ed.buf_vtx.create(device, graphics::BufferUsageVertex, 360000);
		ed.buf_idx.create(device, graphics::BufferUsageIndex, 240000);
		ed.ds_element.reset(graphics::DescriptorSet::create(dsp, graphics::DescriptorSetLayout::get(device, L"element/element.dsl")));
		ed.reses.resize(element::DSL_element::images_count);
		for (auto i = 0; i < ed.reses.size(); i++)
		{
			ed.ds_element->set_image(element::DSL_element::images_binding, i, iv_white, sp_linear);
			ed.reses[i] = iv_white;
		}
		ed.ds_element->update();

		auto& nd = *_nd;

		nd.tex_reses.resize(DSL_material::maps_count);
		nd.mat_reses.resize(_countof(DSL_material::MaterialInfos::material_infos));
		nd.mesh_reses.resize(128);

		for (auto& v : nd.meshes)
			v.resize(nd.mat_reses.size());

		for (auto& b : nd.buf_mesh_indirs)
			b.create(device, graphics::BufferUsageIndirect, 65536);

		{
			auto& dst = nd.mat_reses[MaterialWireframe];
			dst.mat = (graphics::Material*)INVALID_POINTER;
			dst.pipeline_file = L"standard_mat.glsl";
			dst.pipeline_defines = "WIREFRAME";
		}
		{
			auto& dst = nd.mat_reses[MaterialOutline];
			dst.mat = (graphics::Material*)INVALID_POINTER;
			dst.pipeline_file = L"standard_mat.glsl";
			dst.pipeline_defines = "OUTLINE";
		}
		{
			auto& dst = nd.mat_reses[MaterialPickup];
			dst.mat = (graphics::Material*)INVALID_POINTER;
			dst.pipeline_file = L"standard_mat.glsl";
			dst.pipeline_defines = "PICKUP";
		}
		{
			auto& dst = nd.mat_reses[MaterialNormalData];
			dst.mat = (graphics::Material*)INVALID_POINTER;
			dst.pipeline_file = L"standard_mat.glsl";
			dst.pipeline_defines = "NORMAL_DATA";
		}

		nd.buf_mesh_vtx.create(device, graphics::BufferUsageVertex, 200000);
		nd.buf_mesh_idx.create(device, graphics::BufferUsageIndex, 200000);
		nd.buf_arm_mesh_vtx.create(device, graphics::BufferUsageVertex, 200000);
		nd.buf_arm_mesh_idx.create(device, graphics::BufferUsageIndex, 200000);
		nd.buf_ptc_vtx.create(device, graphics::BufferUsageVertex, 6000);
		nd.buf_ptc_idx.create(device, graphics::BufferUsageIndex, 4000);

		nd.buf_lines.create(device, graphics::BufferUsageVertex, 2000000);

		nd.buf_render_data.create(device, graphics::BufferUsageUniform);
		{
			auto& data = *(nd.buf_render_data.pstag);
			data.fog_color = vec3(1.f);
			data.sky_intensity = 1.f;
			data.sky_rad_levels = 0;
		}
		nd.ds_render_data.reset(graphics::DescriptorSet::create(dsp, graphics::DescriptorSetLayout::get(device, L"render_data.dsl")));
		nd.ds_render_data->set_buffer(DSL_render_data::RenderData_binding, 0, nd.buf_render_data.buf.get());
		nd.ds_render_data->update();

		nd.buf_materials.create(device, graphics::BufferUsageStorage);
		nd.ds_material.reset(graphics::DescriptorSet::create(dsp, graphics::DescriptorSetLayout::get(device, L"material.dsl")));
		nd.ds_material->set_buffer(DSL_material::MaterialInfos_binding, 0, nd.buf_materials.buf.get());
		for (auto i = 0; i < nd.tex_reses.size(); i++)
			nd.ds_material->set_image(DSL_material::maps_binding, i, iv_white, sp_linear);
		nd.ds_material->update();

		nd.buf_mesh_transforms.create(device, graphics::BufferUsageStorage);
		nd.buf_mesh_armatures.create(device, graphics::BufferUsageStorage);
		fassert(_countof(mesh::DSL_mesh::Armature::bones) == ArmatureMaxBones);
		nd.ds_mesh.reset(graphics::DescriptorSet::create(dsp, graphics::DescriptorSetLayout::get(device, L"mesh/mesh.dsl")));
		nd.ds_mesh->set_buffer(mesh::DSL_mesh::Transforms_binding, 0, nd.buf_mesh_transforms.buf.get());
		nd.ds_mesh->set_buffer(mesh::DSL_mesh::Armatures_binding, 0, nd.buf_mesh_armatures.buf.get());
		nd.ds_mesh->update();

		nd.buf_terrain.create(device, graphics::BufferUsageStorage);
		nd.ds_terrain.reset(graphics::DescriptorSet::create(dsp, graphics::DescriptorSetLayout::get(device, L"terrain/terrain.dsl")));
		nd.ds_terrain->set_buffer(terrain::DSL_terrain::TerrainInfos_binding, 0, nd.buf_terrain.buf.get());
		nd.ds_terrain->update();

		nd.buf_light_infos.create(device, graphics::BufferUsageStorage);
		nd.buf_tile_lights.create(device, graphics::BufferUsageStorage);
		nd.buf_dir_shadows.create(device, graphics::BufferUsageStorage);
		nd.buf_pt_shadows.create(device, graphics::BufferUsageStorage);
		nd.img_dir_shadow_maps.resize(DSL_light::dir_shadow_maps_count);
		for (auto i = 0; i < nd.img_dir_shadow_maps.size(); i++)
		{
			nd.img_dir_shadow_maps[i].reset(graphics::Image::create(device, graphics::Format_Depth16, shadow_map_size, 1, 4,
				graphics::SampleCount_1, graphics::ImageUsageSampled | graphics::ImageUsageAttachment));
			nd.img_dir_shadow_maps[i]->change_layout(graphics::ImageLayoutUndefined, graphics::ImageLayoutShaderReadOnly);
		}
		nd.img_pt_shadow_maps.resize(DSL_light::pt_shadow_maps_count);
		for (auto i = 0; i < nd.img_pt_shadow_maps.size(); i++)
		{
			nd.img_pt_shadow_maps[i].reset(graphics::Image::create(device, graphics::Format_Depth16, vec2(shadow_map_size) * 0.5f, 1, 6,
				graphics::SampleCount_1, graphics::ImageUsageSampled | graphics::ImageUsageAttachment, true));
			nd.img_pt_shadow_maps[i]->change_layout(graphics::ImageLayoutUndefined, graphics::ImageLayoutShaderReadOnly);
		}
		nd.ds_light.reset(graphics::DescriptorSet::create(dsp, graphics::DescriptorSetLayout::get(device, L"light.dsl")));
		nd.ds_light->set_buffer(DSL_light::LightInfos_binding, 0, nd.buf_light_infos.buf.get());
		nd.ds_light->set_buffer(DSL_light::TileLightsMap_binding, 0, nd.buf_tile_lights.buf.get());
		nd.ds_light->set_buffer(DSL_light::DirShadows_binding, 0, nd.buf_dir_shadows.buf.get());
		nd.ds_light->set_buffer(DSL_light::PtShadows_binding, 0, nd.buf_pt_shadows.buf.get());
		auto sp_shadow = graphics::Sampler::get(device, graphics::FilterLinear, graphics::FilterLinear, false, graphics::AddressClampToBorder);
		for (auto i = 0; i < nd.img_dir_shadow_maps.size(); i++)
		{
			auto iv = nd.img_dir_shadow_maps[i]->get_view({ 0, 1, 0, 4 });
			nd.ds_light->set_image(DSL_light::dir_shadow_maps_binding, i, iv, sp_shadow);
		}
		for (auto i = 0; i < nd.img_pt_shadow_maps.size(); i++)
		{
			auto iv = nd.img_pt_shadow_maps[i]->get_view({ 0, 1, 0, 6 });
			nd.ds_light->set_image(DSL_light::pt_shadow_maps_binding, i, iv, sp_shadow);
		}
		nd.ds_light->set_image(DSL_light::sky_box_binding, 0, iv_black_cube, sp_linear);
		nd.ds_light->set_image(DSL_light::sky_irr_binding, 0, iv_black_cube, sp_linear);
		nd.ds_light->set_image(DSL_light::sky_rad_binding, 0, iv_black_cube, sp_linear);
		nd.ds_light->set_image(DSL_light::sky_lut_binding, 0, iv_black, sp_linear);
		nd.ds_light->update();

		nd.pll_mesh_fwd = graphics::PipelineLayout::get(device, L"mesh/forward.pll");
		nd.pll_mesh_gbuf = graphics::PipelineLayout::get(device, L"mesh/gbuffer.pll");
		nd.pll_terrain_fwd = graphics::PipelineLayout::get(device, L"terrain/forward.pll");
		nd.pll_terrain_gbuf = graphics::PipelineLayout::get(device, L"terrain/gbuffer.pll");

		nd.pl_def = graphics::Pipeline::get(device, L"deferred/deferred.pl");
		nd.ds_def.reset(graphics::DescriptorSet::create(dsp, graphics::DescriptorSetLayout::get(device, L"deferred/deferred.dsl")));

		nd.pl_ptc = graphics::Pipeline::get(device, L"particle/particle.pl");

		nd.pl_line = graphics::Pipeline::get(device, L"plain/line.pl");
		
		nd.pl_downsample = graphics::Pipeline::get(device, L"post/downsample.pl");
		nd.pl_upsample = graphics::Pipeline::get(device, L"post/upsample.pl");
		nd.pl_add = graphics::Pipeline::get(device, L"post/add.pl");
		nd.pl_gamma = graphics::Pipeline::get(device, L"post/gamma.pl");
	}

	void sRendererPrivate::update()
	{
		if (fb_tars.empty() || (!dirty && !always_update))
			return;

		frame = looper().get_frame();

		auto& ed = *_ed;
		ed.scissor = Rect(vec2(0.f), tar_sz);
		if (world->first_element && world->first_element->global_visibility)
		{
			ed.should_render = true;
			ed.max_layer = 0;
			element_render(0, world->first_element->get_component_i<cElementPrivate>(0));
		}
		else
			ed.should_render = false;

		auto& nd = *_nd;
		{ // TODO
			auto& data = nd.buf_tile_lights.item(0);
			data.dir_count = 0;
			data.pt_count = 0;
		}
		if (world->first_node && world->first_node->global_visibility && camera)
		{
			nd.should_render = true;
			Frustum frustums[MaxLod + 1];
			auto dist = camera->far;
			for (auto i = 0; i <= MaxLod; i++)
			{
				frustums[i] = camera->get_frustum(-1.f, dist);
				dist /= 2.f;
			}
			node_render(world->first_node->get_component_i<cNodePrivate>(0), frustums);
		}
		else
			nd.should_render = false;

		dirty = false;
	}

	sRenderer* sRenderer::create(void* parms)
	{
		return new sRendererPrivate((sRendererParms*)parms);
	}
}
