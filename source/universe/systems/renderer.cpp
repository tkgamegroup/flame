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
#include <terrain/gbuffer.pll.h>
}
#include <deferred/deferred.dsl.h>
#include <deferred/deferred.pll.h>
#include <post/post.dsl.h>
#include <post/post.pll.h>

namespace flame
{
	static graphics::AccessFlags usage2access(graphics::BufferUsageFlags usage)
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

	static void get_frustum_points(float zNear, float zFar, float tan_hf_fovy, float aspect, const mat4& transform, vec3* dst)
	{
		auto y1 = zNear * tan_hf_fovy;
		auto y2 = zFar * tan_hf_fovy;
		auto x1 = y1 * aspect;
		auto x2 = y2 * aspect;

		dst[0] = vec3(transform * vec4(-x1, y1, -zNear, 1.f));
		dst[1] = vec3(transform * vec4(x1, y1, -zNear, 1.f));
		dst[2] = vec3(transform * vec4(x1, -y1, -zNear, 1.f));
		dst[3] = vec3(transform * vec4(-x1, -y1, -zNear, 1.f));
		dst[4] = vec3(transform * vec4(-x2, y2, -zFar, 1.f));
		dst[5] = vec3(transform * vec4(x2, y2, -zFar, 1.f));
		dst[6] = vec3(transform * vec4(x2, -y2, -zFar, 1.f));
		dst[7] = vec3(transform * vec4(-x2, -y2, -zFar, 1.f));
	}

	static void get_frustum_points(const mat4& m, vec3* dst)
	{
		dst[0] = vec3(m * vec4(-1.f, 1.f, 0.f, 1.f));
		dst[1] = vec3(m * vec4(1.f, 1.f, 0.f, 1.f));
		dst[2] = vec3(m * vec4(1.f, -1.f, 0.f, 1.f));
		dst[3] = vec3(m * vec4(-1.f, -1.f, 0.f, 1.f));
		dst[4] = vec3(m * vec4(-1.f, 1.f, 1.f, 1.f));
		dst[5] = vec3(m * vec4(1.f, 1.f, 1.f, 1.f));
		dst[6] = vec3(m * vec4(1.f, -1.f, 1.f, 1.f));
		dst[7] = vec3(m * vec4(-1.f, -1.f, 1.f, 1.f));
	}

