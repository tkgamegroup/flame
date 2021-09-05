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
namespace water
{
#include <water/water.dsl.h>
#include <water/water.pll.h>
}
#include <deferred/ssao.dsl.h>
#include <deferred/ssao.pll.h>
#include <deferred/deferred.dsl.h>
#include <deferred/deferred.pll.h>
#include <particle/particle.pll.h>
#include <post/post.dsl.h>
#include <post/post.pll.h>
#include <post/luminance.dsl.h>
#include <post/luminance.pll.h>
#include <post/tone.dsl.h>
#include <post/tone.pll.h>

namespace flame
{
	using namespace graphics;

	inline AccessFlags u2a(BufferUsageFlags u)
	{
		switch (u)
		{
		case BufferUsageVertex:
			return AccessVertexAttributeRead;
		case BufferUsageIndex:
			return AccessIndexRead;
		case BufferUsageIndirect:
			return AccessIndirectCommandRead;
		}
		return AccessNone;
	}

	inline PipelineStageFlags u2s(BufferUsageFlags u)
	{
		switch (u)
		{
		case BufferUsageVertex:
		case BufferUsageIndex:
			return PipelineStageVertexInput;
		case BufferUsageIndirect:
			return PipelineStageDrawIndirect;
		}
		return PipelineStageAllCommand;
	}

	template <class T>
	struct SequentialBuffer
	{
		uint capacity;
		AccessFlags access;
		PipelineStageFlags plstg;
		T* pstag = nullptr;
		uint stag_num = 0;

		UniPtr<Buffer> buf;
		UniPtr<Buffer> stagbuf;

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

		void create(Device* device, BufferUsageFlags usage, uint _capacity)
		{
			capacity = _capacity;
			access = u2a(usage);
			plstg = u2s(usage);
			auto size = capacity * sizeof(T);
			buf.reset(Buffer::create(device, size, BufferUsageTransferDst | usage, MemoryPropertyDevice));
			stagbuf.reset(Buffer::create(device, size, BufferUsageTransferSrc, MemoryPropertyHost | MemoryPropertyCoherent));
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

		void upload(CommandBuffer* cb)
		{
			if (stag_num == 0)
				return;
			BufferCopy cpy;
			cpy.size = stag_num * sizeof(T);
			cb->copy_buffer(stagbuf.get(), buf.get(), 1, &cpy);
			cb->buffer_barrier(buf.get(), AccessTransferWrite, access, PipelineStageTransfer, plstg);
			stag_num = 0;
		}
	};

	template <class T>
	struct SparseBuffer
	{
		uint capacity;
		AccessFlags access;
		PipelineStageFlags plstg;
		uint n0 = 0;
		uint n1 = 0;

		UniPtr<Buffer> buf;
		UniPtr<Buffer> stagbuf;
		uint stag_capacity;
		T* pstag = nullptr;

		void create(Device* device, BufferUsageFlags usage, uint _capacity)
		{
			capacity = _capacity;
			access = u2a(usage);
			plstg = u2s(usage);
			auto size = capacity * sizeof(T);
			buf.reset(Buffer::create(device, size, BufferUsageTransferDst | usage, MemoryPropertyDevice));
			stag_capacity = 100;
			stagbuf.reset(Buffer::create(device, sizeof(T) * stag_capacity, BufferUsageTransferSrc, MemoryPropertyHost |
				MemoryPropertyCoherent));
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

		void upload(CommandBuffer* cb)
		{
			if (n1 > n0)
			{
				BufferCopy cpy;
				cpy.size = (n1 - n0) * sizeof(T);
				cpy.dst_off = n0 * sizeof(T);
				cb->copy_buffer(stagbuf.get(), buf.get(), 1, &cpy);
				cb->buffer_barrier(buf.get(), AccessTransferWrite, access, PipelineStageTransfer, plstg);
				n0 = n1;
			}
		}
	};

	template <class T>
	struct StorageBuffer
	{
		T* pstag = nullptr;

		UniPtr<Buffer> buf;
		UniPtr<Buffer> stagbuf;

		std::vector<BufferCopy> cpies;

		void create(Device* device, BufferUsageFlags usage)
		{
			buf.reset(Buffer::create(device, sizeof(T), BufferUsageTransferDst | usage, MemoryPropertyDevice));
			stagbuf.reset(Buffer::create(device, sizeof(T), BufferUsageTransferSrc, MemoryPropertyHost | MemoryPropertyCoherent));
			pstag = (T*)stagbuf->map();
		}

		void cpy_whole()
		{
			fassert(cpies.empty());
			BufferCopy cpy;
			cpy.size = sizeof(T);
			cpies.push_back(cpy);
		}

		void upload(CommandBuffer* cb)
		{
			if (cpies.empty())
				return;
			cb->copy_buffer(stagbuf.get(), buf.get(), cpies.size(), cpies.data());
			cb->buffer_barrier(buf.get(), AccessTransferWrite, AccessShaderRead, PipelineStageTransfer);
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
			fassert(idx < countof(items));

			auto& item = items[idx];

			if (mark_cpy)
			{
				BufferCopy cpy;
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
				BufferCopy cpy;
				cpy.src_off = cpy.dst_off = n * sizeof(i);
				cpy.size = overwrite_size;
				cpies.push_back(cpy);
			}

			n++;
			return i;
		}

		void upload(CommandBuffer* cb)
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
		vec3 xext;
		vec3 yext;
		vec4 uv;
		vec4 col;
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
		UniPtr<Pipeline> pipeline;
	};

	struct MaterialRes
	{
		Material* mat;
		bool opaque;
		bool sort;
		std::filesystem::path pipeline_file;
		std::vector<std::string> pipeline_defines;
		int texs[MaxMaterialTexturesCount];
		Pipeline* pls[MaterialUsageCount] = {};

		Pipeline* get_pl(sRendererPrivate* thiz, MaterialUsage u);

	};

	struct MeshRes
	{
		Mesh* mesh = nullptr;
		bool arm;
		uint vtx_off;
		uint vtx_cnt;
		uint idx_off;
		uint idx_cnt;
		std::vector<uint> mat_ids;
	};

	struct ElemenetRenderData
	{
		bool should_render;

		std::vector<ImageView*> reses;

		Rect						scissor;
		std::vector<ElementDrawCmd> layers[128];
		uint						max_layer = 0;

		SequentialBuffer<ElementVertex>	buf_vtx;
		SequentialBuffer<uint>			buf_idx;
		UniPtr<DescriptorSet>			ds_element;

		Pipeline* pl_element;
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

	const inline auto MsaaSampleCount = SampleCount_4;
	const inline uint MaxMatCount = _countof(DSL_material::MaterialInfos::material_infos);
	const inline uint MaxTrnMatCount = 16;
	const inline uint TrnMatBase = MaxMatCount - MaxTrnMatCount;

	struct NodeRenderData
	{
		bool should_render;

		float min_log_lum = -5.f;
		float max_log_lum = +5.f;
		float white_point = 4.f;
		float gamma = 2.2f;
		uint dir_shadow_levels = 3U;
		float dir_shadow_dist = 100.f;
		float pt_shadow_dist = 20.f;
		float pt_shadow_near = 0.1f;
		float ssao_radius = 0.5f;
		float ssao_bias = 0.025f;

		std::vector<ImageView*> tex_reses;
		std::vector<MaterialRes> mat_reses;
		uint max_opq_mat_id = 0;
		uint max_trn_mat_id = 0;
		std::vector<MeshRes> mesh_reses;

		std::vector<std::pair<uint, mat3>>				dir_shadows;
		std::vector<std::pair<uint, vec3>>				pt_shadows;
		std::vector<std::vector<std::pair<uint, uint>>>	meshes[MaterialMeshUsageCount];
		std::vector<std::pair<uint, uint>>				terrains[MaterialTypeCount];
		std::vector<std::pair<uint, uint>>				waters[MaterialTypeCount];
		std::vector<std::pair<uint, uint>>				particles;

		SequentialBuffer<DrawIndexedIndirectCommand> buf_mesh_indirs[MaterialMeshUsageCount];

		SparseBuffer<MeshVertex>	buf_mesh_vtx;
		SparseBuffer<uint>			buf_mesh_idx;
		SparseBuffer<ArmMeshVertex>	buf_arm_mesh_vtx;
		SparseBuffer<uint>			buf_arm_mesh_idx;

		StorageBuffer<DSL_render_data::RenderData>							buf_render_data;
		UniPtr<DescriptorSet>												ds_render_data;
		ArrayStorageBuffer<DSL_material::MaterialInfos>						buf_materials;
		UniPtr<DescriptorSet>												ds_material;
		SequentialArrayStorageBuffer<mesh::DSL_mesh::Transforms>			buf_mesh_transforms;
		SequentialArrayStorageBuffer<mesh::DSL_mesh::Armatures>				buf_mesh_armatures;
		UniPtr<DescriptorSet>												ds_mesh;
		SequentialArrayStorageBuffer<terrain::DSL_terrain::TerrainInfos>	buf_terrain;
		UniPtr<DescriptorSet>												ds_terrain;
		SequentialArrayStorageBuffer<water::DSL_water::WaterInfos>			buf_water;
		UniPtr<DescriptorSet>												ds_water;

		UniPtr<Image> img_dep;
		UniPtr<Image> img_col_met;	// color, metallic
		UniPtr<Image> img_nor_rou;	// normal, roughness
		UniPtr<Image> img_ao;		// ambient occlusion
		UniPtr<Image> img_ao_back;
		UniPtr<Image> img_col_ms;
		UniPtr<Image> img_dep_ms;
		UniPtr<Image> img_dst_back;
		UniPtr<Image> img_dep_back;

		SequentialArrayStorageBuffer<DSL_light::LightInfos>		buf_light_infos;
		ArrayStorageBuffer<DSL_light::TileLightsMap>			buf_tile_lights;
		SequentialArrayStorageBuffer<DSL_light::DirShadows>		buf_dir_shadows;
		SequentialArrayStorageBuffer<DSL_light::PtShadows>		buf_pt_shadows;
		std::vector<UniPtr<Image>>								img_dir_shadow_maps;
		std::vector<UniPtr<Image>>								img_pt_shadow_maps;
		UniPtr<DescriptorSet>									ds_light;

		PipelineLayout* pll_mesh_fwd;
		PipelineLayout* pll_mesh_gbuf;
		PipelineLayout* pll_terrain_fwd;
		PipelineLayout* pll_terrain_gbuf;
		PipelineLayout* pll_water;
		PipelineLayout* pll_post;

		UniPtr<Framebuffer> fb_gbuf;
		std::vector<UniPtr<Framebuffer>> fb_tars_dep;
		UniPtr<Framebuffer> fb_fwd_ms4;

		std::vector<MaterialPipeline> pl_mats[MaterialUsageCount];

