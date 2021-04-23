#include "../../graphics/device.h"
#include "../../graphics/buffer.h"
#include "../../graphics/image.h"
#include "../../graphics/renderpass.h"
#include "../../graphics/shader.h"
#include "../../graphics/font.h"
#include "../../graphics/model.h"
#include "../world_private.h"
#include "../components/element_private.h"
#include "../components/node_private.h"
#include "../components/camera_private.h"
#include "renderer_private.h"

#include <element/element.dsl.h>
#include <element/element.pll.h>
#include <render_data.dsl.h>
#include <transform.dsl.h>
#include <material.dsl.h>
#include <light.dsl.h>
#include <mesh/defe_geom.pll.h>
#include <deferred/shade.dsl.h>
#include <deferred/shade.pll.h>
#include <post/post.dsl.h>

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
			graphics::BufferCopy cpy;
			cpy.size = stag_num * sizeof(T);
			cb->copy_buffer(stagbuf.get(), buf.get(), 1, &cpy);
			cb->buffer_barrier(buf.get(), graphics::AccessTransferWrite, access);
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

		auto& set_item(uint idx)
		{
			auto& [items] = *pstag;
			fassert(idx < _countof(items));

			auto& item = items[idx];

			graphics::BufferCopy cpy;
			cpy.dst_off = idx * sizeof(item);
			cpy.size = sizeof(item);
			cpies.push_back(cpy);

			return item;
		}
	};

	template <class T>
	struct SequentialArrayStorageBuffer : StorageBuffer<T>
	{
		using StorageBuffer<T>::pstag;
		using StorageBuffer<T>::cpies;

		uint n = 0;

		auto& add_item()
		{
			auto& [items] = *pstag;
			fassert(n < _countof(items));

			auto& item = items[n];

			if (cpies.empty())
				cpies.emplace_back();
			cpies.back().size += sizeof(item);

			n++;
			return item;
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
		graphics::model::Mesh* mesh = nullptr;
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

		std::vector<cLightPrivate*>						lights;
		std::vector<std::vector<std::pair<uint, uint>>> meshes;

		SequentialBuffer<graphics::DrawIndexedIndirectCommand>	buf_indirs;

		SparseBuffer<MeshVertex>	buf_mesh_vtx;
		SparseBuffer<uint>			buf_mesh_idx;
		SparseBuffer<ArmMeshVertex>	buf_arm_mesh_vtx;
		SparseBuffer<uint>			buf_arm_mesh_idx;

		StorageBuffer<DSL_render_data::RenderData>					buf_render_data;
		UniPtr<graphics::DescriptorSet>								ds_render_data;
		SequentialArrayStorageBuffer<DSL_transform::Transforms>		buf_transforms;
		UniPtr<graphics::DescriptorSet>								ds_transform;
		ArrayStorageBuffer<DSL_material::MaterialInfos>				buf_materials;
		UniPtr<graphics::DescriptorSet>								ds_material;

		UniPtr<graphics::Image> img_dep;
		UniPtr<graphics::Image> img_alb_met; // albedo, metallic
		UniPtr<graphics::Image> img_nor_rou; // normal, roughness

		StorageBuffer<DSL_light::LightInfos>		buf_light_infos;
		StorageBuffer<DSL_light::GridLights>		buf_grid_lights;
		StorageBuffer<DSL_light::DirShadowMats>		buf_dir_shadow_mats;
		StorageBuffer<DSL_light::PtShadowMats>		buf_pt_shadow_mats;
		std::vector<UniPtr<graphics::Image>>		img_dir_maps;
		std::vector<UniPtr<graphics::Image>>		img_pt_maps;
		UniPtr<graphics::DescriptorSet>				ds_light;

		UniPtr<graphics::Framebuffer> fb_def;

		std::vector<MaterialPipeline>	pl_mats[MaterialUsageCount];
		graphics::Pipeline* pl_defe_shad;
		UniPtr<graphics::DescriptorSet>	ds_defe_shad;

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
		auto iv_wht = img_wht->get_view();
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
			v = iv_wht;
		}

		if (idx == -1)
		{
			for (auto i = 1; i < ed.reses.size(); i++)
			{
				if (ed.reses[i].v == iv_wht)
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
			{
				auto n = normalize((normal + normals[i]) * 0.5f);
				normals[i] = n / dot(n, normal);
			}
			else
				normals[i] = normal;

			if (closed && i + 1 == points.size() - 1)
			{
				auto n = normalize((normal + normals[0]) * 0.5f);
				normals.front() = normals.back() = n / dot(n, normal);
			}
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
		nd.ds_material->set_image(DSL_material::maps_binding, idx, tex ? tex : img_wht->get_view(), sp);

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

	int sRendererPrivate::set_mesh_res(int idx, graphics::model::Mesh* mesh)
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
				for (auto i = 0; i < dst.idx_cnt; i++)
					pidx[i] = dst.idx_off + aidx[i];

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

	int sRendererPrivate::find_mesh_res(graphics::model::Mesh* mesh) const
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
		graphics::PolygonMode polygon_mode = graphics::PolygonModeFill;
		graphics::CullMode cull_mode = graphics::CullModeBack;
		auto depth_test = true;
		auto depth_write = true;
		auto use_mat = true;
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
		}
		else if (find_define("PICKUP"))
			use_mat = false;
		else if (find_define("OUTLINE"))
		{
			use_mat = false;
			depth_test = false;
			depth_write = false;
		}
		if (find_define("DOUBLE_SIDE"))
			cull_mode == graphics::CullModeNone;
		if (use_mat && !mat.empty())
		{
			defines.push_back("MAT");
			substitutes.emplace_back("MAT_FILE", mat.string());
		}

		auto defines_str = [&]() {
			std::string ret;
			for (auto& t : defines)
			{
				ret += t;
				ret += ' ';
			}
			return ret;
		};

		auto substitutes_str = [&]() {
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
		//case MaterialForMeshShadow:
		//	defines.push_back("SHADOW_PASS");
		case MaterialForMesh:
		{
			defines.push_back("DEFERRED");
			graphics::Shader* shaders[] = {
				graphics::Shader::get(device, L"mesh/defe_geom.vert", defines_str().c_str(), substitutes_str().c_str()),
				graphics::Shader::get(device, L"mesh/defe_geom.frag", defines_str().c_str(), substitutes_str().c_str())
			};
			graphics::GraphicsPipelineInfo info;
			info.renderpass = graphics::Renderpass::get(device, L"deferred.rp");
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
			info.depth_test = depth_test;
			info.depth_write = depth_write;
			ret = graphics::Pipeline::create(device, _countof(shaders), shaders, graphics::PipelineLayout::get(device, L"mesh/defe_geom.pll"), info);
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
		//case MaterialForTerrain:
		//{
		//	graphics::Shader* shaders[] = {
		//		graphics::Shader::get(device, L"terrain/terrain.vert", defines, {}),
		//		graphics::Shader::get(device, L"terrain/terrain.tesc", defines, {}),
		//		graphics::Shader::get(device, L"terrain/terrain.tese", defines, {}),
		//		graphics::Shader::get(device, L"terrain/terrain.frag", defines, substitutes)
		//	};
		//	VertexInfo vi;
		//	vi.primitive_topology = PrimitiveTopologyPatchList;
		//	vi.patch_control_points = 4;
		//	RasterInfo rst;
		//	rst.polygon_mode = polygon_mode;
		//	DepthInfo dep;
		//	dep.test = depth_test;
		//	dep.write = depth_write;
		//	ret = PipelinePrivate::create(device, shaders, terrain_pipeline_layout, mesh_renderpass.get(), 0, &vi, &rst, &dep);
		//}
		//	break;
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

	void sRendererPrivate::add_light(cNodePtr node, const vec3& color, bool cast_shadow)
	{
		auto& nd = *_nd;
	}

	void sRendererPrivate::draw_mesh(cNodePtr node, uint mesh_id)
	{
		auto& nd = *_nd;

		node->update_transform();

		auto idx = nd.buf_transforms.n;

		auto& data = nd.buf_transforms.add_item();
		data.mat = node->transform;
		data.nor = mat4(node->rot);

		nd.meshes[nd.mesh_reses[mesh_id].mat_id].emplace_back(idx, mesh_id);
	}

	void sRendererPrivate::set_targets(uint tar_cnt, graphics::ImageView* const* ivs)
	{
		fb_tars.clear();
		fb_tars.resize(tar_cnt);
		for (auto i = 0; i < tar_cnt; i++)
			fb_tars[i].reset(graphics::Framebuffer::create(device, rp_rgba8c, 1, &ivs[i]));
		tar_sz = ivs[0]->get_image()->get_size();

		img_back.reset(graphics::Image::create(device, graphics::Format_R16G16B16A16_SFLOAT, tar_sz, 1, 1,
			graphics::SampleCount_1, graphics::ImageUsageSampled | graphics::ImageUsageAttachment));
		ds_back->set_image(DSL_post::image_binding, 0, img_back->get_view(), sp_nearest);

		auto& nd = *_nd;

		nd.img_dep.reset(graphics::Image::create(device, graphics::Format_Depth16, tar_sz, 1, 1,
			graphics::SampleCount_1, graphics::ImageUsageSampled | graphics::ImageUsageAttachment));
		nd.img_alb_met.reset(graphics::Image::create(device, graphics::Format_R8G8B8A8_UNORM, tar_sz, 1, 1,
			graphics::SampleCount_1, graphics::ImageUsageSampled | graphics::ImageUsageAttachment));
		nd.img_nor_rou.reset(graphics::Image::create(device, graphics::Format_R8G8B8A8_UNORM, tar_sz, 1, 1,
			graphics::SampleCount_1, graphics::ImageUsageSampled | graphics::ImageUsageAttachment));

		{
			graphics::ImageView* vs[4];
			vs[0] = nd.img_alb_met->get_view();
			vs[1] = nd.img_nor_rou->get_view();
			vs[2] = nd.img_dep->get_view();
			vs[3] = img_back->get_view();
			nd.fb_def.reset(graphics::Framebuffer::create(device, graphics::Renderpass::get(device, L"deferred.rp"), _countof(vs), vs));
		}

		nd.ds_defe_shad->set_image(DSL_shade::img_alb_met_binding, 0, nd.img_alb_met->get_view(), sp_nearest);
		nd.ds_defe_shad->set_image(DSL_shade::img_nor_rou_binding, 0, nd.img_nor_rou->get_view(), sp_nearest);
		nd.ds_defe_shad->set_image(DSL_shade::img_dep_binding, 0, nd.img_dep->get_view(), sp_nearest);

	}

	void sRendererPrivate::record(uint tar_idx, graphics::CommandBuffer* cb)
	{
		auto tar = fb_tars[tar_idx].get();

		auto& nd = *_nd;
		if (nd.should_render)
		{
			nd.should_render = false;

			{
				auto node = camera->node;
				node->update_transform();
				auto view = mat4(node->rot);
				view[3] = vec4(node->g_pos, 1.f);
				view = inverse(view);
				auto proj = perspective(radians(camera->fovy), tar_sz.x / tar_sz.y, camera->near, camera->far);
				proj[1][1] *= -1.f;
				auto& data = *(nd.buf_render_data.pstag);
				data.proj_view = proj * view;
			}
			nd.buf_render_data.cpy_whole();
			nd.buf_render_data.upload(cb);

			nd.buf_transforms.upload(cb);
			nd.buf_transforms.n = 0;

			nd.buf_indirs.stag_num = 0;
			for (auto mat_id = 0; mat_id < nd.meshes.size(); mat_id++)
			{
				auto& vec = nd.meshes[mat_id];
				if (!vec.empty())
				{
					auto indirs = nd.buf_indirs.stag(vec.size());
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
			nd.buf_indirs.upload(cb);

			auto vp = Rect(vec2(0.f), tar_sz);
			cb->set_viewport(vp);
			cb->set_scissor(vp);
			vec4 cvs[] = {
				vec4(0.f, 0.f, 0.f, 0.f),
				vec4(0.f, 0.f, 0.f, 0.f),
				vec4(1.f, 0.f, 0.f, 0.f),
				vec4(0.f, 0.f, 0.f, 0.f)
			};
			cb->begin_renderpass(nullptr, nd.fb_def.get(), cvs);
			cb->bind_pipeline_layout(graphics::PipelineLayout::get(device, L"mesh/defe_geom.pll"));
			cb->bind_vertex_buffer(nd.buf_mesh_vtx.buf.get(), 0);
			cb->bind_index_buffer(nd.buf_mesh_idx.buf.get(), graphics::IndiceTypeUint);
			{
				graphics::DescriptorSet* sets[PLL_defe_geom::Binding_Max];
				sets[PLL_defe_geom::Binding_render_data] = nd.ds_render_data.get();
				sets[PLL_defe_geom::Binding_transform] = nd.ds_transform.get();
				sets[PLL_defe_geom::Binding_material] = nd.ds_material.get();
				cb->bind_descriptor_sets(0, _countof(sets), sets);
			}
			auto indir_off = 0;
			for (auto mat_id = 0; mat_id < nd.meshes.size(); mat_id++)
			{
				auto& vec = nd.meshes[mat_id];
				if (!vec.empty())
				{
					cb->bind_pipeline(nd.mat_reses[mat_id].get_pl(this, MaterialForMesh));
					cb->draw_indexed_indirect(nd.buf_indirs.buf.get(), indir_off, vec.size());
					indir_off += vec.size();
				}
			}
			cb->next_pass();
			cb->bind_pipeline(nd.pl_defe_shad);
			{
				graphics::DescriptorSet* sets[PLL_shade::Binding_Max];
				sets[PLL_shade::Binding_shade] = nd.ds_defe_shad.get();
				sets[PLL_shade::Binding_light] = nd.ds_light.get();
				cb->bind_descriptor_sets(0, _countof(sets), sets);
			}
			cb->draw(3, 1, 0, 0);
			cb->end_renderpass();

			cb->image_barrier(img_back.get(), {}, graphics::ImageLayoutShaderReadOnly, graphics::ImageLayoutAttachment, graphics::AccessColorAttachmentWrite);
			cb->begin_renderpass(rp_rgba8, tar);
			cb->bind_pipeline(nd.pl_gamma);
			cb->bind_descriptor_set(0, ds_back.get());
			cb->draw(3, 1, 0, 0);
			cb->end_renderpass();
		}

		auto& ed = *_ed;
		if (ed.should_render)
		{
			ed.should_render = false;

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
									pidx[1] = c.d.a.vtx_cnt + 3;
									pidx[2] = c.d.a.vtx_cnt + 1;
									pidx[3] = c.d.a.vtx_cnt + 0;
									pidx[4] = c.d.a.vtx_cnt + 2;
									pidx[5] = c.d.a.vtx_cnt + 3;

									c.d.a.vtx_cnt += 4;
									c.d.a.idx_cnt += 6;
								}
								else if (closed && i == info.points.size() - 2)
								{
									auto pidx = ed.buf_element_idx.stag(6);
									pidx[0] = c.d.a.vtx_cnt - 2;
									pidx[1] = vtx_cnt0 + 1;
									pidx[2] = c.d.a.vtx_cnt - 1;
									pidx[3] = c.d.a.vtx_cnt - 2;
									pidx[4] = vtx_cnt0 + 0;
									pidx[5] = vtx_cnt0 + 1;

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
									pidx[1] = c.d.a.vtx_cnt + 1;
									pidx[2] = c.d.a.vtx_cnt - 1;
									pidx[3] = c.d.a.vtx_cnt - 2;
									pidx[4] = c.d.a.vtx_cnt + 0;
									pidx[5] = c.d.a.vtx_cnt + 1;

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
			auto cv = vec4(1.f, 1.f, 1.f, 1.f);
			cb->begin_renderpass(nd.should_render ? rp_rgba8 : rp_rgba8c, tar, &cv);
			cb->bind_pipeline(ed.pl_element);
			cb->bind_vertex_buffer(ed.buf_element_vtx.buf.get(), 0);
			cb->bind_index_buffer(ed.buf_element_idx.buf.get(), graphics::IndiceTypeUint);
			cb->bind_descriptor_set(PLL_element::Binding_element, ed.ds_element.get());
			cb->push_constant_t(0, PLL_element::PushConstant{ 2.f / tar_sz });
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
		}
	}

	const auto shadow_map_size = uvec2(1024);

	void sRendererPrivate::on_added()
	{
		device = graphics::Device::get_default();
		dsp = graphics::DescriptorPool::get_default(device);
		sp_nearest = graphics::Sampler::get(device, graphics::FilterNearest, graphics::FilterNearest, false, graphics::AddressClampToEdge);
		sp_linear = graphics::Sampler::get(device, graphics::FilterLinear, graphics::FilterLinear, false, graphics::AddressClampToEdge);

		rp_rgba8c = graphics::Renderpass::get(device, L"rgba8c.rp");
		rp_rgba8 = graphics::Renderpass::get(device, L"rgba8.rp");

		img_wht.reset(graphics::Image::create(device, graphics::Format_R8G8B8A8_UNORM, uvec2(1), 1, 1,
			graphics::SampleCount_1, graphics::ImageUsageTransferDst | graphics::ImageUsageSampled));
		img_wht->clear(graphics::ImageLayoutUndefined, graphics::ImageLayoutShaderReadOnly, cvec4(255));
		auto iv_wht = img_wht->get_view();

		ds_back.reset(graphics::DescriptorSet::create(dsp, graphics::DescriptorSetLayout::get(device, L"post/post.dsl")));

		auto& ed = *_ed;

		ed.pl_element = graphics::Pipeline::get(device, L"element/element.pl");

		ed.buf_element_vtx.create(device, graphics::BufferUsageVertex, 360000);
		ed.buf_element_idx.create(device, graphics::BufferUsageIndex, 240000);
		ed.ds_element.reset(graphics::DescriptorSet::create(dsp, graphics::DescriptorSetLayout::get(device, L"element/element.dsl")));
		ed.reses.resize(DSL_element::images_count);
		for (auto i = 0; i < ed.reses.size(); i++)
		{
			ed.ds_element->set_image(DSL_element::images_binding, i, iv_wht, sp_linear);

			auto& res = ed.reses[i];
			res.type = ElementResImage;
			res.v = iv_wht;
		}

		auto& nd = *_nd;

		nd.tex_reses.resize(DSL_material::maps_count);
		nd.mat_reses.resize(_countof(DSL_material::MaterialInfos::material_infos));
		nd.mesh_reses.resize(64);

		nd.meshes.resize(nd.mat_reses.size());
		
		nd.buf_indirs.create(device, graphics::BufferUsageIndirect, 65536);

		nd.buf_mesh_vtx.create(device, graphics::BufferUsageVertex, 10000);
		nd.buf_mesh_idx.create(device, graphics::BufferUsageIndex, 10000);
		nd.buf_arm_mesh_vtx.create(device, graphics::BufferUsageVertex, 10000);
		nd.buf_arm_mesh_idx.create(device, graphics::BufferUsageIndex, 10000);

		nd.buf_render_data.create(device, graphics::BufferUsageUniform);
		nd.ds_render_data.reset(graphics::DescriptorSet::create(dsp, graphics::DescriptorSetLayout::get(device, L"render_data.dsl")));
		nd.ds_render_data->set_buffer(DSL_render_data::RenderData_binding, 0, nd.buf_render_data.buf.get());

		nd.buf_transforms.create(device, graphics::BufferUsageStorage);
		nd.ds_transform.reset(graphics::DescriptorSet::create(dsp, graphics::DescriptorSetLayout::get(device, L"transform.dsl")));
		nd.ds_transform->set_buffer(DSL_transform::Transforms_binding, 0, nd.buf_transforms.buf.get());

		nd.buf_materials.create(device, graphics::BufferUsageStorage);
		nd.ds_material.reset(graphics::DescriptorSet::create(dsp, graphics::DescriptorSetLayout::get(device, L"material.dsl")));
		nd.ds_material->set_buffer(DSL_material::MaterialInfos_binding, 0, nd.buf_materials.buf.get());
		for (auto i = 0; i < nd.tex_reses.size(); i++)
			nd.ds_material->set_image(DSL_material::maps_binding, i, iv_wht, sp_linear);

		nd.buf_light_infos.create(device, graphics::BufferUsageStorage);
		nd.buf_grid_lights.create(device, graphics::BufferUsageStorage);
		nd.buf_dir_shadow_mats.create(device, graphics::BufferUsageStorage);
		nd.buf_pt_shadow_mats.create(device, graphics::BufferUsageStorage);
		nd.img_dir_maps.resize(DSL_light::dir_maps_count);
		for (auto i = 0; i < nd.img_dir_maps.size(); i++)
		{
			nd.img_dir_maps[i].reset(graphics::Image::create(device, graphics::Format_R16_SFLOAT, shadow_map_size, 1, 4,
				graphics::SampleCount_1, graphics::ImageUsageSampled | graphics::ImageUsageAttachment));
		}
		nd.img_pt_maps.resize(DSL_light::pt_maps_count);
		for (auto i = 0; i < nd.img_pt_maps.size(); i++)
		{
			nd.img_pt_maps[i].reset(graphics::Image::create(device, graphics::Format_R16_SFLOAT, vec2(shadow_map_size) * 0.5f, 1, 6,
				graphics::SampleCount_1, graphics::ImageUsageSampled | graphics::ImageUsageAttachment));
		}
		nd.ds_light.reset(graphics::DescriptorSet::create(dsp, graphics::DescriptorSetLayout::get(device, L"light.dsl")));
		nd.ds_light->set_buffer(DSL_light::LightInfos_binding, 0, nd.buf_light_infos.buf.get());
		nd.ds_light->set_buffer(DSL_light::GridLights_binding, 0, nd.buf_grid_lights.buf.get());
		nd.ds_light->set_buffer(DSL_light::DirShadowMats_binding, 0, nd.buf_dir_shadow_mats.buf.get());
		nd.ds_light->set_buffer(DSL_light::PtShadowMats_binding, 0, nd.buf_pt_shadow_mats.buf.get());
		for (auto i = 0; i < nd.img_dir_maps.size(); i++)
			nd.ds_light->set_image(DSL_light::dir_maps_binding, i, nd.img_dir_maps[i]->get_view(), sp_linear);
		for (auto i = 0; i < nd.img_pt_maps.size(); i++)
			nd.ds_light->set_image(DSL_light::pt_maps_binding, i, nd.img_pt_maps[i]->get_view(), sp_linear);

		nd.pl_defe_shad = graphics::Pipeline::get(device, L"deferred/shade.pl");

		nd.ds_defe_shad.reset(graphics::DescriptorSet::create(dsp, graphics::DescriptorSetLayout::get(device, L"deferred/shade.dsl")));

		nd.pl_gamma = graphics::Pipeline::get(device, L"post/gamma.pl");
	}

	void sRendererPrivate::update()
	{
		if (fb_tars.empty() || (!dirty && !always_update))
			return;

		if (world->first_element)
		{
			auto& ed = *_ed;
			ed.should_render = true;
			ed.scissor = Rect(vec2(0.f), tar_sz);
			for (auto i = 0; i < _countof(ed.layers); i++)
				ed.layers[i].clear();
			ed.buf_element_vtx.stag_num = 0;
			ed.buf_element_idx.stag_num = 0;
			element_render(0, world->first_element->get_component_i<cElementPrivate>(0));
		}

		if (world->first_node && camera)
		{
			auto& nd = *_nd;
			nd.should_render = true;
			for (auto i = 0; i < nd.meshes.size(); i++)
				nd.meshes[i].clear();
			node_render(world->first_node->get_component_i<cNodePrivate>(0));
		}

		dirty = false;
	}

	sRenderer* sRenderer::create(void* parms)
	{
		return f_new<sRendererPrivate>((sRendererParms*)parms);
	}
}