	static vec4 make_plane(const vec3& p1, const vec3& p2, const vec3& p3)
	{
		auto v1 = p2 - p1;
		auto v2 = p3 - p1;
		auto n = -normalize(cross(v1, v2));
		return vec4(n, dot(n, -p1));
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
			access = usage2access(usage);
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
			access = usage2access(usage);
			auto size = capacity * sizeof(T);
			buf.reset(graphics::Buffer::create(device, size, graphics::BufferUsageTransferDst | usage, graphics::MemoryPropertyDevice));
			stag_capacity = 100;
			stagbuf.reset(graphics::Buffer::create(device, sizeof(T) * stag_capacity, graphics::BufferUsageTransferSrc, graphics::MemoryPropertyHost | graphics::MemoryPropertyCoherent));
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

		auto& set_item(uint idx, bool mark_cpy = true)
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
		using ArrayStorageBuffer<T>::set_item;

		uint n = 0;

		auto& add_item()
		{
			auto& item = set_item(n, false);

			if (cpies.empty())
				cpies.emplace_back();
			cpies.back().size += sizeof(item);

			n++;
			return item;
		}

		void add_empty(uint count)
		{
			auto& [items] = *pstag;

			if (cpies.empty())
				cpies.emplace_back();
			cpies.back().size += sizeof(items[0]) * count;

			n += count;
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

		void set(const vec2& _pos, const vec2& _uv, const cvec4& _col)
		{
			pos = _pos;
			uv = _uv;
			col = _col;
		}
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

	struct ElementDrawCmd
	{
		enum Type
		{
			Fill,
			Stroke,
			Text,
			Scissor
		};

		Type type;
		uint res_id;
		std::vector<vec2> points;
		std::wstring text;
		cvec4 color;
		vec4 misc;
	};

	enum ElementResType
	{
		ElementResImage,
		ElementResAtlas,
		ElementResFont
	};

	struct ElementRes
	{
		ElementResType type;
		void* v;
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

	struct ElemnetRenderData
	{
		bool should_render;

		std::vector<ElementRes> reses;

		Rect						scissor;
		std::vector<ElementDrawCmd> layers[128];

		SequentialBuffer<ElementVertex>	buf_element_vtx;
		SequentialBuffer<uint>			buf_element_idx;
		UniPtr<graphics::DescriptorSet>	ds_element;

		graphics::Pipeline* pl_element;
	};

	struct NodeRenderData
	{
		bool should_render;

		std::vector<graphics::ImageView*> tex_reses;
		std::vector<MaterialRes> mat_reses;
		std::vector<MeshRes> mesh_reses;

		std::vector<std::pair<uint, cNodePrivate*>>		dir_shadows;
		std::vector<std::pair<uint, cNodePrivate*>>		pt_shadows;
		std::vector<std::vector<std::pair<uint, uint>>>	meshes[MaterialMeshUsageCount];
		std::vector<std::pair<uint, uint>>				terrains;

		SequentialBuffer<graphics::DrawIndexedIndirectCommand>	buf_mesh_indirs[MaterialMeshUsageCount];

		SparseBuffer<MeshVertex>	buf_mesh_vtx;
		SparseBuffer<uint>			buf_mesh_idx;
		SparseBuffer<ArmMeshVertex>	buf_arm_mesh_vtx;
		SparseBuffer<uint>			buf_arm_mesh_idx;

		StorageBuffer<DSL_render_data::RenderData>							buf_render_data;
		UniPtr<graphics::DescriptorSet>										ds_render_data;
		ArrayStorageBuffer<DSL_material::MaterialInfos>						buf_materials;
		UniPtr<graphics::DescriptorSet>										ds_material;
		SequentialArrayStorageBuffer<mesh::DSL_mesh::Transforms>			buf_transforms;
		SequentialArrayStorageBuffer<mesh::DSL_mesh::Armatures>				buf_armatures;
		UniPtr<graphics::DescriptorSet>										ds_mesh;
		SequentialArrayStorageBuffer<terrain::DSL_terrain::TerrainInfos>	buf_terrain;
		UniPtr<graphics::DescriptorSet>										ds_terrain;

		UniPtr<graphics::Image> img_dep;
		UniPtr<graphics::Image> img_col_met; // color, metallic
		UniPtr<graphics::Image> img_nor_rou; // normal, roughness

		SequentialArrayStorageBuffer<DSL_light::LightInfos>			buf_light_infos;
		ArrayStorageBuffer<DSL_light::GridLights>					buf_grid_lights;
		SequentialArrayStorageBuffer<DSL_light::DirShadowMats>		buf_dir_shadow_mats;
		SequentialArrayStorageBuffer<DSL_light::PtShadowMats>		buf_pt_shadow_mats;
		std::vector<UniPtr<graphics::Image>>						img_dir_shadow_maps;
		std::vector<UniPtr<graphics::Framebuffer>>					fb_dir_shadow_maps;
		std::vector<UniPtr<graphics::Image>>						img_pt_shadow_maps;
		std::vector<UniPtr<graphics::Framebuffer>>					fb_pt_shadow_maps;
		UniPtr<graphics::DescriptorSet>								ds_light;

		graphics::PipelineLayout* pll_mesh_fwd;
		graphics::PipelineLayout* pll_mesh_gbuf;
		graphics::PipelineLayout* pll_terrain_gbuf;

		UniPtr<graphics::Framebuffer> fb_gbuf;
		UniPtr<graphics::Framebuffer> fb_def;

		std::vector<MaterialPipeline>	pl_mats[MaterialUsageCount];
		graphics::Pipeline* pl_def;
		graphics::Pipeline* pl_nor_dat;
		UniPtr<graphics::DescriptorSet>	ds_def;

		graphics::Pipeline* pl_gamma;
	};

	sRendererPrivate::sRendererPrivate(sRendererParms* _parms)
	{
		sRendererParms parms;
		if (_parms)
			parms = *_parms;

		_ed.reset(new ElemnetRenderData);
		_nd.reset(new NodeRenderData);
	}

	uint sRendererPrivate::element_render(uint layer, cElementPrivate* element)
	{
		auto& ed = *_ed;

		auto e = element->entity;
		if (!e->global_visibility)
			return layer;

		element->parent_scissor = ed.scissor;
		element->update_transform();
		auto culled = !ed.scissor.overlapping(element->aabb);
		if (element->culled != culled)
		{
			element->culled = culled;
			e->component_data_changed(element, S<"culled"_h>);
		}
		if (culled)
			return layer;

		element->draw(layer, this);

		auto clipping = false;
		Rect last_scissor;
		if (element->clipping && !(ed.scissor == element->aabb))
		{
			element->layer_policy = 2;

			clipping = true;
			last_scissor = ed.scissor;
			ed.scissor = element->aabb;
			auto& info = ed.layers[layer].emplace_back();
			info.type = ElementDrawCmd::Scissor;
			info.misc = ed.scissor;
		}

		auto _layer = layer;
		for (auto& d : element->drawers)
			_layer = max(_layer, d->call(layer, this));

		_layer++;
		auto max_layer = _layer;
		for (auto& c : e->children)
		{
			auto celement = c->get_component_i<cElementPrivate>(0);
			if (celement)
			{
				max_layer = max(max_layer, element_render(_layer, celement));
				if (celement->layer_policy > 0)
				{
					_layer = max_layer + 1;
					if (celement->layer_policy == 2)
						element->layer_policy = 2;
				}
			}
		}

		if (clipping)
		{
			ed.scissor = last_scissor;
			auto& info = ed.layers[max_layer + 1].emplace_back();
			info.type = ElementDrawCmd::Scissor;
			info.misc = last_scissor;
		}

		return max_layer;
	}

	void sRendererPrivate::node_render(cNodePrivate* element)
	{
		auto e = element->entity;
		if (!e->global_visibility)
			return;

		auto node = e->get_component_i<cNodePrivate>(0);
		if (node)
		{
			node->update_transform();
			for (auto& d : node->drawers)
				d->call(this);
		}

		for (auto& c : e->children)
		{
			auto cnode = c->get_component_i<cNodePrivate>(0);
			if (cnode)
				node_render(cnode);
		}
	}

	void* sRendererPrivate::get_element_res(uint idx, char* type) const
	{
		auto& res = _ed->reses[idx];
		if (type)
		{
			switch (res.type)
			{
			case ElementResImage:
				strcpy(type, "ImageView");
				break;
			case ElementResAtlas:
				strcpy(type, "ImageAtlas");
				break;
			case ElementResFont:
				strcpy(type, "FontAtlas");
				break;
			}
		}
		return res.v;
	}

	int sRendererPrivate::set_element_res(int idx, const char* _type_name, void* v)
	{
		auto iv_white = img_white->get_view();
		auto& ed = *_ed;

		auto type_name = std::string(_type_name);
		ElementResType type;
		if (type_name == "ImageView")
			type = ElementResImage;
		else if (type_name == "ImageAtlas")
			type = ElementResAtlas;
		else if (type_name == "FontAtlas")
			type = ElementResFont;
		else
			fassert(0);

		if (!v)
		{
			if (type != ElementResImage)
				return - 1;
			v = iv_white;
		}

		if (idx == -1)
		{
			for (auto i = 1; i < ed.reses.size(); i++)
			{
				if (ed.reses[i].v == iv_white)
				{
					idx = i;
					break;
				}
			}
		}
		if (idx == -1)
			return -1;

		auto& res = ed.reses[idx];
		res.type = type;
		res.v = v;

		auto bd = graphics::DescriptorSetLayout::get(device, L"element/element.dsl")->find_binding("images");
		switch (type)
		{
		case ElementResImage:
			ed.ds_element->set_image(bd, idx, (graphics::ImageView*)v, sp_linear);
			break;
		case ElementResAtlas:
		{
			auto ia = (graphics::ImageAtlas*)v;
			ed.ds_element->set_image(bd, idx, ia->get_image()->get_view(0), ia->get_border() ? sp_linear : sp_nearest);
		}
			break;
		case ElementResFont:
			ed.ds_element->set_image(bd, idx, ((graphics::FontAtlas*)v)->get_view(), sp_nearest);
			break;
		}

		return idx;
	}

	int sRendererPrivate::find_element_res(void* v) const
	{
		auto& ed = *_ed;
		for (auto i = 0; i < ed.reses.size(); i++)
		{
			if (ed.reses[i].v == v)
				return i;
		}
		return -1;
	}

	void sRendererPrivate::fill_rect(uint layer, cElementPtr element, const vec2& pos, const vec2& size, const cvec4& color)
	{
		auto& ed = *_ed;

		auto& info = ed.layers[layer].emplace_back();
		info.type = ElementDrawCmd::Fill;

		info.res_id = 0;
		info.points.resize(4);
		auto a = pos - element->pivot * element->size;
		auto b = a + size;
		auto c = vec2(element->transform[2]);
		if (element->crooked)
		{
			a = c + element->axes * a;
			b = c + element->axes * b;
		}
		else
		{
			a += c;
			b += c;
		}
		info.points[0] = vec2(a.x, a.y);
		info.points[1] = vec2(b.x, a.y);
		info.points[2] = vec2(b.x, b.y);
		info.points[3] = vec2(a.x, b.y);
		info.color = color;
	}

	void sRendererPrivate::stroke(uint layer, cElementPtr element, uint pt_cnt, const vec2* pts, float thickness, const cvec4& color)
	{
		auto& ed = *_ed;

		auto& info = ed.layers[layer].emplace_back();
		info.type = ElementDrawCmd::Stroke;

		info.res_id = 0;
		info.points.resize(pt_cnt);
		auto c = vec2(element->transform[2]);
		if (element->crooked)
		{
			for (auto i = 0; i < pt_cnt; i++)
				info.points[i] = c + element->axes * pts[i];
		}
		else
		{
			for (auto i = 0; i < pt_cnt; i++)
				info.points[i] = c + pts[i];
		}
		info.color = color;
		info.misc[0] = thickness;
	}

	void sRendererPrivate::stroke_rect(uint layer, cElementPtr element, const vec2& pos, const vec2& size, float thickness, const cvec4& color)
	{
		auto& ed = *_ed;

		auto& info = ed.layers[layer].emplace_back();
		info.type = ElementDrawCmd::Stroke;

		info.res_id = 0;
		info.points.resize(5);
		auto a = pos - element->pivot * element->size;
		auto b = a + size;
		auto c = vec2(element->transform[2]);
		if (element->crooked)
		{
			a = c + element->axes * a;
			b = c + element->axes * b;
		}
		else
		{
			a += c;
			b += c;
		}
		info.points[0] = vec2(a.x, a.y);
		info.points[1] = vec2(b.x, a.y);
		info.points[2] = vec2(b.x, b.y);
		info.points[3] = vec2(a.x, b.y);
		info.points[4] = info.points[0];
		info.color = color;
		info.misc[0] = thickness;
	}

	void sRendererPrivate::draw_text(uint layer, cElementPtr element, const vec2& pos, uint font_size, uint font_id, const wchar_t* text_beg, const wchar_t* text_end, const cvec4& color)
	{
		auto& ed = *_ed;

		auto& res = ed.reses[font_id];
		if (!res.type == ElementResFont)
			return;

		auto& info = ed.layers[layer].emplace_back();
		info.type = ElementDrawCmd::Text;

		info.res_id = font_id;
		info.points.resize(3);
		auto a = pos - element->pivot * element->size;
		auto c = vec2(element->transform[2]);
		if (element->crooked)
			a = c + element->axes * a;
		else
			a += c;
		info.points[0] = a;
		info.points[1] = element->axes[0];
		info.points[2] = element->axes[1];
		info.color = color;
		info.misc[0] = font_size;
		info.text.assign(text_beg, text_end);
	}

	static std::vector<vec2> calculate_normals(const std::vector<vec2>& points, bool closed)
	{
		std::vector<vec2> normals(points.size());
		for (auto i = 0; i < points.size() - 1; i++)
		{
			auto d = normalize(points[i + 1] - points[i]);
			auto normal = vec2(d.y, -d.x);

			if (i > 0)
				normals[i] = normalize((normal + normals[i]) * 0.5f);
			else
				normals[i] = normal;

			if (closed && i + 1 == points.size() - 1)
				normals.front() = normalize((normal + normals[0]) * 0.5f);
			else
				normals[i + 1] = normal;
		}
		return normals;
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
		nd.ds_material->set_image(DSL_material::maps_binding, idx, tex ? tex : img_white->get_view(), sp);

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
		pls[u] = thiz->get_material_pipeline(u, mat->get_pipeline_file(), mat->get_pipeline_defines());
		return pls[u];
	}

	int sRendererPrivate::set_material_res(int idx, graphics::Material* mat)
	{
		auto& nd = *_nd;

		if (idx == -1)
		{
			for (auto i = 0; i < nd.mat_reses.size(); i++)
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
		if (mat)
		{
			graphics::InstanceCB cb(device);

			auto& data = nd.buf_materials.set_item(idx);
			data.color = mat->get_color();
			data.metallic = mat->get_metallic();
			data.roughness = mat->get_roughness();
			data.alpha_test = mat->get_alpha_test();
			for (auto i = 0; i < 4; i++)
			{
				wchar_t buf[260];
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
			auto aidx = mesh->get_indices();

			auto bone_cnt = mesh->get_bones_count();
			if (bone_cnt == 0)
			{
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
				auto pidx = nd.buf_mesh_idx.alloc(dst.idx_cnt);
				memcpy(pidx, aidx, sizeof(uint) * dst.idx_cnt);

				nd.buf_mesh_vtx.upload(cb.get());
				nd.buf_mesh_idx.upload(cb.get());
			}
			else
			{
				auto n = nd.buf_arm_mesh_idx.n1;
				auto pidx = nd.buf_arm_mesh_idx.alloc(dst.idx_cnt);
				for (auto i = 0; i < dst.idx_cnt; i++)
					pidx[i] = n + aidx[i];
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

		auto find_define = [&](const std::string& s, bool remove = false) {
			for (auto& d : defines)
			{
				if (d == s)
				{
					if (remove)
						d = "";
					return true;
				}
			}
			return false;
		};
		if (find_define("WIREFRAME"))
		{
			use_mat = false;
			polygon_mode = graphics::PolygonModeLine;
			depth_test = false;
			depth_write = false;
		}
		else if (find_define("PICKUP"))
			use_mat = false;
		else if (find_define("OUTLINE"))
		{
			use_mat = false;
			depth_test = false;
			depth_write = false;
		}
		if (find_define("DOUBLE_SIDE", true))
			cull_mode = graphics::CullModeNone;
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

		graphics::Renderpass* rp = nullptr;

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
			graphics::Shader* shaders[] = {
				graphics::Shader::get(device, L"mesh/mesh.vert", defines_str.c_str(), substitutes_str.c_str()),
				graphics::Shader::get(device, L"mesh/mesh.frag", defines_str.c_str(), substitutes_str.c_str())
			};
			graphics::GraphicsPipelineInfo info;
			if (!rp)
				rp = graphics::Renderpass::get(device, deferred ? L"gbuffer.rp" : L"forward.rp");
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
			ret = graphics::Pipeline::create(device, _countof(shaders), shaders, 
				graphics::PipelineLayout::get(device, deferred ? L"mesh/gbuffer.pll" : L"mesh/forward.pll"), info);
		}
			break;
		//case MaterialForMeshShadowArmature:
		//	defines.push_back("SHADOW_PASS");
		//case MaterialForMeshArmature:
		//{
		//	defines.push_back("ARMATURE");
		//	graphics::Shader* shaders[] = {
		//		graphics::Shader::get(device, L"mesh/mesh.vert", defines, {}),
		//		graphics::Shader::get(device, L"mesh/mesh.frag", defines, substitutes)
		//	};
		//	VertexAttributeInfo vias1[3];
		//	vias1[0].location = 0;
		//	vias1[0].format = Format_R32G32B32_SFLOAT;
		//	vias1[1].location = 1;
		//	vias1[1].format = Format_R32G32_SFLOAT;
		//	vias1[2].location = 2;
		//	vias1[2].format = Format_R32G32B32_SFLOAT;
		//	VertexAttributeInfo vias2[2];
		//	vias2[0].location = 5;
		//	vias2[0].format = Format_R32G32B32A32_INT;
		//	vias2[1].location = 6;
		//	vias2[1].format = Format_R32G32B32A32_SFLOAT;
		//	VertexBufferInfo vibs[2];
		//	vibs[0].attributes_count = _countof(vias1);
		//	vibs[0].attributes = vias1;
		//	vibs[1].attributes_count = _countof(vias2);
		//	vibs[1].attributes = vias2;
		//	VertexInfo vi;
		//	vi.buffers_count = 2;
		//	vi.buffers = vibs;
		//	RasterInfo rst;
		//	rst.polygon_mode = polygon_mode;
		//	DepthInfo dep;
		//	dep.test = depth_test;
		//	dep.write = depth_write;
		//	ret = PipelinePrivate::create(device, shaders, mesh_pipeline_layout, mesh_renderpass.get(), 0, &vi, &rst, &dep);
		//}
		//	break;
		case MaterialForTerrain:
		{
			if (deferred)
				defines.push_back("DEFERRED");
			auto defines_str = get_defines_str();
			auto substitutes_str = get_substitutes_str();
			graphics::Shader* shaders[] = {
				graphics::Shader::get(device, L"terrain/terrain.vert", defines_str.c_str(), substitutes_str.c_str()),
				graphics::Shader::get(device, L"terrain/terrain.tesc", defines_str.c_str(), substitutes_str.c_str()),
				graphics::Shader::get(device, L"terrain/terrain.tese", defines_str.c_str(), substitutes_str.c_str()),
				graphics::Shader::get(device, L"terrain/terrain.frag", defines_str.c_str(), substitutes_str.c_str())
			};
			graphics::GraphicsPipelineInfo info;
			info.renderpass = graphics::Renderpass::get(device, deferred ? L"gbuffer.rp" : L"forward.rp");
			info.subpass_index = 0;
			info.primitive_topology = graphics::PrimitiveTopologyPatchList;
			info.patch_control_points = 4;
			info.polygon_mode = polygon_mode;
			info.cull_mode = cull_mode;
			info.depth_test = depth_test;
			info.depth_write = depth_write;
			ret = graphics::Pipeline::create(device, _countof(shaders), shaders, 
				graphics::PipelineLayout::get(device, deferred ? L"terrain/gbuffer.pll" : L"terrain/forward.pll"), info);
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

	void sRendererPrivate::get_sky(graphics::ImageView** out_box, graphics::ImageView** out_irr,
		graphics::ImageView** out_rad, graphics::ImageView** out_lut, void** out_id)
	{
		if (out_box)
			*out_box = sky_box;
		if (out_irr)
			*out_irr = sky_irr;
		if (out_rad)
			*out_rad = sky_rad;
		if (out_lut)
			*out_lut = sky_lut;
		if (out_id)
			*out_id = sky_id;
	}

	void sRendererPrivate::set_sky(graphics::ImageView* box, graphics::ImageView* irr,
		graphics::ImageView* rad, graphics::ImageView* lut, void* id)
	{
		sky_box = box;
		sky_irr = irr;
		sky_rad = rad;
		sky_lut = lut;
		sky_id = id;

		auto& nd = *_nd;
		auto iv_black = img_black->get_view();
		nd.ds_light->set_image(DSL_light::sky_box_binding, 0, box ? box : iv_black, sp_linear);
		nd.ds_light->set_image(DSL_light::sky_irr_binding, 0, irr ? irr : iv_black, sp_linear);
		nd.ds_light->set_image(DSL_light::sky_rad_binding, 0, rad ? rad : iv_black, sp_linear);
		nd.ds_light->set_image(DSL_light::sky_lut_binding, 0, lut ? lut : iv_black, sp_linear);
	}

	void sRendererPrivate::add_light(cNodePtr node, LightType type, const vec3& color, bool cast_shadow)
	{
		auto& nd = *_nd;

		node->update_transform();

		auto idx = nd.buf_light_infos.n;

		// TODO
		{
			auto& data = nd.buf_light_infos.add_item();
			data.color = color;
			data.shadow_index = -1;

			auto& grid = nd.buf_grid_lights.set_item(0);
			switch (type)
			{
			case LightDirectional:
				data.pos = -node->g_rot[2];
				if (grid.dir_count < _countof(grid.dir_indices))
				{
					grid.dir_indices[grid.dir_count] = idx;
					grid.dir_count++;
				}
				if (cast_shadow)
				{
					if (nd.dir_shadows.size() < 4)
					{
						data.shadow_index = nd.dir_shadows.size();
						nd.dir_shadows.emplace_back(idx, node);
					}
				}
				break;
			case LightPoint:
				data.pos = node->g_pos;
				if (grid.pt_count < _countof(grid.pt_indices))
				{
					grid.pt_indices[grid.pt_count] = idx;
					grid.pt_count++;
				}
				if (cast_shadow)
				{
					if (nd.pt_shadows.size() < 4)
					{
						data.shadow_index = nd.pt_shadows.size();
						nd.pt_shadows.emplace_back(idx, node);
					}
				}
				break;
			}
		}
	}

	void sRendererPrivate::draw_mesh(cNodePtr node, uint mesh_id, bool cast_shadow)
	{
		auto& nd = *_nd;

		node->update_transform();

		auto idx = nd.buf_transforms.n;

		auto& data = nd.buf_transforms.add_item();
		data.mat = node->transform;
		data.nor = mat4(node->rot);

		auto mat_id = nd.mesh_reses[mesh_id].mat_id;
		nd.meshes[/*TODO:Armature?MaterialForMeshArmature:*/MaterialForMesh][mat_id].emplace_back(idx, mesh_id);
		if (cast_shadow)
			nd.meshes[/*TODO:Armature?MaterialForMeshShadowArmature:*/MaterialForMeshShadow][mat_id].emplace_back(idx, mesh_id);
	}

	void sRendererPrivate::draw_terrain(cNodePtr node, const uvec2& blocks, uint tess_levels, uint height_map_id, uint normal_map_id, uint material_id)
	{
		auto& nd = *_nd;

		node->update_transform();

		auto& data = nd.buf_terrain.add_item();
		data.coord = node->g_pos;
		data.scale = node->g_scl;
		data.blocks = blocks;
		data.tess_levels = tess_levels;
		data.height_map_id = height_map_id;
		data.normal_map_id = normal_map_id;
		data.material_id = material_id;

		nd.terrains.emplace_back(blocks.x * blocks.y, material_id);
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
		tar_sz = ivs[0]->get_image()->get_size();

		img_back.reset(graphics::Image::create(device, graphics::Format_R16G16B16A16_SFLOAT, tar_sz, 1, 1,
			graphics::SampleCount_1, graphics::ImageUsageSampled | graphics::ImageUsageAttachment));
		ds_back->set_image(DSL_post::image_binding, 0, img_back->get_view(), sp_nearest);

		auto& nd = *_nd;

		nd.img_dep.reset(graphics::Image::create(device, graphics::Format_Depth16, tar_sz, 1, 1,
			graphics::SampleCount_1, graphics::ImageUsageSampled | graphics::ImageUsageAttachment));
		nd.img_col_met.reset(graphics::Image::create(device, graphics::Format_R8G8B8A8_UNORM, tar_sz, 1, 1,
			graphics::SampleCount_1, graphics::ImageUsageSampled | graphics::ImageUsageAttachment));
		nd.img_nor_rou.reset(graphics::Image::create(device, graphics::Format_R8G8B8A8_UNORM, tar_sz, 1, 1,
			graphics::SampleCount_1, graphics::ImageUsageSampled | graphics::ImageUsageAttachment));

		{
			graphics::ImageView* vs[3];
			vs[0] = nd.img_col_met->get_view();
			vs[1] = nd.img_nor_rou->get_view();
			vs[2] = nd.img_dep->get_view();
			nd.fb_gbuf.reset(graphics::Framebuffer::create(device, graphics::Renderpass::get(device, L"gbuffer.rp"), _countof(vs), vs));
		}

		{
			auto iv = img_back->get_view();
			nd.fb_def.reset(graphics::Framebuffer::create(device, graphics::Renderpass::get(device, L"deferred/deferred.rp"), 1, &iv));
		}

		nd.ds_def->set_image(DSL_deferred::img_col_met_binding, 0, nd.img_col_met->get_view(), sp_nearest);
		nd.ds_def->set_image(DSL_deferred::img_nor_rou_binding, 0, nd.img_nor_rou->get_view(), sp_nearest);
		nd.ds_def->set_image(DSL_deferred::img_dep_binding, 0, nd.img_dep->get_view(), sp_nearest);
	}

	const auto shadow_map_size = uvec2(1024);

	void sRendererPrivate::record(uint tar_idx, graphics::CommandBuffer* cb)
	{
		auto& nd = *_nd;
		if (nd.should_render)
		{
			auto csm_levels = 3U; // TODO
			auto ptsm_near = 0.01f; // TODO
			auto tar_aspect = tar_sz.x / tar_sz.y;

			{
				camera->update_view();

				auto& data = *(nd.buf_render_data.pstag);
				data.sky_rad_levels = 0;
				data.csm_levels = csm_levels;
				data.csm_factor = 0.3f; // TODO
				data.ptsm_near = ptsm_near;
				data.zNear = camera->near;
				data.zFar = camera->far;
				data.camera_coord = camera->node->g_pos;
				data.view = camera->view;
				data.view_inv = camera->view_inv;
				data.proj = perspective(radians(camera->fovy), tar_aspect, data.zNear, data.zFar);
				data.proj[1][1] *= -1.f;
				data.proj_inv = inverse(data.proj);
				data.proj_view = data.proj * data.view;
				vec3 ps[8];
				get_frustum_points(data.zNear, data.zFar, tan(radians(camera->fovy * 0.5f)), tar_aspect, data.view_inv, ps);
				data.frustum_planes[0] = make_plane(ps[0], ps[1], ps[2]); // near
				data.frustum_planes[1] = make_plane(ps[5], ps[4], ps[6]); // far
				data.frustum_planes[2] = make_plane(ps[4], ps[0], ps[7]); // left
				data.frustum_planes[3] = make_plane(ps[1], ps[5], ps[2]); // right
				data.frustum_planes[4] = make_plane(ps[4], ps[5], ps[0]); // top
				data.frustum_planes[5] = make_plane(ps[3], ps[2], ps[7]); // bottom
			}
			nd.buf_render_data.cpy_whole();
			nd.buf_render_data.upload(cb);

			nd.buf_transforms.upload(cb);
			nd.buf_terrain.upload(cb);

			auto pack_mesh_indirs = [&](MaterialUsage u) {
				for (auto mat_id = 0; mat_id < nd.meshes[u].size(); mat_id++)
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
					}
				}
				nd.buf_mesh_indirs[u].upload(cb);
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

			auto draw_mesh_indirs = [&](MaterialUsage u) {
				auto off = 0;
				for (auto mat_id = 0; mat_id < nd.meshes[u].size(); mat_id++)
				{
					auto& vec = nd.meshes[u][mat_id];
					if (!vec.empty())
					{
						cb->bind_pipeline(nd.mat_reses[mat_id].get_pl(this, u));
						cb->draw_indexed_indirect(nd.buf_mesh_indirs[u].buf.get(), off, vec.size());
						off += vec.size();
					}
				}
			};

			auto draw_terrains = [&]() {
				if (!nd.terrains.empty())
				{
					cb->bind_pipeline_layout(nd.pll_terrain_gbuf);
					graphics::DescriptorSet* sets[terrain::PLL_gbuffer::Binding_Max];
					sets[terrain::PLL_gbuffer::Binding_render_data] = nd.ds_render_data.get();
					sets[terrain::PLL_gbuffer::Binding_material] = nd.ds_material.get();
					sets[terrain::PLL_gbuffer::Binding_terrain] = nd.ds_terrain.get();
					cb->bind_descriptor_sets(0, _countof(sets), sets);
					for (auto i = 0; i < nd.terrains.size(); i++)
					{
						cb->bind_pipeline(nd.mat_reses[nd.terrains[i].second].get_pl(this, MaterialForTerrain));
						cb->draw(4, nd.terrains[i].first, 0, i << 16);
					}
				}
			};

			for (auto& s : nd.dir_shadows)
			{
				auto dist = 100.f; // TODO

				auto& data = nd.buf_light_infos.set_item(s.first, false);
				data.shadow_distance = dist;

				auto tan_hf_fovy = tan(radians(camera->fovy * 0.5f));

				auto mat = s.second->g_rot;
				mat[2] *= -1.f;
				auto inv = inverse(mat);

				for (auto i = 0; i < csm_levels; i++)
				{
					auto n = i / (float)csm_levels;
					n = n * n * dist;
					auto f = (i + 1) / (float)csm_levels;
					f = f * f * dist;

					vec3 a = vec3(+10000.f);
					vec3 b = vec3(-10000.f);
					vec3 ps[8];
					get_frustum_points(n, f, tan_hf_fovy, tar_aspect, camera->view_inv, ps);
					for (auto k = 0; k < 8; k++)
					{
						auto p = inv * ps[k];
						a = min(a, p);
						b = max(b, p);
					}
					auto c = mat * ((a + b) * 0.5f);
					auto w = (b.x - a.x) * 0.5f;
					auto h = (b.y - a.y) * 0.5f;
					nd.buf_dir_shadow_mats.add_item() = orthoRH(-w, +w, -h, +h, 0.f, dist) * lookAt(c - s.second->g_rot[2] * dist * 0.5f, c, s.second->g_rot[1]);
				}

				if (csm_levels < 4)
					nd.buf_dir_shadow_mats.add_empty(4 - csm_levels);
			}

			for (auto& s : nd.pt_shadows)
			{
				auto dist = 20.f; // TODO

				auto proj = perspective(radians(90.f), ptsm_near, 1.f, dist);
				proj[1][1] *= -1.f;

				for (auto i = 0; i < 6; i++)
				{
					mat4 matrix;
					switch (i)
					{
					case 0:
						matrix[0][0] = -1.f;
						matrix = matrix * proj * lookAt(s.second->g_pos, s.second->g_pos + vec3(1.f, 0.f, 0.f), vec3(0.f, 1.f, 0.f));
						break;
					case 1:
						matrix[0][0] = -1.f;
						matrix = matrix * proj * lookAt(s.second->g_pos, s.second->g_pos + vec3(-1.f, 0.f, 0.f), vec3(0.f, 1.f, 0.f));
						break;
					case 2:
						matrix[1][1] = -1.f;
						matrix = matrix * proj * lookAt(s.second->g_pos, s.second->g_pos + vec3(0.f, 1.f, 0.f), vec3(1.f, 0.f, 0.f));
						break;
					case 3:
						matrix[1][1] = -1.f;
						matrix = matrix * proj * lookAt(s.second->g_pos, s.second->g_pos + vec3(0.f, -1.f, 0.f), vec3(0.f, 0.f, -1.f));
						break;
					case 4:
						matrix[0][0] = -1.f;
						matrix = matrix * proj * lookAt(s.second->g_pos, s.second->g_pos + vec3(0.f, 0.f, 1.f), vec3(0.f, 1.f, 0.f));
						break;
					case 5:
						matrix[0][0] = -1.f;
						matrix = matrix * proj * lookAt(s.second->g_pos, s.second->g_pos + vec3(0.f, 0.f, -1.f), vec3(0.f, 1.f, 0.f));
						break;
					}

					nd.buf_pt_shadow_mats.add_item() = matrix;
				}
			}

			nd.buf_dir_shadow_mats.upload(cb);
			nd.buf_pt_shadow_mats.upload(cb);

			/*  HOW TO DRAW ARMATURED MESH REFERENCE:
				auto mrm = m.res;
				auto mat = material_resources[mrm->material_id].get();
				if (m.deformer)
				{
					cb->bind_pipeline(mat->get_pipeline(MaterialForMeshArmature));
					cb->bind_vertex_buffer(mrm->weight_buffer.buf.get(), 1);
					//cb->bind_descriptor_set(S<"armature"_h>, m.deformer->descriptorset.get());
				}
				cb->draw_indexed(mrm->index_buffer.capacity, 0, 0, 1, (mesh_off << 16) + mrm->material_id);
			*/

			pack_mesh_indirs(MaterialForMeshShadow);

			cb->set_viewport(Rect(0.f, 0.f, shadow_map_size.x, shadow_map_size.y));
			cb->set_scissor(Rect(0.f, 0.f, shadow_map_size.x, shadow_map_size.y));

			for (auto i = 0; i < nd.dir_shadows.size(); i++)
			{
				for (auto lv = 0; lv < csm_levels; lv++)
				{
					auto cv = vec4(1.f, 0.f, 0.f, 0.f);
					cb->begin_renderpass(nullptr, nd.fb_dir_shadow_maps[i * 4 + lv].get(), &cv);
					bind_mesh_fwd_res();
					cb->push_constant_t(mesh::PLL_forward::PushConstant{ .i=ivec4(0, lv, 0, 0) });
					cb->bind_vertex_buffer(nd.buf_mesh_vtx.buf.get(), 0);
					cb->bind_index_buffer(nd.buf_mesh_idx.buf.get(), graphics::IndiceTypeUint);
					draw_mesh_indirs(MaterialForMeshShadow);
					cb->end_renderpass();
				}

				cb->image_barrier(nd.img_dir_shadow_maps[i].get(), { 0U, 1U, 0U, csm_levels }, graphics::ImageLayoutAttachment, graphics::ImageLayoutShaderReadOnly);
			}

			cb->set_viewport(Rect(0.f, 0.f, shadow_map_size.x * 0.5f, shadow_map_size.y * 0.5f));
			cb->set_scissor(Rect(0.f, 0.f, shadow_map_size.x * 0.5f, shadow_map_size.y * 0.5f));

			for (auto i = 0; i < nd.pt_shadows.size(); i++)
			{
				for (auto ly = 0; ly < 6; ly++)
				{
					auto cv = vec4(1.f, 0.f, 0.f, 0.f);
					cb->begin_renderpass(nullptr, nd.fb_pt_shadow_maps[i * 6 + ly].get(), &cv);
					bind_mesh_fwd_res();
					cb->push_constant_t(mesh::PLL_forward::PushConstant{ .i=ivec4(1, ly, 0, 0) });
					cb->bind_vertex_buffer(nd.buf_mesh_vtx.buf.get(), 0);
					cb->bind_index_buffer(nd.buf_mesh_idx.buf.get(), graphics::IndiceTypeUint);
					draw_mesh_indirs(MaterialForMeshShadow);
					cb->end_renderpass();
				}

				cb->image_barrier(nd.img_pt_shadow_maps[i].get(), { 0U, 1U, 0U, 6U }, graphics::ImageLayoutAttachment, graphics::ImageLayoutShaderReadOnly);
			}

			nd.buf_light_infos.upload(cb);
			nd.buf_grid_lights.upload(cb);

			pack_mesh_indirs(MaterialForMesh);

			auto vp = Rect(vec2(0.f), tar_sz);
			cb->set_viewport(vp);
			cb->set_scissor(vp);
			{
				vec4 cvs[] = {
					vec4(0.f, 0.f, 0.f, 0.f),
					vec4(0.f, 0.f, 0.f, 0.f),
					vec4(1.f, 0.f, 0.f, 0.f),
					vec4(0.f, 0.f, 0.f, 0.f)
				};
				cb->begin_renderpass(nullptr, nd.fb_gbuf.get(), cvs);
			}
			bind_mesh_def_res();
			cb->bind_vertex_buffer(nd.buf_mesh_vtx.buf.get(), 0);
			cb->bind_index_buffer(nd.buf_mesh_idx.buf.get(), graphics::IndiceTypeUint);
			draw_mesh_indirs(MaterialForMesh);
			draw_terrains();
			cb->end_renderpass();

			cb->image_barrier(nd.img_col_met.get(), {}, graphics::ImageLayoutAttachment, graphics::ImageLayoutShaderReadOnly);
			cb->image_barrier(nd.img_nor_rou.get(), {}, graphics::ImageLayoutAttachment, graphics::ImageLayoutShaderReadOnly);
			cb->image_barrier(nd.img_dep.get(), {}, graphics::ImageLayoutAttachment, graphics::ImageLayoutShaderReadOnly);

			cb->begin_renderpass(nullptr, nd.fb_def.get());
			switch (shading_type)
			{
			case ShadingNormalData:
				cb->bind_pipeline(nd.pl_nor_dat);
				break;
			default:
				cb->bind_pipeline(nd.pl_def);
			}
			{
				graphics::DescriptorSet* sets[PLL_deferred::Binding_Max];
				sets[PLL_deferred::Binding_deferred] = nd.ds_def.get();
				sets[PLL_deferred::Binding_render_data] = nd.ds_render_data.get();
				sets[PLL_deferred::Binding_light] = nd.ds_light.get();
				cb->bind_descriptor_sets(0, _countof(sets), sets);
			}
			cb->draw(3, 1, 0, 0);
			cb->end_renderpass();

			cb->image_barrier(img_back.get(), {}, graphics::ImageLayoutAttachment, graphics::ImageLayoutShaderReadOnly);
			cb->begin_renderpass(nullptr, fb_tars[tar_idx].get());
			cb->bind_pipeline(nd.pl_gamma);
			cb->bind_descriptor_set(0, ds_back.get());
			cb->push_constant_t(PLL_post::PushConstant{ .f=vec4(shading_type == ShadingCombined ? 2.2 : 1.0, 0.f, 0.f, 0.f) });
			cb->draw(3, 1, 0, 0);
			cb->end_renderpass();
		}

		auto& ed = *_ed;
		if (ed.should_render)
		{
			struct PackedCmd
			{
				bool b;
				union
				{
					struct
					{
						uint res_id;
						uint vtx_cnt;
						uint idx_cnt;
					}a;
					Rect b;
				}d;
			};
			std::vector<PackedCmd> cmds;
			{
				auto& c = cmds.emplace_back();
				c.b = true;
				c.d.b = Rect(vec2(0.f), tar_sz);
			}

			Rect scissor;
			scissor.reset();
			for (auto i = 0; i < _countof(ed.layers); i++)
			{
				for (auto& info : ed.layers[i])
				{
					switch (info.type)
					{
					case ElementDrawCmd::Fill:
					{
						if (cmds.back().b || cmds.back().d.a.res_id != info.res_id)
						{
							auto& c = cmds.emplace_back();
							c.b = false;
							c.d.a.res_id = info.res_id;
						}
						auto& c = cmds.back();

						for (auto i = 0; i < info.points.size() - 2; i++)
						{
							auto pvtx = ed.buf_element_vtx.stag(3);
							pvtx[0].set(info.points[0], vec2(0.5), info.color);
							pvtx[1].set(info.points[i + 1], vec2(0.5), info.color);
							pvtx[2].set(info.points[i + 2], vec2(0.5), info.color);

							auto pidx = ed.buf_element_idx.stag(3);
							pidx[0] = c.d.a.vtx_cnt + 0;
							pidx[1] = c.d.a.vtx_cnt + 2;
							pidx[2] = c.d.a.vtx_cnt + 1;

							c.d.a.vtx_cnt += 3;
							c.d.a.idx_cnt += 3;

							if (/*aa*/false)
							{
								//auto vtx_cnt0 = cmd->vertices_count;
								//auto feather = 2.f;
							
								//points.push_back(points.front());
								//auto normals = calculate_normals(points, true);
							
								//auto col_t = col;
								//col_t.a = 0;
							
								//for (auto i = 0; i < points.size() - 1; i++)
								//{
								//	if (i == 0)
								//	{
								//		auto p0 = points[0];
								//		auto p1 = points[1];
							
								//		auto n0 = normals[0];
								//		auto n1 = normals[1];
							
								//		auto vtx_cnt = cmd->vertices_count;
							
								//		add_vtx(p0, uv, col);
								//		add_vtx(p0 - n0 * feather, uv, col_t);
								//		add_vtx(p1, uv, col);
								//		add_vtx(p1 - n1 * feather, uv, col_t);
								//		add_idx(vtx_cnt + 0); add_idx(vtx_cnt + 3); add_idx(vtx_cnt + 1); add_idx(vtx_cnt + 0); add_idx(vtx_cnt + 2); add_idx(vtx_cnt + 3);
								//	}
								//	else if (i == points.size() - 2)
								//	{
								//		auto vtx_cnt = cmd->vertices_count;
							
								//		add_idx(vtx_cnt - 2); add_idx(vtx_cnt0 + 1); add_idx(vtx_cnt - 1); add_idx(vtx_cnt - 2); add_idx(vtx_cnt0 + 0); add_idx(vtx_cnt0 + 1);
								//	}
								//	else
								//	{
								//		auto p1 = points[i + 1];
							
								//		auto n1 = normals[i + 1];
							
								//		auto vtx_cnt = cmd->vertices_count;
							
								//		add_vtx(p1, uv, col);
								//		add_vtx(p1 - n1 * feather, uv, col_t);
								//		add_idx(vtx_cnt - 2); add_idx(vtx_cnt + 1); add_idx(vtx_cnt - 1); add_idx(vtx_cnt - 2); add_idx(vtx_cnt + 0); add_idx(vtx_cnt + 1);
								//	}
								//}
							}
						}
					}
						break;
					case ElementDrawCmd::Stroke:
					{
						if (cmds.back().b || cmds.back().d.a.res_id != info.res_id)
						{
							auto& c = cmds.emplace_back();
							c.b = false;
							c.d.a.res_id = info.res_id;
						}
						auto& c = cmds.back();

						auto thickness = info.misc[0];
						auto closed = info.points.front() == info.points.back();
						auto normals = calculate_normals(info.points, closed);

						if (/*aa*/ false)
						{
							//static const auto feather = 0.5f;
							//auto col_c = col;
							//col_c.a *= min(thickness / feather, 1.f);
							//auto col_t = col;
							//col_t.a = 0;
							//
							//if (thickness > feather)
							//{
							//	auto edge = thickness - feather;
							//
							//	for (auto i = 0; i < points.size() - 1; i++)
							//	{
							//		if (i == 0)
							//		{
							//			auto p0 = points[0];
							//			auto p1 = points[1];
							//
							//			auto n0 = normals[0];
							//			auto n1 = normals[1];
							//
							//			auto vtx_cnt = cmd->vertices_count;
							//
							//			add_vtx(p0 + n0 * thickness, uv, col_t);
							//			add_vtx(p0 + n0 * edge, uv, col_c);
							//			add_vtx(p0 - n0 * edge, uv, col_c);
							//			add_vtx(p0 - n0 * thickness, uv, col_t);
							//			add_vtx(p1 + n1 * thickness, uv, col_t);
							//			add_vtx(p1 + n1 * edge, uv, col_c);
							//			add_vtx(p1 - n1 * edge, uv, col_c);
							//			add_vtx(p1 - n1 * thickness, uv, col_t);
							//			add_idx(vtx_cnt + 1); add_idx(vtx_cnt + 6); add_idx(vtx_cnt + 2); add_idx(vtx_cnt + 1); add_idx(vtx_cnt + 5); add_idx(vtx_cnt + 6);
							//			add_idx(vtx_cnt + 0); add_idx(vtx_cnt + 5); add_idx(vtx_cnt + 1); add_idx(vtx_cnt + 0); add_idx(vtx_cnt + 4); add_idx(vtx_cnt + 5);
							//			add_idx(vtx_cnt + 2); add_idx(vtx_cnt + 7); add_idx(vtx_cnt + 3); add_idx(vtx_cnt + 2); add_idx(vtx_cnt + 6); add_idx(vtx_cnt + 7);
							//		}
							//		else if (closed && i == points.size() - 2)
							//		{
							//			auto vtx_cnt = cmd->vertices_count;
							//
							//			add_idx(vtx_cnt - 3); add_idx(vtx_cnt0 + 2); add_idx(vtx_cnt - 2); add_idx(vtx_cnt - 3); add_idx(vtx_cnt0 + 1); add_idx(vtx_cnt0 + 2);
							//			add_idx(vtx_cnt - 4); add_idx(vtx_cnt0 + 1); add_idx(vtx_cnt - 3); add_idx(vtx_cnt - 4); add_idx(vtx_cnt0 + 0); add_idx(vtx_cnt0 + 1);
							//			add_idx(vtx_cnt - 2); add_idx(vtx_cnt0 + 3); add_idx(vtx_cnt - 1); add_idx(vtx_cnt - 2); add_idx(vtx_cnt0 + 2); add_idx(vtx_cnt0 + 3);
							//		}
							//		else
							//		{
							//			auto p1 = points[i + 1];
							//
							//			auto n1 = normals[i + 1];
							//
							//			auto vtx_cnt = cmd->vertices_count;
							//
							//			add_vtx(p1 + n1 * thickness, uv, col_t);
							//			add_vtx(p1 + n1 * edge, uv, col_c);
							//			add_vtx(p1 - n1 * edge, uv, col_c);
							//			add_vtx(p1 - n1 * thickness, uv, col_t);
							//			add_idx(vtx_cnt - 3); add_idx(vtx_cnt + 2); add_idx(vtx_cnt - 2); add_idx(vtx_cnt - 3); add_idx(vtx_cnt + 1); add_idx(vtx_cnt + 2);
							//			add_idx(vtx_cnt - 4); add_idx(vtx_cnt + 1); add_idx(vtx_cnt - 3); add_idx(vtx_cnt - 4); add_idx(vtx_cnt + 0); add_idx(vtx_cnt + 1);
							//			add_idx(vtx_cnt - 2); add_idx(vtx_cnt + 3); add_idx(vtx_cnt - 1); add_idx(vtx_cnt - 2); add_idx(vtx_cnt + 2); add_idx(vtx_cnt + 3);
							//		}
							//	}
							//}
							//else
							//{
							//	for (auto i = 0; i < points.size() - 1; i++)
							//	{
							//		if (i == 0)
							//		{
							//			auto p0 = points[0];
							//			auto p1 = points[1];
							//
							//			auto n0 = normals[0];
							//			auto n1 = normals[1];
							//
							//			auto vtx_cnt = cmd->vertices_count;
							//
							//			add_vtx(p0 + n0 * feather, uv, col_t);
							//			add_vtx(p0, uv, col_c);
							//			add_vtx(p0 - n0 * feather, uv, col_t);
							//			add_vtx(p1 + n1 * feather, uv, col_t);
							//			add_vtx(p1, uv, col_c);
							//			add_vtx(p1 - n1 * feather, uv, col_t);
							//			add_idx(vtx_cnt + 0); add_idx(vtx_cnt + 4); add_idx(vtx_cnt + 1); add_idx(vtx_cnt + 0); add_idx(vtx_cnt + 3); add_idx(vtx_cnt + 4);
							//			add_idx(vtx_cnt + 1); add_idx(vtx_cnt + 5); add_idx(vtx_cnt + 2); add_idx(vtx_cnt + 1); add_idx(vtx_cnt + 4); add_idx(vtx_cnt + 5);
							//		}
							//		else if (closed && i == points.size() - 2)
							//		{
							//			auto vtx_cnt = cmd->vertices_count;
							//
							//			add_idx(vtx_cnt - 3); add_idx(vtx_cnt0 + 1); add_idx(vtx_cnt - 2); add_idx(vtx_cnt - 3); add_idx(vtx_cnt0 + 0); add_idx(vtx_cnt0 + 1);
							//			add_idx(vtx_cnt - 2); add_idx(vtx_cnt0 + 2); add_idx(vtx_cnt - 1); add_idx(vtx_cnt - 2); add_idx(vtx_cnt0 + 1); add_idx(vtx_cnt0 + 2);
							//		}
							//		else
							//		{
							//			auto p1 = points[i + 1];
							//
							//			auto n1 = normals[i + 1];
							//
							//			auto vtx_cnt = cmd->vertices_count;
							//
							//			add_vtx(p1 + n1 * feather, uv, col_t);
							//			add_vtx(p1, uv, col_c);
							//			add_vtx(p1 - n1 * feather, uv, col_t);
							//			add_idx(vtx_cnt - 3); add_idx(vtx_cnt + 1); add_idx(vtx_cnt - 2); add_idx(vtx_cnt - 3); add_idx(vtx_cnt + 0); add_idx(vtx_cnt + 1);
							//			add_idx(vtx_cnt - 2); add_idx(vtx_cnt + 2); add_idx(vtx_cnt - 1); add_idx(vtx_cnt - 2); add_idx(vtx_cnt + 1); add_idx(vtx_cnt + 2);
							//		}
							//	}
							//}
							//
							//if (!closed)
							//{
							//	auto ext = max(feather, thickness);
							//
							//	{
							//		auto vtx_cnt = cmd->vertices_count;
							//
							//		auto p0 = points[0];
							//		auto p1 = points[1];
							//
							//		auto n0 = normals[0];
							//
							//		auto p = p0 - normalize(p1 - p0);
							//		add_vtx(p + n0 * ext, uv, col_t);
							//		add_vtx(p - n0 * ext, uv, col_t);
							//		add_vtx(p0 + n0 * ext, uv, col_t);
							//		add_vtx(p0, uv, col_c);
							//		add_vtx(p0 - n0 * ext, uv, col_t);
							//		add_idx(vtx_cnt + 1); add_idx(vtx_cnt + 4); add_idx(vtx_cnt + 2);
							//		add_idx(vtx_cnt + 1); add_idx(vtx_cnt + 3); add_idx(vtx_cnt + 4); 
							//		add_idx(vtx_cnt + 1); add_idx(vtx_cnt + 3); add_idx(vtx_cnt + 0);
							//	}
							//
							//	{
							//		auto vtx_cnt = cmd->vertices_count;
							//
							//		auto p0 = points[points.size() - 2];
							//		auto p1 = points[points.size() - 1];
							//
							//		auto n1 = normals[points.size() - 1];
							//
							//		auto p = p1 + normalize(p1 - p0);
							//		add_vtx(p1 + n1 * ext, uv, col_t);
							//		add_vtx(p1, uv, col_c);
							//		add_vtx(p1 - n1 * ext, uv, col_t);
							//		add_vtx(p + n1 * ext, uv, col_t);
							//		add_vtx(p - n1 * ext, uv, col_t);
							//		add_idx(vtx_cnt + 1); add_idx(vtx_cnt + 4); add_idx(vtx_cnt + 2);
							//		add_idx(vtx_cnt + 1); add_idx(vtx_cnt + 3); add_idx(vtx_cnt + 4);
							//		add_idx(vtx_cnt + 1); add_idx(vtx_cnt + 0); add_idx(vtx_cnt + 3);
							//	}
							//}
						}
						else
						{
							auto vtx_cnt0 = c.d.a.vtx_cnt;
							for (auto i = 0; i < info.points.size() - 1; i++)
							{
								if (i == 0)
								{
									auto p0 = info.points[0];
									auto p1 = info.points[1];

									auto n0 = normals[0];
									auto n1 = normals[1];

									auto pvtx = ed.buf_element_vtx.stag(4);
									pvtx[0].set(p0 + n0 * thickness, vec2(0.5), info.color);
									pvtx[1].set(p0 - n0 * thickness, vec2(0.5), info.color);
									pvtx[2].set(p1 + n1 * thickness, vec2(0.5), info.color);
									pvtx[3].set(p1 - n1 * thickness, vec2(0.5), info.color);

									auto pidx = ed.buf_element_idx.stag(6);
									pidx[0] = c.d.a.vtx_cnt + 0;
									pidx[1] = c.d.a.vtx_cnt + 1;
									pidx[2] = c.d.a.vtx_cnt + 3;
									pidx[3] = c.d.a.vtx_cnt + 0;
									pidx[4] = c.d.a.vtx_cnt + 3;
									pidx[5] = c.d.a.vtx_cnt + 2;

									c.d.a.vtx_cnt += 4;
									c.d.a.idx_cnt += 6;
								}
								else if (closed && i == info.points.size() - 2)
								{
									auto pidx = ed.buf_element_idx.stag(6);
									pidx[0] = c.d.a.vtx_cnt - 2;
									pidx[1] = c.d.a.vtx_cnt - 1;
									pidx[2] = vtx_cnt0 + 1;
									pidx[3] = c.d.a.vtx_cnt - 2;
									pidx[4] = vtx_cnt0 + 1;
									pidx[5] = vtx_cnt0 + 0;

									c.d.a.idx_cnt += 6;
								}
								else
								{
									auto p1 = info.points[i + 1];

									auto n1 = normals[i + 1];

									auto pvtx = ed.buf_element_vtx.stag(2);
									pvtx[0].set(p1 + n1 * thickness, vec2(0.5), info.color);
									pvtx[1].set(p1 - n1 * thickness, vec2(0.5), info.color);

									auto pidx = ed.buf_element_idx.stag(6);
									pidx[0] = c.d.a.vtx_cnt - 2;
									pidx[1] = c.d.a.vtx_cnt - 1;
									pidx[2] = c.d.a.vtx_cnt + 1;
									pidx[3] = c.d.a.vtx_cnt - 2;
									pidx[4] = c.d.a.vtx_cnt + 1;
									pidx[5] = c.d.a.vtx_cnt + 0;

									c.d.a.vtx_cnt += 2;
									c.d.a.idx_cnt += 6;
								}
							}
						}
					}
						break;
					case ElementDrawCmd::Text:
					{
						if (cmds.back().b || cmds.back().d.a.res_id != info.res_id)
						{
							auto& c = cmds.emplace_back();
							c.b = false;
							c.d.a.res_id = info.res_id;
						}
						auto& c = cmds.back();

						auto atlas = (graphics::FontAtlas*)ed.reses[info.res_id].v;
						auto pos = info.points[0];
						auto axes = mat2(info.points[1], info.points[2]);
						auto font_size = info.misc[0];
						auto p = vec2(0.f);
						for (auto ch : info.text)
						{
							if (ch == '\n')
							{
								p.y += font_size;
								p.x = 0.f;
							}
							else if (ch != '\r')
							{
								if (ch == '\t')
									ch = ' ';

								auto& g = atlas->get_glyph(ch, font_size);
								auto o = p + vec2(g.off);
								auto s = vec2(g.size);
								auto uv = g.uv;
								auto uv0 = vec2(uv.x, uv.y);
								auto uv1 = vec2(uv.z, uv.w);

								auto pvtx = ed.buf_element_vtx.stag(4);
								pvtx[0].set(pos + o * axes, uv0, info.color);
								pvtx[1].set(pos + o.x * axes[0] + (o.y - s.y) * axes[1], vec2(uv0.x, uv1.y), info.color);
								pvtx[2].set(pos + (o.x + s.x) * axes[0] + (o.y - s.y) * axes[1], uv1, info.color);
								pvtx[3].set(pos + (o.x + s.x) * axes[0] + o.y * axes[1], vec2(uv1.x, uv0.y), info.color);
								auto pidx = ed.buf_element_idx.stag(6);
								pidx[0] = c.d.a.vtx_cnt + 0;
								pidx[1] = c.d.a.vtx_cnt + 2;
								pidx[2] = c.d.a.vtx_cnt + 1;
								pidx[3] = c.d.a.vtx_cnt + 0;
								pidx[4] = c.d.a.vtx_cnt + 3;
								pidx[5] = c.d.a.vtx_cnt + 2;

								c.d.a.vtx_cnt += 4;
								c.d.a.idx_cnt += 6;

								p.x += g.advance;
							}
						}
					}
						break;
					case ElementDrawCmd::Scissor:
					{
						if (!cmds.back().b)
							cmds.emplace_back().b = true;
						if (scissor == info.misc)
							cmds.pop_back();
						else
							cmds.back().d.b = info.misc;
						scissor = info.misc;
					}
						break;
					}
				}
			}

			ed.buf_element_vtx.upload(cb);
			ed.buf_element_idx.upload(cb);

			cb->set_viewport(Rect(vec2(0.f), tar_sz));
			cb->set_scissor(Rect(vec2(0.f), tar_sz));
			if (nd.should_render)
				cb->begin_renderpass(rp_bgra8, fb_tars[tar_idx].get());
			else
			{
				auto cv = vec4(0.6f, 0.7f, 0.8f, 1.f);
				cb->begin_renderpass(rp_bgra8c, fb_tars[tar_idx].get(), &cv);
			}
			cb->bind_pipeline(ed.pl_element);
			cb->bind_vertex_buffer(ed.buf_element_vtx.buf.get(), 0);
			cb->bind_index_buffer(ed.buf_element_idx.buf.get(), graphics::IndiceTypeUint);
			cb->bind_descriptor_set(element::PLL_element::Binding_element, ed.ds_element.get());
			cb->push_constant_t(element::PLL_element::PushConstant{ 2.f / tar_sz });
			auto vtx_off = 0;
			auto idx_off = 0;
			for (auto& c : cmds)
			{
				if (!c.b)
				{
					cb->draw_indexed(c.d.a.idx_cnt, idx_off, vtx_off, 1, c.d.a.res_id);
					vtx_off += c.d.a.vtx_cnt;
					idx_off += c.d.a.idx_cnt;
				}
				else
					cb->set_scissor(c.d.b);
			}
			cb->end_renderpass();

			{
				auto sub = iv_tars[tar_idx]->get_sub();
				cb->image_barrier(iv_tars[tar_idx]->get_image(), { sub.base_level, 1, sub.base_layer, 1 }, graphics::ImageLayoutAttachment, graphics::ImageLayoutPresent);
			}
		}

		nd.should_render = false;
		ed.should_render = false;
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

		auto iv_white = img_white->get_view();
		auto iv_black = img_black->get_view();

		ds_back.reset(graphics::DescriptorSet::create(dsp, graphics::DescriptorSetLayout::get(device, L"post/post.dsl")));

		auto& ed = *_ed;

		ed.pl_element = graphics::Pipeline::get(device, L"element/element.pl");

		ed.buf_element_vtx.create(device, graphics::BufferUsageVertex, 360000);
		ed.buf_element_idx.create(device, graphics::BufferUsageIndex, 240000);
		ed.ds_element.reset(graphics::DescriptorSet::create(dsp, graphics::DescriptorSetLayout::get(device, L"element/element.dsl")));
		ed.reses.resize(element::DSL_element::images_count);
		for (auto i = 0; i < ed.reses.size(); i++)
		{
			ed.ds_element->set_image(element::DSL_element::images_binding, i, iv_white, sp_linear);

			auto& res = ed.reses[i];
			res.type = ElementResImage;
			res.v = iv_white;
		}

		auto& nd = *_nd;

		nd.tex_reses.resize(DSL_material::maps_count);
		nd.mat_reses.resize(_countof(DSL_material::MaterialInfos::material_infos));
		nd.mesh_reses.resize(64);

		for (auto& v : nd.meshes)
			v.resize(nd.mat_reses.size());
		
		for (auto& b : nd.buf_mesh_indirs)
			b.create(device, graphics::BufferUsageIndirect, 65536);

		nd.buf_mesh_vtx.create(device, graphics::BufferUsageVertex, 10000);
		nd.buf_mesh_idx.create(device, graphics::BufferUsageIndex, 10000);
		nd.buf_arm_mesh_vtx.create(device, graphics::BufferUsageVertex, 10000);
		nd.buf_arm_mesh_idx.create(device, graphics::BufferUsageIndex, 10000);

		nd.buf_render_data.create(device, graphics::BufferUsageUniform);
		nd.ds_render_data.reset(graphics::DescriptorSet::create(dsp, graphics::DescriptorSetLayout::get(device, L"render_data.dsl")));
		nd.ds_render_data->set_buffer(DSL_render_data::RenderData_binding, 0, nd.buf_render_data.buf.get());

		nd.buf_materials.create(device, graphics::BufferUsageStorage);
		nd.ds_material.reset(graphics::DescriptorSet::create(dsp, graphics::DescriptorSetLayout::get(device, L"material.dsl")));
		nd.ds_material->set_buffer(DSL_material::MaterialInfos_binding, 0, nd.buf_materials.buf.get());
		for (auto i = 0; i < nd.tex_reses.size(); i++)
			nd.ds_material->set_image(DSL_material::maps_binding, i, iv_white, sp_linear);

		nd.buf_transforms.create(device, graphics::BufferUsageStorage);
		nd.ds_mesh.reset(graphics::DescriptorSet::create(dsp, graphics::DescriptorSetLayout::get(device, L"mesh/mesh.dsl")));
		nd.ds_mesh->set_buffer(mesh::DSL_mesh::Transforms_binding, 0, nd.buf_transforms.buf.get());

		nd.buf_terrain.create(device, graphics::BufferUsageStorage);
		nd.ds_terrain.reset(graphics::DescriptorSet::create(dsp, graphics::DescriptorSetLayout::get(device, L"terrain/terrain.dsl")));
		nd.ds_terrain->set_buffer(terrain::DSL_terrain::TerrainInfos_binding, 0, nd.buf_terrain.buf.get());

		nd.buf_light_infos.create(device, graphics::BufferUsageStorage);
		nd.buf_grid_lights.create(device, graphics::BufferUsageStorage);
		nd.buf_dir_shadow_mats.create(device, graphics::BufferUsageStorage);
		nd.buf_pt_shadow_mats.create(device, graphics::BufferUsageStorage);
		nd.img_dir_shadow_maps.resize(DSL_light::dir_shadow_maps_count);
		nd.fb_dir_shadow_maps.resize(nd.img_dir_shadow_maps.size() * 4);
		auto rp_d16 = graphics::Renderpass::get(device, L"d16.rp");
		for (auto i = 0; i < nd.img_dir_shadow_maps.size(); i++)
		{
			nd.img_dir_shadow_maps[i].reset(graphics::Image::create(device, graphics::Format_Depth16, shadow_map_size, 1, 4,
				graphics::SampleCount_1, graphics::ImageUsageSampled | graphics::ImageUsageAttachment));
			nd.img_dir_shadow_maps[i]->change_layout(graphics::ImageLayoutUndefined, graphics::ImageLayoutShaderReadOnly);
			for (auto j = 0; j < 4; j++)
			{
				auto iv = graphics::ImageView::create(nd.img_dir_shadow_maps[i].get(), true, graphics::ImageView2D, { 0U, 1U, (uint)j, 1U });
				nd.fb_dir_shadow_maps[i * 4 + j].reset(graphics::Framebuffer::create(device, rp_d16, 1, &iv));
			}
		}
		nd.img_pt_shadow_maps.resize(DSL_light::pt_shadow_maps_count);
		nd.fb_pt_shadow_maps.resize(nd.img_pt_shadow_maps.size() * 6);
		for (auto i = 0; i < nd.img_pt_shadow_maps.size(); i++)
		{
			nd.img_pt_shadow_maps[i].reset(graphics::Image::create(device, graphics::Format_Depth16, vec2(shadow_map_size) * 0.5f, 1, 6,
				graphics::SampleCount_1, graphics::ImageUsageSampled | graphics::ImageUsageAttachment, true));
			nd.img_pt_shadow_maps[i]->change_layout(graphics::ImageLayoutUndefined, graphics::ImageLayoutShaderReadOnly);
			for (auto j = 0; j < 6; j++)
			{
				auto iv = graphics::ImageView::create(nd.img_pt_shadow_maps[i].get(), true, graphics::ImageView2D, { 0U, 1U, (uint)j, 1U });
				nd.fb_pt_shadow_maps[i * 6 + j].reset(graphics::Framebuffer::create(device, rp_d16, 1, &iv));
			}
		}
		nd.ds_light.reset(graphics::DescriptorSet::create(dsp, graphics::DescriptorSetLayout::get(device, L"light.dsl")));
		nd.ds_light->set_buffer(DSL_light::LightInfos_binding, 0, nd.buf_light_infos.buf.get());
		nd.ds_light->set_buffer(DSL_light::GridLights_binding, 0, nd.buf_grid_lights.buf.get());
		nd.ds_light->set_buffer(DSL_light::DirShadowMats_binding, 0, nd.buf_dir_shadow_mats.buf.get());
		nd.ds_light->set_buffer(DSL_light::PtShadowMats_binding, 0, nd.buf_pt_shadow_mats.buf.get());
		auto sp_shadow = graphics::Sampler::get(device, graphics::FilterLinear, graphics::FilterLinear, false, graphics::AddressClampToBorder);
		for (auto i = 0; i < nd.img_dir_shadow_maps.size(); i++)
		{
			auto iv = graphics::ImageView::create(nd.img_dir_shadow_maps[i].get(), true, graphics::ImageView2DArray, { 0, 1, 0, 4 });
			nd.ds_light->set_image(DSL_light::dir_shadow_maps_binding, i, iv, sp_shadow);
		}
		for (auto i = 0; i < nd.img_pt_shadow_maps.size(); i++)
		{
			auto iv = graphics::ImageView::create(nd.img_pt_shadow_maps[i].get(), true, graphics::ImageViewCube, { 0, 1, 0, 6 });
			nd.ds_light->set_image(DSL_light::pt_shadow_maps_binding, i, iv, sp_shadow);
		}
		nd.ds_light->set_image(DSL_light::sky_box_binding, 0, iv_black, sp_linear);
		nd.ds_light->set_image(DSL_light::sky_irr_binding, 0, iv_black, sp_linear);
		nd.ds_light->set_image(DSL_light::sky_rad_binding, 0, iv_black, sp_linear);
		nd.ds_light->set_image(DSL_light::sky_lut_binding, 0, iv_black, sp_linear);

		nd.pll_mesh_fwd = graphics::PipelineLayout::get(device, L"mesh/forward.pll");
		nd.pll_mesh_gbuf = graphics::PipelineLayout::get(device, L"mesh/gbuffer.pll");
		nd.pll_terrain_gbuf = graphics::PipelineLayout::get(device, L"terrain/gbuffer.pll");

		nd.pl_def = graphics::Pipeline::get(device, L"deferred/deferred.pl");
		nd.pl_nor_dat = graphics::Pipeline::get(device, L"deferred/normal_data.pl");
		nd.ds_def.reset(graphics::DescriptorSet::create(dsp, graphics::DescriptorSetLayout::get(device, L"deferred/deferred.dsl")));

		nd.pl_gamma = graphics::Pipeline::get(device, L"post/gamma.pl");
	}

	void sRendererPrivate::update()
	{
		if (fb_tars.empty() || (!dirty && !always_update))
			return;

		auto& ed = *_ed;
		ed.scissor = Rect(vec2(0.f), tar_sz);
		for (auto i = 0; i < _countof(ed.layers); i++)
			ed.layers[i].clear();
		ed.buf_element_vtx.stag_num = 0;
		ed.buf_element_idx.stag_num = 0;
		if (world->first_element)
		{
			ed.should_render = true;
			element_render(0, world->first_element->get_component_i<cElementPrivate>(0));
		}
		else
			ed.should_render = false;

		auto& nd = *_nd;
		nd.dir_shadows.clear();
		nd.pt_shadows.clear();
		for (auto& v : nd.meshes)
		{
			for (auto& vec : v)
				vec.clear();
		}
		nd.terrains.clear();
		{ // TODO
			auto& data = nd.buf_grid_lights.set_item(0);
			data.dir_count = 0;
			data.pt_count = 0;
		}
		if (world->first_node && camera)
		{
			nd.should_render = true;
			node_render(world->first_node->get_component_i<cNodePrivate>(0));
		}
		else
			nd.should_render = false;

		dirty = false;
	}

	sRenderer* sRenderer::create(void* parms)
	{
		return f_new<sRendererPrivate>((sRendererParms*)parms);
	}
}