		StorageBuffer<DSL_ssao::SampleLocations>	buf_ssao_loc;
		StorageBuffer<DSL_ssao::SampleNoises>		buf_ssao_noi;
		Pipeline*									pl_ssao;
		Pipeline*									pl_ssao_blur;
		UniPtr<DescriptorSet>						ds_ssao;

		Pipeline*				pl_def;
		UniPtr<DescriptorSet>	ds_def;

		SequentialBuffer<ParticleVertex>	buf_ptc_vtx;
		Pipeline* pl_ptc;

		Pipeline* pl_blit_rgba8;
		Pipeline* pl_blit_rgba16;
		Pipeline* pl_blit_rgba16ms4;
		Pipeline* pl_blit_d16;
		Pipeline* pl_blit_d16ms4;
		Pipeline* pl_add_bgra8;
		Pipeline* pl_add_rgba8;
		Pipeline* pl_add_rgba16;
		Pipeline* pl_fxaa;
		Pipeline* pl_downsample;
		Pipeline* pl_upsample;

		StorageBuffer<DSL_luminance::Histogram>		buf_lum_htg;
		StorageBuffer<DSL_luminance::AverageLum>	buf_lum_avg;
		UniPtr<DescriptorSet>						ds_lum;
		PipelineLayout* pll_lum;
		Pipeline* pl_lum_htg;
		Pipeline* pl_lum_avg;

		Pipeline* pl_bright;

		Pipeline* pl_tone;
		UniPtr<DescriptorSet>	ds_tone;

		SequentialBuffer<Line>	buf_lines;
		Pipeline* pl_line;

		NodeRenderData()
		{
			particles.emplace_back(0xffff, 0);
		}
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
		Queue::get(device)->wait_idle();
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

	void sRendererPrivate::get_ssao_props(float* radius, float* bias)
	{
		auto& nd = *_nd;

		if (radius)
			*radius = nd.ssao_radius;
		if (bias)
			*bias = nd.ssao_bias;
	}

	void sRendererPrivate::set_ssao_props(float radius, float bias)
	{
		auto& nd = *_nd;

		nd.ssao_radius = radius;
		nd.ssao_bias = bias;
	}

	ImageView* sRendererPrivate::get_element_res(uint idx) const
	{
		return _ed->reses[idx];
	}

	int sRendererPrivate::set_element_res(int idx, ImageView* iv, Sampler* sp)
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
		ed.ds_element->set_image(DescriptorSetLayout::get(device, L"element/element.dsl")
			->find_binding("images"), idx, iv, sp ? sp : sp_linear);
		ed.ds_element->update();

		return idx;
	}

	int sRendererPrivate::find_element_res(ImageView* iv) const
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

	void sRendererPrivate::draw_glyphs(uint layer, uint cnt, const GlyphDraw* glyphs, uint res_id, const cvec4& color)
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

	int sRendererPrivate::set_texture_res(int idx, ImageView* tex, Sampler* sp)
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

	int sRendererPrivate::find_texture_res(ImageView* tex) const
	{
		auto& nd = *_nd;
		for (auto i = 0; i < nd.tex_reses.size(); i++)
		{
			if (nd.tex_reses[i] == tex)
				return i;
		}
		return -1;
	}

	Pipeline* MaterialRes::get_pl(sRendererPrivate* thiz, MaterialUsage u)
	{
		if (pls[u])
			return pls[u];
		pls[u] = thiz->get_material_pipeline(u, pipeline_file, pipeline_defines);
		return pls[u];
	}

	bool parse_define(const std::vector<std::string>& defines, const std::string& n, std::string& v)
	{
		for (auto& d : defines)
		{
			if (d.compare(0, n.size(), n) == 0)
			{
				if (d.size() == n.size())
					return true;
				if (d[n.size()] == '=')
				{
					v = d.substr(n.size() + 1);
					return true;
				}
			}
		}
		return false;
	};

	int sRendererPrivate::set_material_res(int idx, Material* mat)
	{
		auto& nd = *_nd;

		auto opaque = mat->get_opaque();

		if (idx == -1)
		{
			auto beg = opaque ? (uint)MaterialCustom : TrnMatBase;
			auto end = opaque ? TrnMatBase : MaxMatCount;
			for (auto i = beg; i < end; i++)
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
			for (auto i = 0; i < MaxMaterialTexturesCount; i++)
			{
				if (dst.texs[i] != -1)
					set_texture_res(dst.texs[i], nullptr, nullptr);
			}
			for (auto i = 0; i < countof(dst.pls); i++)
			{
				if (dst.pls[i])
					release_material_pipeline((MaterialUsage)i, dst.pls[i]);
			}
		}
		dst.mat = mat;
		dst.opaque = mat->get_opaque();
		dst.sort = mat->get_sort();
		{
			wchar_t buf[260];
			mat->get_pipeline_file(buf);
			dst.pipeline_file = buf;
		}
		dst.pipeline_defines = Shader::format_defines(mat->get_pipeline_defines());
		if (!opaque)
			dst.pipeline_defines.push_back("TRANSPARENT");
		if (mat)
		{
			InstanceCB cb(device);

			auto& data = nd.buf_materials.item(idx);
			data.color = mat->get_color();
			data.metallic = mat->get_metallic();
			data.roughness = mat->get_roughness();
			auto alpha_map_id = std::make_pair(-1, 0);
			auto alpha_test = 0.f;
			std::string str;
			if (parse_define(dst.pipeline_defines, "ALPHA_TEST", str))
				alpha_test = std::stof(str);
			if (parse_define(dst.pipeline_defines, "ALPHA_MAP", str))
				alpha_map_id = { std::stoi(str), 0 };
			else if (parse_define(dst.pipeline_defines, "COLOR_MAP", str))
				alpha_map_id = { std::stoi(str), 3 };
			for (auto i = 0; i < MaxMaterialTexturesCount; i++)
			{
				wchar_t buf[260]; buf[0] = 0;
				mat->get_texture_file(i, buf);
				auto fn = std::filesystem::path(buf);
				if (fn.empty() || !std::filesystem::exists(fn))
				{
					dst.texs[i] = -1;
					data.map_indices[i] = -1;
				}
				else
				{
					auto srgb = mat->get_texture_srgb(i);
					if (mat->get_texture_auto_mipmap(i))
					{
						auto ext = fn.extension();
						if (ext != L".dds" && ext != L".ktx")
						{
							auto dds_fn = fn;
							dds_fn += L".dds";
							if (!std::filesystem::exists(dds_fn))
							{
								auto img = Image::get(device, fn.c_str(), srgb);
								img->generate_mipmaps();
								if (alpha_test > 0.f && alpha_map_id.first == i)
								{
									auto coverage = img->alpha_test_coverage(0, alpha_test, alpha_map_id.second, 1.f);
									auto lvs = img->get_levels();
									for (auto i = 1; i < lvs; i++)
										img->scale_alpha_to_coverage(i, coverage, alpha_test, alpha_map_id.second);
								}
								img->save(dds_fn.c_str());
								img->release();
							}
							fn = dds_fn;
							srgb = false;
						}
					}

					auto img = Image::get(device, fn.c_str(), srgb);
					auto iv = img->get_view({ 0, img->get_levels(), 0, 1 });
					auto id = find_texture_res(iv);
					if (id == -1)
						id = set_texture_res(-1, iv, mat->get_texture_sampler(device, i));
					dst.texs[i] = id;
					data.map_indices[i] = id;
				}
			}

			nd.buf_materials.upload(cb.get());
		}

		if (opaque)
			nd.max_opq_mat_id = max((uint)idx, nd.max_opq_mat_id);
		else
			nd.max_trn_mat_id = max((uint)idx, nd.max_trn_mat_id);
		return idx;
	}

	int sRendererPrivate::find_material_res(Material* mat) const
	{
		auto& nd = *_nd;
		for (auto i = 0; i < nd.mat_reses.size(); i++)
		{
			if (nd.mat_reses[i].mat == mat)
				return i;
		}
		return -1;
	}

	int sRendererPrivate::set_mesh_res(int idx, Mesh* mesh)
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
			InstanceCB cb(device);

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

			dst.mat_ids.resize(mesh->get_skins_count());
			for (auto i = 0; i < dst.mat_ids.size(); i++)
			{
				auto mat = mesh->get_material(i);
				auto mid = find_material_res(mat);
				if (mid == -1)
					mid = set_material_res(-1, mat);
				dst.mat_ids[i] = mid;
			}
		}

		return idx;
	}

	int sRendererPrivate::find_mesh_res(Mesh* mesh) const
	{
		auto& nd = *_nd;
		for (auto i = 0; i < nd.mesh_reses.size(); i++)
		{
			if (nd.mesh_reses[i].mesh == mesh)
				return i;
		}
		return -1;
	}

	Pipeline* sRendererPrivate::get_material_pipeline(MaterialUsage usage, const std::filesystem::path& mat, std::vector<std::string> defines)
	{
		auto& nd = *_nd;

		for (auto& p : nd.pl_mats[usage])
		{
			if (p.mat == mat && p.defines == defines)
			{
				p.ref_count++;
				return p.pipeline.get();
			}
		}

		Pipeline* ret = nullptr;

		std::vector<std::pair<std::string, std::string>> substitutes;
		auto polygon_mode = PolygonModeFill;
		auto cull_mode = CullModeBack;
		auto a2c = false;
		auto depth_test = true;
		auto depth_write = true;
		auto use_mat = true;
		auto deferred = true;
		auto alpha_blend = false;

		Renderpass* rp = nullptr;

		std::string str;
		if (parse_define(defines, "WIREFRAME", str))
		{
			use_mat = false;
			polygon_mode = PolygonModeLine;
			depth_test = false;
			depth_write = false;
			deferred = false;
			rp = Renderpass::get(device, L"bgra8l.rp");
		}
		else if (parse_define(defines, "PICKUP", str))
		{
			use_mat = false;
			deferred = false;
			rp = Renderpass::get(device, L"rgba16.rp");
		}
		else if (parse_define(defines, "OUTLINE", str))
		{
			use_mat = false;
			depth_test = false;
			depth_write = false;
			deferred = false;
			rp = Renderpass::get(device, L"rgba16.rp");
		}
		else if (parse_define(defines, "NORMAL_DATA", str))
		{
			use_mat = false;
			deferred = false;
			rp = Renderpass::get(device, L"bgra8d16c.rp");
		}
		if (parse_define(defines, "TRANSPARENT", str))
		{
			deferred = false;
			if (parse_define(defines, "ALPHA_TEST", str))
				a2c = true;
			else
				alpha_blend = true;
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
			rp = Renderpass::get(device, L"d16.rp");
			alpha_blend = false;
		case MaterialForMesh:
		{
			if (deferred)
				defines.push_back("DEFERRED");
			auto defines_str = get_defines_str();
			auto substitutes_str = get_substitutes_str();
			GraphicsPipelineInfo info;
			Shader* shaders[] = {
				Shader::get(device, L"mesh/mesh.vert", defines_str.c_str(), substitutes_str.c_str()),
				Shader::get(device, L"mesh/mesh.frag", defines_str.c_str(), substitutes_str.c_str())
			};
			info.shaders_count = countof(shaders);
			info.shaders = shaders;
			info.layout = PipelineLayout::get(device, deferred ? L"mesh/gbuffer.pll" : L"mesh/forward.pll");
			if (!rp)
			{
				if (deferred)
					rp = Renderpass::get(device, L"gbuffer.rp");
				else
				{
					rp = Renderpass::get(device, L"forward_ms4.rp");
					info.sample_count = MsaaSampleCount;
				}
			}
			info.renderpass = rp;
			info.subpass_index = 0;
			VertexAttributeInfo vias[3];
			vias[0].location = 0;
			vias[0].format = Format_R32G32B32_SFLOAT;
			vias[1].location = 1;
			vias[1].format = Format_R32G32_SFLOAT;
			vias[2].location = 2;
			vias[2].format = Format_R32G32B32_SFLOAT;
			VertexBufferInfo vib;
			vib.attributes_count = countof(vias);
			vib.attributes = vias;
			info.vertex_buffers_count = 1;
			info.vertex_buffers = &vib;
			info.polygon_mode = polygon_mode;
			info.cull_mode = cull_mode;
			info.alpha_to_coverage = a2c;
			info.depth_test = depth_test;
			info.depth_write = depth_write;
			BlendOption bo = { true, BlendFactorSrcAlpha, BlendFactorOneMinusSrcAlpha, BlendFactorOne, BlendFactorZero };
			if (alpha_blend)
			{
				info.blend_options_count = 1;
				info.blend_options = &bo;
			}
			ret = Pipeline::create(device, info);
		}
		break;
		case MaterialForMeshShadowArmature:
			deferred = false;
			defines.push_back("SHADOW_PASS");
			rp = Renderpass::get(device, L"d16.rp");
			alpha_blend = false;
		case MaterialForMeshArmature:
		{
			if (deferred)
				defines.push_back("DEFERRED");
			defines.push_back("ARMATURE");
			auto defines_str = get_defines_str();
			auto substitutes_str = get_substitutes_str();
			GraphicsPipelineInfo info;
			Shader* shaders[] = {
				Shader::get(device, L"mesh/mesh.vert", defines_str.c_str(), substitutes_str.c_str()),
				Shader::get(device, L"mesh/mesh.frag", defines_str.c_str(), substitutes_str.c_str())
			};
			info.shaders_count = countof(shaders);
			info.shaders = shaders;
			info.layout = PipelineLayout::get(device, deferred ? L"mesh/gbuffer.pll" : L"mesh/forward.pll");
			if (!rp)
			{
				if (deferred)
					rp = Renderpass::get(device, L"gbuffer.rp");
				else
				{
					rp = Renderpass::get(device, L"forward_ms4.rp");
					info.sample_count = MsaaSampleCount;
				}
			}
			info.renderpass = rp;
			info.subpass_index = 0;
			VertexAttributeInfo vias[5];
			vias[0].location = 0;
			vias[0].format = Format_R32G32B32_SFLOAT;
			vias[1].location = 1;
			vias[1].format = Format_R32G32_SFLOAT;
			vias[2].location = 2;
			vias[2].format = Format_R32G32B32_SFLOAT;
			vias[3].location = 3;
			vias[3].format = Format_R32G32B32A32_INT;
			vias[4].location = 4;
			vias[4].format = Format_R32G32B32A32_SFLOAT;
			VertexBufferInfo vib;
			vib.attributes_count = countof(vias);
			vib.attributes = vias;
			info.vertex_buffers_count = 1;
			info.vertex_buffers = &vib;
			info.polygon_mode = polygon_mode;
			info.cull_mode = cull_mode;
			info.alpha_to_coverage = a2c;
			info.depth_test = depth_test;
			info.depth_write = depth_write;
			if (alpha_blend)
			{
				BlendOption bo = { true, BlendFactorSrcAlpha, BlendFactorOneMinusSrcAlpha, BlendFactorOne, BlendFactorZero };
				info.blend_options_count = 1;
				info.blend_options = &bo;
			}
			ret = Pipeline::create(device, info);
		}
		break;
		case MaterialForTerrain:
		{
			if (deferred)
				defines.push_back("DEFERRED");
			auto defines_str = get_defines_str();
			auto substitutes_str = get_substitutes_str();
			GraphicsPipelineInfo info;
			Shader* shaders[] = {
				Shader::get(device, L"terrain/terrain.vert", defines_str.c_str(), substitutes_str.c_str()),
				Shader::get(device, L"terrain/terrain.tesc", defines_str.c_str(), substitutes_str.c_str()),
				Shader::get(device, L"terrain/terrain.tese", defines_str.c_str(), substitutes_str.c_str()),
				Shader::get(device, L"terrain/terrain.frag", defines_str.c_str(), substitutes_str.c_str())
			};
			info.shaders_count = countof(shaders);
			info.shaders = shaders;
			info.layout = PipelineLayout::get(device, deferred ? L"terrain/gbuffer.pll" : L"terrain/forward.pll");
			if (!rp)
			{
				if (deferred)
					rp = Renderpass::get(device, L"gbuffer.rp");
				else
				{
					rp = Renderpass::get(device, L"forward_ms4.rp");
					info.sample_count = MsaaSampleCount;
				}
			}
			info.renderpass = rp;
			info.subpass_index = 0;
			info.primitive_topology = PrimitiveTopologyPatchList;
			info.patch_control_points = 4;
			info.polygon_mode = polygon_mode;
			info.cull_mode = cull_mode;
			info.depth_test = depth_test;
			info.depth_write = depth_write;
			ret = Pipeline::create(device, info);
		}
		break;
		case MaterialForWater:
		{
			auto defines_str = get_defines_str();
			auto substitutes_str = get_substitutes_str();
			GraphicsPipelineInfo info;
			Shader* shaders[] = {
				Shader::get(device, L"water/water.vert", defines_str.c_str(), substitutes_str.c_str()),
				Shader::get(device, L"water/water.frag", defines_str.c_str(), substitutes_str.c_str())
			};
			info.shaders_count = countof(shaders);
			info.shaders = shaders;
			info.layout = PipelineLayout::get(device, L"water/water.pll");
			if (!rp)
			{
				rp = Renderpass::get(device, L"forward_ms4.rp");
				info.sample_count = MsaaSampleCount;
			}
			info.renderpass = rp;
			info.subpass_index = 0;
			info.primitive_topology = PrimitiveTopologyTriangleList;
			info.polygon_mode = polygon_mode;
			info.cull_mode = cull_mode;
			info.depth_test = depth_test;
			info.depth_write = false;
			info.blend_options_count = 1;
			BlendOption bo = { true, BlendFactorOne, BlendFactorSrcAlpha, BlendFactorOne, BlendFactorZero };
			info.blend_options = &bo;
			ret = Pipeline::create(device, info);
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

	void sRendererPrivate::release_material_pipeline(MaterialUsage usage, Pipeline* pl)
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

	void sRendererPrivate::set_sky(ImageView* box, ImageView* irr,
		ImageView* rad, ImageView* lut, const vec3& fog_color, float intensity, void* id)
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
		data.sky_rad_levels = rad->get_sub().level_count - 1;
	}

	uint sRendererPrivate::add_light(const mat4& mat, LightType type, const vec3& color, bool cast_shadow)
	{
		auto& nd = *_nd;

		auto idx = nd.buf_light_infos.n;

		// TODO
		{
			auto& data = nd.buf_light_infos.add_item();
			data.color = color;
			data.type = type;
			data.shadow_index = -1;

			auto& tile = nd.buf_tile_lights.item(0, false);
			switch (type)
			{
			case LightDirectional:
			{
				auto rot = mat3(mat);
				data.pos = -rot[2];
				if (tile.dir_count < countof(tile.dir_indices))
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
				if (tile.pt_count < countof(tile.pt_indices))
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

		return idx;
	}

	mat4 sRendererPrivate::get_shaodw_mat(uint id, uint idx) const
	{
		auto& nd = *_nd;

		auto& info = nd.buf_light_infos.pstag->light_infos[id];
		if (info.shadow_index == -1)
			return mat4(1.f);

		switch (info.type)
		{
		case LightDirectional:
			return nd.buf_dir_shadows.pstag->dir_shadows[info.shadow_index].mats[idx];
		case LightPoint:
			return nd.buf_pt_shadows.pstag->pt_shadows[info.shadow_index].mats[idx];
		}

		return mat4(1.f);
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

	void sRendererPrivate::draw_mesh(uint idx, uint mesh_id, uint skin, ShadingFlags flags)
	{
		auto& nd = *_nd;

		if (render_type != ShadingMaterial)
			flags = (ShadingFlags)(flags & ~ShadingMaterial);

		auto& mesh = nd.mesh_reses[mesh_id];
		auto usage = mesh.arm ? MaterialForMeshArmature : MaterialForMesh;

		if (flags & ShadingMaterial)
			nd.meshes[usage][mesh.mat_ids[skin]].emplace_back(idx, mesh_id);
		if (render_type == RenderWireframe || (flags & ShadingWireframe))
			nd.meshes[usage][MaterialWireframe].emplace_back(idx, mesh_id);
		if (flags & ShadingOutline)
			nd.meshes[usage][MaterialOutline].emplace_back(idx, mesh_id);
		if (render_type == RenderNormalData)
			nd.meshes[usage][MaterialNormalData].emplace_back(idx, mesh_id);
	}

	void sRendererPrivate::draw_mesh_occluder(uint idx, uint mesh_id, uint skin)
	{
		auto& nd = *_nd;

		auto& mesh = nd.mesh_reses[mesh_id];
		nd.meshes[mesh.arm ? MaterialForMeshShadowArmature : MaterialForMeshShadow][mesh.mat_ids[skin]].emplace_back(idx, mesh_id);
	}

	void sRendererPrivate::draw_terrain(const vec3& coord, const vec3& extent, const uvec2& blocks, uint tess_levels, uint height_map_id, 
		uint normal_map_id, uint tangent_map_id, uint material_id, ShadingFlags flags)
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
		data.tangent_map_id = tangent_map_id;
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

	void sRendererPrivate::draw_water(const vec3& coord, const vec2& extent,
		uint material_id, ShadingFlags flags)
	{
		auto& nd = *_nd;

		if (render_type != ShadingMaterial)
			flags = (ShadingFlags)(flags & ~ShadingMaterial);

		auto& data = nd.buf_water.add_item();
		data.coord = coord;
		data.extent = extent;
		data.material_id = material_id;

		if (flags & ShadingMaterial)
			nd.waters[MaterialCustom].emplace_back(0, material_id);
		if (render_type == RenderWireframe || (flags & ShadingWireframe))
			nd.waters[MaterialWireframe].emplace_back(0, material_id);
		if (flags & ShadingOutline)
			nd.waters[MaterialOutline].emplace_back(0, material_id);
		if (render_type == RenderNormalData)
			nd.waters[MaterialNormalData].emplace_back(0, material_id);
	}

	void sRendererPrivate::draw_particles(uint count, Particle* partcles, uint res_id)
	{
		auto& nd = *_nd;

		if (nd.particles.back().first != res_id)
			nd.particles.emplace_back(res_id, 0);

		auto pvtx = nd.buf_ptc_vtx.stag(count);
		for (auto i = 0; i < count; i++)
		{
			auto& p = partcles[i];
			pvtx[i] = { p.pos, p.xext, p.yext, p.uvs, p.col };
		}

		nd.particles.back().second += count;
	}

	void sRendererPrivate::draw_lines(uint count, Line* lines)
	{
		auto& nd = *_nd;

		memcpy(nd.buf_lines.stag(count), lines, count * sizeof(Line));
	}

	void sRendererPrivate::set_targets(uint tar_cnt, ImageView* const* ivs)
	{
		img_tars.resize(tar_cnt);
		for (auto i = 0; i < tar_cnt; i++)
			img_tars[i] = ivs[i]->get_image();

		fb_tars.clear();
		fb_tars.resize(tar_cnt);
		for (auto i = 0; i < tar_cnt; i++)
			fb_tars[i].reset(Framebuffer::create(device, rp_bgra8, 1, &ivs[i]));

		if (tar_cnt == 0)
			return;

		tar_sz = ivs[0]->get_image()->get_size();
		auto hf_tar_sz = tar_sz / 2U;

		img_dst.reset(Image::create(device, Format_R16G16B16A16_SFLOAT, tar_sz, 1, 1,
			SampleCount_1, ImageUsageSampled | ImageUsageAttachment | ImageUsageStorage));

		auto& nd = *_nd;

		nd.img_dep.reset(Image::create(device, Format_Depth16, tar_sz, 1, 1,
			SampleCount_1, ImageUsageSampled | ImageUsageAttachment | ImageUsageTransferSrc));
		nd.img_col_met.reset(Image::create(device, Format_R8G8B8A8_UNORM, tar_sz, 1, 1,
			SampleCount_1, ImageUsageSampled | ImageUsageAttachment));
		nd.img_nor_rou.reset(Image::create(device, Format_R8G8B8A8_UNORM, tar_sz, 1, 1,
			SampleCount_1, ImageUsageSampled | ImageUsageAttachment));
		nd.img_ao.reset(Image::create(device, Format_R8_UNORM, hf_tar_sz, 1, 1,
			SampleCount_1, ImageUsageSampled | ImageUsageAttachment));
		nd.img_col_ms.reset(Image::create(device, Format_R16G16B16A16_SFLOAT, tar_sz, 1, 1,
			MsaaSampleCount, ImageUsageAttachment));
		nd.img_dep_ms.reset(Image::create(device, Format_Depth16, tar_sz, 1, 1,
			MsaaSampleCount, ImageUsageAttachment));
		nd.img_dst_back.reset(Image::create(device, Format_R16G16B16A16_SFLOAT, tar_sz, 0, 1,
			SampleCount_1, ImageUsageSampled | ImageUsageAttachment));
		nd.img_dep_back.reset(Image::create(device, Format_Depth16, tar_sz, 1, 1,
			SampleCount_1, ImageUsageSampled | ImageUsageAttachment | ImageUsageTransferDst));
		nd.img_dep_back->change_layout(ImageLayoutUndefined, ImageLayoutShaderReadOnly);
		nd.img_ao_back.reset(Image::create(device, Format_R8_UNORM, hf_tar_sz, 1, 1,
			SampleCount_1, ImageUsageSampled | ImageUsageAttachment));

		{
			ImageView* vs[] = {
				nd.img_col_met->get_view(),
				nd.img_nor_rou->get_view(),
				nd.img_dep->get_view()
			};
			nd.fb_gbuf.reset(Framebuffer::create(device, Renderpass::get(device, L"gbuffer.rp"), countof(vs), vs));
		}

		nd.fb_tars_dep.clear();
		nd.fb_tars_dep.resize(tar_cnt);
		auto rp_bgra8d16c = Renderpass::get(device, L"bgra8d16c.rp");
		for (auto i = 0; i < tar_cnt; i++)
		{
			ImageView* vs[] = {
				ivs[i],
				nd.img_dep->get_view()
			};
			nd.fb_tars_dep[i].reset(Framebuffer::create(device, rp_bgra8d16c, countof(vs), vs));
		}

		{
			ImageView* vs[] = {
				nd.img_col_ms->get_view(),
				nd.img_dep_ms->get_view(),
				img_dst->get_view()
			};
			nd.fb_fwd_ms4.reset(Framebuffer::create(device, Renderpass::get(device, L"forward_ms4.rp"), countof(vs), vs));
		}

		nd.ds_ssao->set_image(DSL_ssao::img_nor_rou_binding, 0, nd.img_nor_rou->get_view(), sp_nearest);
		nd.ds_ssao->set_image(DSL_ssao::img_dep_binding, 0, nd.img_dep->get_view(), sp_nearest);
		nd.ds_ssao->update();

		nd.ds_def->set_image(DSL_deferred::img_col_met_binding, 0, nd.img_col_met->get_view(), sp_nearest);
		nd.ds_def->set_image(DSL_deferred::img_nor_rou_binding, 0, nd.img_nor_rou->get_view(), sp_nearest);
		nd.ds_def->set_image(DSL_deferred::img_ao_binding, 0, nd.img_ao->get_view(), sp_nearest);
		nd.ds_def->set_image(DSL_deferred::img_dep_binding, 0, nd.img_dep->get_view(), sp_nearest);
		nd.ds_def->update();

		nd.ds_water->set_image(water::DSL_water::img_depth_binding, 0, nd.img_dep_back->get_view(), sp_nearest);
		nd.ds_water->update();

		nd.ds_lum->set_image(DSL_luminance::img_col_binding, 0, img_dst->get_view(), nullptr);
		nd.ds_lum->update();

		nd.ds_tone->set_image(DSL_tone::image_binding, 0, img_dst->get_view(), sp_nearest);
		nd.ds_tone->update();
	}

	const auto shadow_map_size = uvec2(1024);

	void sRendererPrivate::record(uint tar_idx, CommandBuffer* cb)
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
				data.viewport = tar_sz;
				data.camera_coord = camera->node->g_pos;
				data.camera_dir = -camera->node->g_rot[2];
				data.view = camera->view;
				data.view_inv = camera->view_inv;
				data.proj = camera->proj;
				data.proj_inv = camera->proj_inv;
				data.proj_view = data.proj * data.view;
				*(Frustum*)data.frustum_planes = camera->get_frustum();
				data.time = frame;
			}
			nd.buf_render_data.cpy_whole();
			nd.buf_render_data.upload(cb);

			typedef std::vector<std::pair<uint, uint>> DrawIndirs;

			auto pack_mesh_indirs = [&](MaterialUsage u, bool opaque) {
				DrawIndirs ret;
				auto beg = opaque ? (uint)MaterialCustom : TrnMatBase;
				auto end = opaque ? nd.max_opq_mat_id : nd.max_trn_mat_id;
				for (auto mat_id = beg; mat_id <= end; mat_id++)
				{
					auto& vec = nd.meshes[u][mat_id];
					if (!vec.empty())
					{
						auto indirs = nd.buf_mesh_indirs[u].stag(vec.size());
						auto fill_indirs = [&](const std::vector<std::pair<uint, uint>>& vec) {
							for (auto i = 0; i < vec.size(); i++)
							{
								auto& src = vec[i];
								auto& mr = nd.mesh_reses[src.second];
								auto& dst = indirs[i];
								dst.vertex_offset = mr.vtx_off;
								dst.first_index = mr.idx_off;
								dst.index_count = mr.idx_cnt;
								dst.first_instance = (src.first << 16) + mat_id;
								dst.instance_count = 1;
							}
						};

						if (nd.mat_reses[mat_id].sort)
						{
							auto& trans = nd.buf_mesh_transforms.pstag->transforms;
							auto cam_coord = camera->node->g_pos;
							auto cam_dir = -camera->node->g_rot[2];

							auto _vec = vec;
							std::sort(_vec.begin(), _vec.end(), [&](const auto& a, const auto& b) {
								auto p1 = vec3(trans[a.first].mat[3]);
								auto p2 = vec3(trans[b.first].mat[3]);
								p1 -= cam_coord;
								p2 -= cam_coord;
								auto d1 = dot(p1, cam_dir);
								auto d2 = dot(p2, cam_dir);
								return d1 > d2;
							});
							fill_indirs(_vec);
						}
						else
							fill_indirs(vec);
						ret.emplace_back(mat_id, (uint)vec.size());
						vec.clear();
					}
				}
				return ret;
			};

			auto bind_mesh_fwd_res = [&]() {
				cb->bind_pipeline_layout(nd.pll_mesh_fwd);
				DescriptorSet* sets[mesh::PLL_forward::Binding_Max];
				sets[mesh::PLL_forward::Binding_render_data] = nd.ds_render_data.get();
				sets[mesh::PLL_forward::Binding_material] = nd.ds_material.get();
				sets[mesh::PLL_forward::Binding_light] = nd.ds_light.get();
				sets[mesh::PLL_forward::Binding_mesh] = nd.ds_mesh.get();
				cb->bind_descriptor_sets(0, countof(sets), sets);
			};

			auto bind_mesh_def_res = [&]() {
				cb->bind_pipeline_layout(nd.pll_mesh_gbuf);
				DescriptorSet* sets[mesh::PLL_gbuffer::Binding_Max];
				sets[mesh::PLL_gbuffer::Binding_render_data] = nd.ds_render_data.get();
				sets[mesh::PLL_gbuffer::Binding_material] = nd.ds_material.get();
				sets[mesh::PLL_gbuffer::Binding_mesh] = nd.ds_mesh.get();
				cb->bind_descriptor_sets(0, countof(sets), sets);
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
				DescriptorSet* sets[terrain::PLL_forward::Binding_Max];
				sets[terrain::PLL_forward::Binding_render_data] = nd.ds_render_data.get();
				sets[terrain::PLL_forward::Binding_material] = nd.ds_material.get();
				sets[terrain::PLL_forward::Binding_light] = nd.ds_light.get();
				sets[terrain::PLL_forward::Binding_terrain] = nd.ds_terrain.get();
				cb->bind_descriptor_sets(0, countof(sets), sets);
			};

			auto bind_terrain_def_res = [&]() {
				cb->bind_pipeline_layout(nd.pll_terrain_gbuf);
				DescriptorSet* sets[terrain::PLL_gbuffer::Binding_Max];
				sets[terrain::PLL_gbuffer::Binding_render_data] = nd.ds_render_data.get();
				sets[terrain::PLL_gbuffer::Binding_material] = nd.ds_material.get();
				sets[terrain::PLL_gbuffer::Binding_terrain] = nd.ds_terrain.get();
				cb->bind_descriptor_sets(0, countof(sets), sets);
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

			auto bind_water_res = [&]() {
				cb->bind_pipeline_layout(nd.pll_water);
				DescriptorSet* sets[water::PLL_water::Binding_Max];
				sets[water::PLL_water::Binding_render_data] = nd.ds_render_data.get();
				sets[water::PLL_water::Binding_material] = nd.ds_material.get();
				sets[water::PLL_water::Binding_light] = nd.ds_light.get();
				sets[water::PLL_water::Binding_water] = nd.ds_water.get();
				cb->bind_descriptor_sets(0, countof(sets), sets);
			};

			auto draw_waters = [&]() {
				auto& vec = nd.waters[MaterialCustom];
				for (auto i = 0; i < vec.size(); i++)
				{
					cb->bind_pipeline(nd.mat_reses[vec[i].second].get_pl(this, MaterialForWater));
					cb->draw(6, 1, 0, i << 16);
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
					data.far = camera->far;

					auto& mesh_indirs_vec = dir_shadow_mesh_indirs.emplace_back();
					auto& mesh_arm_indirs_vec = dir_shadow_mesh_arm_indirs.emplace_back();
					for (auto i = 0; i < 4; i++)
					{
						if (i < nd.dir_shadow_levels)
						{
							auto n = i / (float)nd.dir_shadow_levels;
							n = n * n * nd.dir_shadow_dist;
							auto f = (i + 1) / (float)nd.dir_shadow_levels;
							f = f * f * nd.dir_shadow_dist;

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
							auto view = lookAt(c + rot[2] * (data.far - (b.b.z - b.a.z)), c, rot[1]);

							auto mat = proj * view;
							data.mats[i] = mat;
							data.splits[i] = f;

							collect_occluders(world->first_node->get_component_i<cNodePrivate>(0), Frustum(inverse(mat)));
							mesh_indirs_vec.push_back(pack_mesh_indirs(MaterialForMeshShadow, true));
							mesh_arm_indirs_vec.push_back(pack_mesh_indirs(MaterialForMeshShadowArmature, true));
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
						mesh_indirs_vec.push_back(pack_mesh_indirs(MaterialForMeshShadow, true));
						mesh_arm_indirs_vec.push_back(pack_mesh_indirs(MaterialForMeshShadowArmature, true));

					}
					data.near = near;
					data.far = nd.pt_shadow_dist;
				}
			}

			nd.buf_mesh_transforms.upload(cb);
			nd.buf_mesh_armatures.upload(cb);
			nd.buf_terrain.upload(cb);
			nd.buf_water.upload(cb);
			nd.buf_ptc_vtx.upload(cb);
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
						cb->begin_renderpass(nullptr, nd.img_dir_shadow_maps[i]->get_shader_write_dst(0, lv, AttachmentLoadClear), &cv);
						bind_mesh_fwd_res();
						cb->push_constant_t(mesh::PLL_forward::PushConstant{ .i = ivec4(0, i, lv, 0) });
						cb->bind_vertex_buffer(nd.buf_mesh_vtx.buf.get(), 0);
						cb->bind_index_buffer(nd.buf_mesh_idx.buf.get(), IndiceTypeUint);
						dir_mesh_indirs_off = draw_meshes(MaterialForMeshShadow, mesh_indirs_vec[lv], dir_mesh_indirs_off);
						cb->bind_vertex_buffer(nd.buf_arm_mesh_vtx.buf.get(), 0);
						cb->bind_index_buffer(nd.buf_arm_mesh_idx.buf.get(), IndiceTypeUint);
						dir_mesh_arm_indirs_off = draw_meshes(MaterialForMeshShadowArmature, mesh_arm_indirs_vec[lv], dir_mesh_arm_indirs_off);
						cb->end_renderpass();
					}

					cb->image_barrier(nd.img_dir_shadow_maps[i].get(), { 0U, 1U, 0U, nd.dir_shadow_levels }, ImageLayoutAttachment, ImageLayoutShaderReadOnly);
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
						cb->begin_renderpass(nullptr, nd.img_pt_shadow_maps[i]->get_shader_write_dst(0, ly, AttachmentLoadClear), &cv);
						bind_mesh_fwd_res();
						cb->push_constant_t(mesh::PLL_forward::PushConstant{ .i = ivec4(1, i, ly, 0) });
						cb->bind_vertex_buffer(nd.buf_mesh_vtx.buf.get(), 0);
						cb->bind_index_buffer(nd.buf_mesh_idx.buf.get(), IndiceTypeUint);
						pt_mesh_indirs_off = draw_meshes(MaterialForMeshShadow, mesh_indirs_vec[ly], pt_mesh_indirs_off);
						cb->bind_vertex_buffer(nd.buf_arm_mesh_vtx.buf.get(), 0);
						cb->bind_index_buffer(nd.buf_arm_mesh_idx.buf.get(), IndiceTypeUint);
						pt_mesh_arm_indirs_off = draw_meshes(MaterialForMeshShadowArmature, mesh_arm_indirs_vec[ly], pt_mesh_arm_indirs_off);
						cb->end_renderpass();
					}

					cb->image_barrier(nd.img_pt_shadow_maps[i].get(), { 0U, 1U, 0U, 6U }, ImageLayoutAttachment, ImageLayoutShaderReadOnly);
				}
			}

			nd.dir_shadows.clear();
			nd.pt_shadows.clear();

			auto fb_tar = fb_tars[tar_idx].get();
			auto vp = Rect(vec2(0.f), tar_sz);
			cb->set_viewport(vp);
			cb->set_scissor(vp);

			if (render_type == RenderShaded)
			{
				// stag here cause can not update buffer while in render pass
				auto opq_mesh_indirs = pack_mesh_indirs(MaterialForMesh, true);
				auto opq_mesh_arm_indirs = pack_mesh_indirs(MaterialForMeshArmature, true);
				auto trn_mesh_indirs = pack_mesh_indirs(MaterialForMesh, false);
				auto trn_mesh_arm_indirs = pack_mesh_indirs(MaterialForMeshArmature, false);
				nd.buf_mesh_indirs[MaterialForMesh].upload(cb);
				nd.buf_mesh_indirs[MaterialForMeshArmature].upload(cb);
				auto mesh_indirs_off = 0;
				auto mesh_arm_indirs_off = 0;

				vec4 cvs[] = {
					vec4(0.f, 0.f, 0.f, 0.f),
					vec4(0.f, 0.f, 0.f, 0.f),
					vec4(1.f, 0.f, 0.f, 0.f),
					vec4(0.f, 0.f, 0.f, 0.f)
				};
				cb->begin_renderpass(nullptr, nd.fb_gbuf.get(), cvs);
				bind_mesh_def_res();
				cb->bind_vertex_buffer(nd.buf_mesh_vtx.buf.get(), 0);
				cb->bind_index_buffer(nd.buf_mesh_idx.buf.get(), IndiceTypeUint);
				mesh_indirs_off = draw_meshes(MaterialForMesh, opq_mesh_indirs, mesh_indirs_off);
				cb->bind_vertex_buffer(nd.buf_arm_mesh_vtx.buf.get(), 0);
				cb->bind_index_buffer(nd.buf_arm_mesh_idx.buf.get(), IndiceTypeUint);
				mesh_arm_indirs_off = draw_meshes(MaterialForMeshArmature, opq_mesh_arm_indirs, mesh_arm_indirs_off);
				bind_terrain_def_res();
				draw_terrains();
				cb->end_renderpass();

				cb->image_barrier(nd.img_col_met.get(), {}, ImageLayoutAttachment, ImageLayoutShaderReadOnly);
				cb->image_barrier(nd.img_nor_rou.get(), {}, ImageLayoutAttachment, ImageLayoutShaderReadOnly);
				cb->image_barrier(nd.img_dep.get(), {}, ImageLayoutAttachment, ImageLayoutShaderReadOnly);

				cb->set_viewport(vec4(vp) * 0.5f);
				cb->begin_renderpass(nullptr, nd.img_ao_back->get_shader_write_dst());
				cb->bind_pipeline(nd.pl_ssao);
				{
					DescriptorSet* sets[PLL_ssao::Binding_Max];
					sets[PLL_ssao::Binding_ssao] = nd.ds_ssao.get();
					sets[PLL_ssao::Binding_render_data] = nd.ds_render_data.get();
					cb->bind_descriptor_sets(0, countof(sets), sets);
				}
				cb->push_constant_t(PLL_ssao::PushConstant{ nd.ssao_radius, nd.ssao_bias });
				cb->draw(3, 1, 0, 0);
				cb->end_renderpass();

				cb->image_barrier(nd.img_ao_back.get(), {}, ImageLayoutAttachment, ImageLayoutShaderReadOnly);
				cb->begin_renderpass(nullptr, nd.img_ao->get_shader_write_dst());
				cb->bind_pipeline(nd.pl_ssao_blur);
				cb->bind_descriptor_set(0, nd.img_ao_back->get_shader_read_src());
				cb->push_constant_t(PLL_post::PushConstant{ .pxsz = { 1.f / vec2(nd.img_ao->get_size()) } });
				cb->draw(3, 1, 0, 0);
				cb->end_renderpass();
				cb->image_barrier(nd.img_ao.get(), {}, ImageLayoutAttachment, ImageLayoutShaderReadOnly);

				cb->set_viewport(vp);
				cb->begin_renderpass(nullptr, img_dst->get_shader_write_dst());
				cb->bind_pipeline(nd.pl_def);
				{
					DescriptorSet* sets[PLL_deferred::Binding_Max];
					sets[PLL_deferred::Binding_deferred] = nd.ds_def.get();
					sets[PLL_deferred::Binding_render_data] = nd.ds_render_data.get();
					sets[PLL_deferred::Binding_light] = nd.ds_light.get();
					cb->bind_descriptor_sets(0, countof(sets), sets);
				}
				cb->draw(3, 1, 0, 0);
				cb->end_renderpass();
				cb->image_barrier(img_dst.get(), {}, ImageLayoutAttachment, ImageLayoutShaderReadOnly);

				cb->begin_renderpass(nullptr, nd.img_dep_back->get_shader_write_dst());
				cb->bind_pipeline(nd.pl_blit_d16);
				cb->bind_descriptor_set(0, nd.img_dep->get_shader_read_src(0, 0, sp_nearest));
				cb->draw(3, 1, 0, 0);
				cb->end_renderpass();
				cb->image_barrier(nd.img_dep_back.get(), {}, ImageLayoutAttachment, ImageLayoutShaderReadOnly);

				cb->begin_renderpass(nullptr, nd.img_col_ms->get_shader_write_dst());
				cb->bind_pipeline(nd.pl_blit_rgba16ms4);
				cb->bind_descriptor_set(0, img_dst->get_shader_read_src(0, 0, sp_nearest));
				cb->draw(3, 1, 0, 0);
				cb->end_renderpass();

				cb->begin_renderpass(nullptr, nd.img_dep_ms->get_shader_write_dst());
				cb->bind_pipeline(nd.pl_blit_d16ms4);
				cb->bind_descriptor_set(0, nd.img_dep->get_shader_read_src(0, 0, sp_nearest));
				cb->draw(3, 1, 0, 0);
				cb->end_renderpass();

				cb->image_barrier(img_dst.get(), {}, ImageLayoutShaderReadOnly, ImageLayoutAttachment);
				cb->begin_renderpass(nullptr, nd.fb_fwd_ms4.get());
				bind_mesh_fwd_res();
				cb->bind_vertex_buffer(nd.buf_mesh_vtx.buf.get(), 0);
				cb->bind_index_buffer(nd.buf_mesh_idx.buf.get(), IndiceTypeUint);
				mesh_indirs_off = draw_meshes(MaterialForMesh, trn_mesh_indirs, mesh_indirs_off);
				cb->bind_vertex_buffer(nd.buf_arm_mesh_vtx.buf.get(), 0);
				cb->bind_index_buffer(nd.buf_arm_mesh_idx.buf.get(), IndiceTypeUint);
				mesh_arm_indirs_off = draw_meshes(MaterialForMeshArmature, trn_mesh_arm_indirs, mesh_arm_indirs_off);
				if (!nd.waters[MaterialCustom].empty())
				{
					bind_water_res();
					draw_waters();
				}
				if (nd.particles.size() > 1)
				{
					cb->bind_vertex_buffer(nd.buf_ptc_vtx.buf.get(), 0);
					cb->bind_pipeline(nd.pl_ptc);
					{
						DescriptorSet* sets[PLL_particle::Binding_Max];
						sets[PLL_particle::Binding_render_data] = nd.ds_render_data.get();
						sets[PLL_particle::Binding_material] = nd.ds_material.get();
						cb->bind_descriptor_sets(0, countof(sets), sets);
					}
					auto cnt = 0;
					for (auto& vec : nd.particles)
					{
						if (vec.second == 0)
							continue;
						cb->draw(vec.second, 1, cnt, vec.first);
						cnt += vec.second;
					}

					nd.particles.clear();
					nd.particles.emplace_back(0xffff, 0);
				}
				cb->end_renderpass();

				cb->image_barrier(img_dst.get(), {}, ImageLayoutAttachment, ImageLayoutGeneral);
				cb->bind_pipeline_layout(nd.pll_lum, PipelineCompute);
				cb->bind_descriptor_set(0, nd.ds_lum.get());
				cb->push_constant_t(PLL_luminance::PushConstant{ nd.min_log_lum, nd.max_log_lum - nd.min_log_lum, 1.1f, float(tar_sz.x * tar_sz.y) });
				cb->bind_pipeline(nd.pl_lum_htg);
				cb->dispatch(uvec3(ceil(tar_sz.x / 16), ceil(tar_sz.y / 16), 1));
				cb->buffer_barrier(nd.buf_lum_htg.buf.get(), AccessShaderRead | AccessShaderWrite, AccessShaderRead | AccessShaderWrite,
					PipelineStageCompShader, PipelineStageCompShader);
				cb->bind_pipeline(nd.pl_lum_avg);
				cb->dispatch(uvec3(256, 1, 1));
				cb->buffer_barrier(nd.buf_lum_avg.buf.get(), AccessShaderRead | AccessShaderWrite, AccessShaderRead,
					PipelineStageCompShader, PipelineStageAllGraphics);

				cb->image_barrier(img_dst.get(), {}, ImageLayoutGeneral, ImageLayoutShaderReadOnly);
				cb->begin_renderpass(nullptr, nd.img_dst_back->get_shader_write_dst());
				cb->bind_pipeline(nd.pl_bright);
				cb->bind_descriptor_set(0, img_dst->get_shader_read_src(0, 0, sp_nearest));
				cb->push_constant_t(PLL_post::PushConstant{ .f = vec4(nd.white_point, 0.f, 0.f, 0.f) });
				cb->draw(3, 1, 0, 0);
				cb->end_renderpass();

				auto lvs = nd.img_dst_back->get_levels();
				for (auto i = 0; i < lvs - 1; i++)
				{
					cb->image_barrier(nd.img_dst_back.get(), { (uint)i }, ImageLayoutAttachment, ImageLayoutShaderReadOnly);
					cb->set_viewport(Rect(vec2(0.f), nd.img_dst_back->get_size(i + 1)));
					cb->begin_renderpass(nullptr, nd.img_dst_back->get_shader_write_dst(i + 1));
					cb->bind_pipeline(nd.pl_downsample);
					cb->bind_descriptor_set(0, nd.img_dst_back->get_shader_read_src(i, 0, sp_linear));
					cb->push_constant_t(1.f / vec2(nd.img_dst_back->get_size(i)));
					cb->draw(3, 1, 0, 0);
					cb->end_renderpass();
				}
				for (auto i = lvs - 1; i > 1; i--)
				{
					cb->image_barrier(nd.img_dst_back.get(), { (uint)i }, ImageLayoutAttachment, ImageLayoutShaderReadOnly);
					cb->set_viewport(Rect(vec2(0.f), nd.img_dst_back->get_size(i - 1)));
					cb->image_barrier(nd.img_dst_back.get(), { (uint)i - 1 }, ImageLayoutShaderReadOnly, ImageLayoutAttachment);
					cb->begin_renderpass(nullptr, nd.img_dst_back->get_shader_write_dst(i - 1, 0, AttachmentLoadLoad));
					cb->bind_pipeline(nd.pl_upsample);
					cb->bind_descriptor_set(0, nd.img_dst_back->get_shader_read_src(i, 0, sp_linear));
					cb->push_constant_t(1.f / vec2(nd.img_dst_back->get_size(i)));
					cb->draw(3, 1, 0, 0);
					cb->end_renderpass();
				}
				cb->image_barrier(nd.img_dst_back.get(), { 1U }, ImageLayoutAttachment, ImageLayoutShaderReadOnly);
				cb->set_viewport(vp);
				cb->image_barrier(img_dst.get(), {}, ImageLayoutShaderReadOnly, ImageLayoutAttachment);
				cb->begin_renderpass(nullptr, img_dst->get_shader_write_dst(0, 0, AttachmentLoadLoad));
				cb->bind_pipeline(nd.pl_upsample);
				cb->bind_descriptor_set(0, nd.img_dst_back->get_shader_read_src(1, 0, sp_linear));
				cb->push_constant_t(1.f / vec2(nd.img_dst_back->get_size(1)));
				cb->draw(3, 1, 0, 0);
				cb->end_renderpass();

				cb->image_barrier(img_dst.get(), {}, ImageLayoutAttachment, ImageLayoutShaderReadOnly);
				cb->begin_renderpass(nullptr, fb_tar);
				cb->bind_pipeline(nd.pl_tone);
				cb->bind_descriptor_set(0, nd.ds_tone.get());
				cb->push_constant_t(PLL_tone::PushConstant{ nd.white_point, 1.f / nd.gamma });
				cb->draw(3, 1, 0, 0);
				cb->end_renderpass();
			}
			else
			{
				vec4 cvs[] = {
					vec4(0.f, 0.f, 0.f, 0.f),
					vec4(1.f, 0.f, 0.f, 0.f)
				};
				cb->begin_renderpass(nullptr, nd.fb_tars_dep[tar_idx].get(), cvs);

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
							cb->bind_index_buffer(nd.buf_mesh_idx.buf.get(), IndiceTypeUint);
							bind_mesh_fwd_res();
							cb->bind_pipeline(nd.mat_reses[MaterialNormalData].get_pl(this, MaterialForMesh));
							for (auto& m : meshes)
							{
								auto& mr = nd.mesh_reses[m.second];
								cb->draw_indexed(mr.idx_cnt, mr.idx_off, mr.vtx_off, 1, (m.first << 16) + mr.mat_ids[0]);
							}
							meshes.clear();
						}
						if (!arm_meshes.empty())
						{
							cb->bind_vertex_buffer(nd.buf_arm_mesh_vtx.buf.get(), 0);
							cb->bind_index_buffer(nd.buf_arm_mesh_idx.buf.get(), IndiceTypeUint);
							bind_mesh_fwd_res();
							cb->push_constant_t(vec4(0.f, 1.f, 0.f, 1.f));
							cb->bind_pipeline(nd.mat_reses[MaterialNormalData].get_pl(this, MaterialForMeshArmature));
							for (auto& m : arm_meshes)
							{
								auto& mr = nd.mesh_reses[m.second];
								cb->draw_indexed(mr.idx_cnt, mr.idx_off, mr.vtx_off, 1, (m.first << 16) + mr.mat_ids[0]);
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
				cb->begin_renderpass(rp_bgra8l, fb_tar);
				if (!wireframe_meshes.empty())
				{
					cb->bind_vertex_buffer(nd.buf_mesh_vtx.buf.get(), 0);
					cb->bind_index_buffer(nd.buf_mesh_idx.buf.get(), IndiceTypeUint);
					bind_mesh_fwd_res();
					cb->push_constant_t(vec4(0.f, 1.f, 0.f, 1.f));
					cb->bind_pipeline(nd.mat_reses[MaterialWireframe].get_pl(this, MaterialForMesh));
					for (auto& m : wireframe_meshes)
					{
						auto& mr = nd.mesh_reses[m.second];
						cb->draw_indexed(mr.idx_cnt, mr.idx_off, mr.vtx_off, 1, (m.first << 16) + mr.mat_ids[0]);
					}
					wireframe_meshes.clear();
				}
				if (!wireframe_arm_meshes.empty())
				{
					cb->bind_vertex_buffer(nd.buf_arm_mesh_vtx.buf.get(), 0);
					cb->bind_index_buffer(nd.buf_arm_mesh_idx.buf.get(), IndiceTypeUint);
					bind_mesh_fwd_res();
					cb->push_constant_t(vec4(0.f, 1.f, 0.f, 1.f));
					cb->bind_pipeline(nd.mat_reses[MaterialWireframe].get_pl(this, MaterialForMeshArmature));
					for (auto& m : wireframe_arm_meshes)
					{
						auto& mr = nd.mesh_reses[m.second];
						cb->draw_indexed(mr.idx_cnt, mr.idx_off, mr.vtx_off, 1, (m.first << 16) + mr.mat_ids[0]);
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

			auto& outline_meshes = nd.meshes[MaterialForMesh][MaterialOutline];
			auto& outline_arm_meshes = nd.meshes[MaterialForMeshArmature][MaterialOutline];
			auto& outline_terrains = nd.terrains[MaterialOutline];
			if (!outline_meshes.empty() || !outline_arm_meshes.empty() || !outline_terrains.empty())
			{
				auto cv = vec4(0.f);
				cb->begin_renderpass(nullptr, nd.img_dst_back->get_shader_write_dst(0, 0, AttachmentLoadClear), &cv);
				if (!outline_meshes.empty())
				{
					cb->bind_vertex_buffer(nd.buf_mesh_vtx.buf.get(), 0);
					cb->bind_index_buffer(nd.buf_mesh_idx.buf.get(), IndiceTypeUint);
					bind_mesh_fwd_res();
					cb->push_constant_t(vec4(1.f, 1.f, 0.f, 1.f));
					cb->bind_pipeline(nd.mat_reses[MaterialOutline].get_pl(this, MaterialForMesh));
					for (auto& m : outline_meshes)
					{
						auto& mr = nd.mesh_reses[m.second];
						cb->draw_indexed(mr.idx_cnt, mr.idx_off, mr.vtx_off, 1, (m.first << 16) + mr.mat_ids[0]);
					}
				}
				if (!outline_arm_meshes.empty())
				{
					cb->bind_vertex_buffer(nd.buf_arm_mesh_vtx.buf.get(), 0);
					cb->bind_index_buffer(nd.buf_arm_mesh_idx.buf.get(), IndiceTypeUint);
					bind_mesh_fwd_res();
					cb->push_constant_t(vec4(1.f, 1.f, 0.f, 1.f));
					cb->bind_pipeline(nd.mat_reses[MaterialOutline].get_pl(this, MaterialForMeshArmature));
					for (auto& m : outline_arm_meshes)
					{
						auto& mr = nd.mesh_reses[m.second];
						cb->draw_indexed(mr.idx_cnt, mr.idx_off, mr.vtx_off, 1, (m.first << 16) + mr.mat_ids[0]);
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

				auto lvs = min(nd.img_dst_back->get_levels(), 3U);
				for (auto i = 0U; i < lvs - 1; i++)
				{
					cb->image_barrier(nd.img_dst_back.get(), { i }, ImageLayoutAttachment, ImageLayoutShaderReadOnly);
					cb->set_viewport(Rect(vec2(0.f), nd.img_dst_back->get_size(i + 1)));
					cb->begin_renderpass(nullptr, nd.img_dst_back->get_shader_write_dst(i + 1, 0));
					cb->bind_pipeline(nd.pl_downsample);
					cb->bind_descriptor_set(0, nd.img_dst_back->get_shader_read_src(i, 0, sp_linear));
					cb->push_constant_t(1.f / vec2(nd.img_dst_back->get_size(i)));
					cb->draw(3, 1, 0, 0);
					cb->end_renderpass();	
				}
				for (auto i = lvs - 1; i > 0; i--)
				{
					cb->image_barrier(nd.img_dst_back.get(), { i }, ImageLayoutAttachment, ImageLayoutShaderReadOnly);
					cb->set_viewport(Rect(vec2(0.f), nd.img_dst_back->get_size(i - 1)));
					cb->begin_renderpass(nullptr, nd.img_dst_back->get_shader_write_dst(i - 1, 0));
					cb->bind_pipeline(nd.pl_upsample);
					cb->bind_descriptor_set(0, nd.img_dst_back->get_shader_read_src(i, 0, sp_linear));
					cb->push_constant_t(1.f / vec2(nd.img_dst_back->get_size(i)));
					cb->draw(3, 1, 0, 0);
					cb->end_renderpass();
				}

				cb->image_barrier(nd.img_dst_back.get(), {}, ImageLayoutAttachment, ImageLayoutShaderReadOnly);
				cb->set_viewport(vp);
				cb->begin_renderpass(nullptr, nd.img_dst_back->get_shader_write_dst());
				if (!outline_meshes.empty())
				{
					cb->bind_vertex_buffer(nd.buf_mesh_vtx.buf.get(), 0);
					cb->bind_index_buffer(nd.buf_mesh_idx.buf.get(), IndiceTypeUint);
					bind_mesh_fwd_res();
					cb->push_constant_t(vec4(0.f, 0.f, 0.f, 1.f));
					cb->bind_pipeline(nd.mat_reses[MaterialOutline].get_pl(this, MaterialForMesh));
					for (auto& m : outline_meshes)
					{
						auto& mr = nd.mesh_reses[m.second];
						cb->draw_indexed(mr.idx_cnt, mr.idx_off, mr.vtx_off, 1, (m.first << 16) + mr.mat_ids[0]);
					}
					outline_meshes.clear();
				}
				if (!outline_arm_meshes.empty())
				{
					cb->bind_vertex_buffer(nd.buf_arm_mesh_vtx.buf.get(), 0);
					cb->bind_index_buffer(nd.buf_arm_mesh_idx.buf.get(), IndiceTypeUint);
					bind_mesh_fwd_res();
					cb->push_constant_t(vec4(0.f, 0.f, 0.f, 1.f));
					cb->bind_pipeline(nd.mat_reses[MaterialOutline].get_pl(this, MaterialForMeshArmature));
					for (auto& m : outline_arm_meshes)
					{
						auto& mr = nd.mesh_reses[m.second];
						cb->draw_indexed(mr.idx_cnt, mr.idx_off, mr.vtx_off, 1, (m.first << 16) + mr.mat_ids[0]);
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

				cb->image_barrier(nd.img_dst_back.get(), {}, ImageLayoutAttachment, ImageLayoutShaderReadOnly);
				cb->begin_renderpass(rp_bgra8l, fb_tar);
				cb->bind_pipeline(nd.pl_add_bgra8);
				cb->bind_descriptor_set(0, nd.img_dst_back->get_shader_read_src(0, 0, sp_nearest));
				cb->draw(3, 1, 0, 0);
				cb->end_renderpass();
			}

			if (nd.buf_lines.stag_num > 0)
			{
				auto count = nd.buf_lines.stag_num;
				nd.buf_lines.upload(cb);

				cb->begin_renderpass(rp_bgra8l, fb_tar);
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
				cb->begin_renderpass(rp_bgra8l, fb_tars[tar_idx].get());
			else
			{
				auto cv = vec4(0.6f, 0.7f, 0.8f, 1.f);
				cb->begin_renderpass(rp_bgra8c, fb_tars[tar_idx].get(), &cv);
			}

			cb->bind_pipeline(ed.pl_element);
			cb->bind_vertex_buffer(ed.buf_vtx.buf.get(), 0);
			cb->bind_index_buffer(ed.buf_idx.buf.get(), IndiceTypeUint);
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

		cb->image_barrier(img_tars[tar_idx], {}, ImageLayoutAttachment, ImageLayoutPresent);

		nd.should_render = false;
		ed.should_render = false;
	}

	void sRendererPrivate::on_added()
	{
		device = Device::get_default();
		dsp = DescriptorPool::get_default(device);
		sp_nearest = Sampler::get(device, FilterNearest, FilterNearest, false, AddressClampToEdge);
		sp_linear = Sampler::get(device, FilterLinear, FilterLinear, false, AddressClampToEdge);

		InstanceCB cb(device);

		rp_rgba8 = Renderpass::get(device, L"rgba8.rp");
		rp_rgba8c = Renderpass::get(device, L"rgba8c.rp");
		rp_bgra8 = Renderpass::get(device, L"bgra8.rp");
		rp_bgra8l = Renderpass::get(device, L"bgra8l.rp");
		rp_bgra8c = Renderpass::get(device, L"bgra8c.rp");

		img_white.reset(Image::create(device, Format_R8G8B8A8_UNORM, uvec2(1), 1, 1,
			SampleCount_1, ImageUsageTransferDst | ImageUsageSampled));
		img_white->clear(ImageLayoutUndefined, ImageLayoutShaderReadOnly, cvec4(255));
		img_black.reset(Image::create(device, Format_R8G8B8A8_UNORM, uvec2(1), 1, 1,
			SampleCount_1, ImageUsageTransferDst | ImageUsageSampled));
		img_black->clear(ImageLayoutUndefined, ImageLayoutShaderReadOnly, cvec4(0, 0, 0, 255));
		img_black_cube.reset(Image::create(device, Format_R8G8B8A8_UNORM, uvec2(1), 1, 6,
			SampleCount_1, ImageUsageTransferDst | ImageUsageSampled, true));
		img_black_cube->clear(ImageLayoutUndefined, ImageLayoutShaderReadOnly, cvec4(0, 0, 0, 255));

		auto iv_white = img_white->get_view();
		auto iv_black = img_black->get_view();
		auto iv_black_cube = img_black_cube->get_view({ 0, 1, 0, 6 });

		auto& ed = *_ed;

		ed.pl_element = Pipeline::get(device, L"element/element.pl");

		ed.buf_vtx.create(device, BufferUsageVertex, 360000);
		ed.buf_idx.create(device, BufferUsageIndex, 240000);
		ed.ds_element.reset(DescriptorSet::create(dsp, DescriptorSetLayout::get(device, L"element/element.dsl")));
		ed.reses.resize(element::DSL_element::images_count);
		for (auto i = 0; i < ed.reses.size(); i++)
		{
			ed.ds_element->set_image(element::DSL_element::images_binding, i, iv_white, sp_linear);
			ed.reses[i] = iv_white;
		}
		ed.ds_element->update();

		auto& nd = *_nd;

		nd.tex_reses.resize(DSL_material::maps_count);
		nd.mat_reses.resize(MaxMatCount);
		nd.mesh_reses.resize(128);

		for (auto& v : nd.meshes)
			v.resize(MaxMatCount);

		for (auto& b : nd.buf_mesh_indirs)
			b.create(device, BufferUsageIndirect, 65536);

		{
			auto& dst = nd.mat_reses[MaterialWireframe];
			dst.mat = (Material*)INVALID_POINTER;
			dst.pipeline_file = L"standard_mat.glsl";
			dst.pipeline_defines.push_back("WIREFRAME");
		}
		{
			auto& dst = nd.mat_reses[MaterialOutline];
			dst.mat = (Material*)INVALID_POINTER;
			dst.pipeline_file = L"standard_mat.glsl";
			dst.pipeline_defines.push_back("OUTLINE");
		}
		{
			auto& dst = nd.mat_reses[MaterialPickup];
			dst.mat = (Material*)INVALID_POINTER;
			dst.pipeline_file = L"standard_mat.glsl";
			dst.pipeline_defines.push_back("PICKUP");
		}
		{
			auto& dst = nd.mat_reses[MaterialNormalData];
			dst.mat = (Material*)INVALID_POINTER;
			dst.pipeline_file = L"standard_mat.glsl";
			dst.pipeline_defines.push_back("NORMAL_DATA");
		}

		nd.buf_mesh_vtx.create(device, BufferUsageVertex, 2000000);
		nd.buf_mesh_idx.create(device, BufferUsageIndex, 2000000);
		nd.buf_arm_mesh_vtx.create(device, BufferUsageVertex, 2000000);
		nd.buf_arm_mesh_idx.create(device, BufferUsageIndex, 2000000);
		nd.buf_ptc_vtx.create(device, BufferUsageVertex, 10000);

		nd.buf_lines.create(device, BufferUsageVertex, 2000000);

		nd.buf_render_data.create(device, BufferUsageUniform);
		{
			auto& data = *nd.buf_render_data.pstag;
			data.fog_color = vec3(0.f);
			data.sky_intensity = 0.2f;
			data.sky_rad_levels = 0;
		}
		nd.ds_render_data.reset(DescriptorSet::create(dsp, DescriptorSetLayout::get(device, L"render_data.dsl")));
		nd.ds_render_data->set_buffer(DSL_render_data::RenderData_binding, 0, nd.buf_render_data.buf.get());
		nd.ds_render_data->update();

		nd.buf_materials.create(device, BufferUsageStorage);
		nd.ds_material.reset(DescriptorSet::create(dsp, DescriptorSetLayout::get(device, L"material.dsl")));
		nd.ds_material->set_buffer(DSL_material::MaterialInfos_binding, 0, nd.buf_materials.buf.get());
		for (auto i = 0; i < nd.tex_reses.size(); i++)
			nd.ds_material->set_image(DSL_material::maps_binding, i, iv_white, sp_linear);
		nd.ds_material->update();

		nd.buf_mesh_transforms.create(device, BufferUsageStorage);
		nd.buf_mesh_armatures.create(device, BufferUsageStorage);
		fassert(_countof(mesh::DSL_mesh::Armature::bones) == ArmatureMaxBones);
		nd.ds_mesh.reset(DescriptorSet::create(dsp, DescriptorSetLayout::get(device, L"mesh/mesh.dsl")));
		nd.ds_mesh->set_buffer(mesh::DSL_mesh::Transforms_binding, 0, nd.buf_mesh_transforms.buf.get());
		nd.ds_mesh->set_buffer(mesh::DSL_mesh::Armatures_binding, 0, nd.buf_mesh_armatures.buf.get());
		nd.ds_mesh->update();

		nd.buf_terrain.create(device, BufferUsageStorage);
		nd.ds_terrain.reset(DescriptorSet::create(dsp, DescriptorSetLayout::get(device, L"terrain/terrain.dsl")));
		nd.ds_terrain->set_buffer(terrain::DSL_terrain::TerrainInfos_binding, 0, nd.buf_terrain.buf.get());
		nd.ds_terrain->update();

		nd.buf_water.create(device, BufferUsageStorage);
		nd.ds_water.reset(DescriptorSet::create(dsp, DescriptorSetLayout::get(device, L"water/water.dsl")));
		nd.ds_water->set_buffer(water::DSL_water::WaterInfos_binding, 0, nd.buf_water.buf.get());
		nd.ds_water->update();

		nd.buf_light_infos.create(device, BufferUsageStorage);
		nd.buf_tile_lights.create(device, BufferUsageStorage);
		nd.buf_dir_shadows.create(device, BufferUsageStorage);
		nd.buf_pt_shadows.create(device, BufferUsageStorage);
		nd.img_dir_shadow_maps.resize(DSL_light::dir_shadow_maps_count);
		for (auto i = 0; i < nd.img_dir_shadow_maps.size(); i++)
		{
			nd.img_dir_shadow_maps[i].reset(Image::create(device, Format_Depth16, shadow_map_size, 1, 4,
				SampleCount_1, ImageUsageSampled | ImageUsageAttachment));
			nd.img_dir_shadow_maps[i]->change_layout(ImageLayoutUndefined, ImageLayoutShaderReadOnly);
		}
		nd.img_pt_shadow_maps.resize(DSL_light::pt_shadow_maps_count);
		for (auto i = 0; i < nd.img_pt_shadow_maps.size(); i++)
		{
			nd.img_pt_shadow_maps[i].reset(Image::create(device, Format_Depth16, vec2(shadow_map_size) * 0.5f, 1, 6,
				SampleCount_1, ImageUsageSampled | ImageUsageAttachment, true));
			nd.img_pt_shadow_maps[i]->change_layout(ImageLayoutUndefined, ImageLayoutShaderReadOnly);
		}
		nd.ds_light.reset(DescriptorSet::create(dsp, DescriptorSetLayout::get(device, L"light.dsl")));
		nd.ds_light->set_buffer(DSL_light::LightInfos_binding, 0, nd.buf_light_infos.buf.get());
		nd.ds_light->set_buffer(DSL_light::TileLightsMap_binding, 0, nd.buf_tile_lights.buf.get());
		nd.ds_light->set_buffer(DSL_light::DirShadows_binding, 0, nd.buf_dir_shadows.buf.get());
		nd.ds_light->set_buffer(DSL_light::PtShadows_binding, 0, nd.buf_pt_shadows.buf.get());
		auto sp_shadow = Sampler::get(device, FilterLinear, FilterLinear, false, AddressClampToBorder);
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

		nd.pll_mesh_fwd = PipelineLayout::get(device, L"mesh/forward.pll");
		nd.pll_mesh_gbuf = PipelineLayout::get(device, L"mesh/gbuffer.pll");
		nd.pll_terrain_fwd = PipelineLayout::get(device, L"terrain/forward.pll");
		nd.pll_terrain_gbuf = PipelineLayout::get(device, L"terrain/gbuffer.pll");
		nd.pll_water = PipelineLayout::get(device, L"water/water.pll");
		nd.pll_post = PipelineLayout::get(device, L"post/post.pll");

		std::uniform_real_distribution<float> r(0.f, 1.f);
		std::default_random_engine rd;

		nd.buf_ssao_loc.create(device, BufferUsageUniform);
		{
			auto& data = *nd.buf_ssao_loc.pstag;
			for (auto i = 0; i < _countof(data.sample_locations); i++)
			{
				vec3 sample(r(rd) * 2.f - 1.f, r(rd), r(rd) * 2.f - 1.f);
				sample = normalize(sample) * r(rd);

				auto scale = float(i) / _countof(data.sample_locations);
				scale = lerp(0.1f, 1.f, scale * scale);
				sample *= scale;
				data.sample_locations[i] = vec4(sample, 0.f);
			}

			nd.buf_ssao_loc.cpy_whole();
			nd.buf_ssao_loc.upload(cb.get());
		}
		nd.buf_ssao_noi.create(device, BufferUsageUniform);
		{
			auto& data = *nd.buf_ssao_noi.pstag;
			for (auto i = 0; i < _countof(data.sample_noises); i++)
				data.sample_noises[i] = vec4(normalize(vec3(r(rd) * 2.f - 1.f, 0.f, r(rd) * 2.f - 1.f)), 0.f);

			nd.buf_ssao_noi.cpy_whole();
			nd.buf_ssao_noi.upload(cb.get());
		}
		nd.pl_ssao = Pipeline::get(device, L"deferred/ssao.pl");
		nd.pl_ssao_blur = Pipeline::get(device, L"deferred/ssao_blur.pl");
		nd.ds_ssao.reset(DescriptorSet::create(dsp, DescriptorSetLayout::get(device, L"deferred/ssao.dsl")));
		nd.ds_ssao->set_buffer(DSL_ssao::SampleLocations_binding, 0, nd.buf_ssao_loc.buf.get());
		nd.ds_ssao->set_buffer(DSL_ssao::SampleNoises_binding, 0, nd.buf_ssao_noi.buf.get());
		nd.ds_ssao->update();

		nd.pl_def = Pipeline::get(device, L"deferred/deferred.pl");
		nd.ds_def.reset(DescriptorSet::create(dsp, DescriptorSetLayout::get(device, L"deferred/deferred.dsl")));

		nd.pl_ptc = Pipeline::get(device, L"particle/particle.pl");

		nd.pl_line = Pipeline::get(device, L"plain/line.pl");

		nd.pl_blit_rgba8 = Pipeline::get(device, L"post/blit_rgba8.pl");
		nd.pl_blit_rgba16 = Pipeline::get(device, L"post/blit_rgba16.pl");
		nd.pl_blit_rgba16ms4 = Pipeline::get(device, L"post/blit_rgba16ms4.pl");
		nd.pl_blit_d16 = Pipeline::get(device, L"post/blit_d16.pl");
		nd.pl_blit_d16ms4 = Pipeline::get(device, L"post/blit_d16ms4.pl");
		nd.pl_add_bgra8 = Pipeline::get(device, L"post/add_bgra8.pl");
		nd.pl_add_rgba8 = Pipeline::get(device, L"post/add_rgba8.pl");
		nd.pl_add_rgba16 = Pipeline::get(device, L"post/add_rgba16.pl");
		nd.pl_fxaa = Pipeline::get(device, L"post/fxaa.pl");
		nd.pl_downsample = Pipeline::get(device, L"post/downsample.pl");
		nd.pl_upsample = Pipeline::get(device, L"post/upsample.pl");

		nd.buf_lum_htg.create(device, BufferUsageStorage);
		nd.buf_lum_avg.create(device, BufferUsageStorage);
		nd.ds_lum.reset(DescriptorSet::create(dsp, DescriptorSetLayout::get(device, L"post/luminance.dsl")));
		nd.ds_lum->set_buffer(DSL_luminance::Histogram_binding, 0, nd.buf_lum_htg.buf.get());
		nd.ds_lum->set_buffer(DSL_luminance::AverageLum_binding, 0, nd.buf_lum_avg.buf.get());
		nd.ds_lum->update();
		nd.pll_lum = PipelineLayout::get(device, L"post/luminance.pll");
		nd.pl_lum_htg = Pipeline::get(device, L"post/luminance_histogram.pl");
		nd.pl_lum_avg = Pipeline::get(device, L"post/luminance_average.pl");

		nd.pl_bright = Pipeline::get(device, L"post/bright.pl");

		nd.ds_tone.reset(DescriptorSet::create(dsp, DescriptorSetLayout::get(device, L"post/tone.dsl")));
		nd.ds_tone->set_buffer(DSL_tone::AverageLum_binding, 0, nd.buf_lum_avg.buf.get());
		nd.ds_tone->update();
		nd.pl_tone = Pipeline::get(device, L"post/tone.pl");
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
