#include <flame/graphics/device.h>
#include <flame/graphics/buffer.h>
#include <flame/graphics/image.h>
#include <flame/graphics/renderpass.h>
#include <flame/graphics/shader.h>
#include <flame/graphics/command.h>
#include <flame/graphics/swapchain.h>
#include <flame/graphics/font.h>
#include <flame/graphics/model.h>
#include <flame/graphics/canvas.h>
#include "../world_private.h"
#include "../components/element_private.h"
#include "../components/node_private.h"
#include "../components/camera_private.h"
#include "renderer_private.h"

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
	void SequentialBuffer<T>::rebuild()
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

	template <class T>
	void SequentialBuffer<T>::create(graphics::Device* device, graphics::BufferUsageFlags usage, uint _capacity)
	{
		capacity = _capacity;
		access = usage2access(usage);
		auto size = capacity * sizeof(T);
		buf.reset(graphics::Buffer::create(device, size, graphics::BufferUsageTransferDst | usage, graphics::MemoryPropertyDevice));
		stagbuf.reset(graphics::Buffer::create(device, size, graphics::BufferUsageTransferSrc, graphics::MemoryPropertyHost | graphics::MemoryPropertyCoherent));
		stagbuf->map();
		pstag = (T*)stagbuf->get_mapped();
	}

	template <class T>
	void SequentialBuffer<T>::push(uint cnt, const T* p)
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

	template <class T>
	T* SequentialBuffer<T>::stag(uint cnt)
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

	template <class T>
	void SequentialBuffer<T>::upload(graphics::CommandBuffer* cb)
	{
		graphics::BufferCopy cpy;
		cpy.size = stag_num * sizeof(T);
		cb->copy_buffer(stagbuf.get(), buf.get(), 1, &cpy);
		cb->buffer_barrier(buf.get(), graphics::AccessTransferWrite, access);
	}

	template <class T>
	void SparseBuffer<T>::create(graphics::Device* device, graphics::BufferUsageFlags usage, uint _capacity)
	{
		capacity = _capacity;
		access = usage2access(usage);
		auto size = capacity * sizeof(T);
		buf.reset(graphics::Buffer::create(device, size, graphics::BufferUsageTransferDst | usage, graphics::MemoryPropertyDevice));
		stag_capacity = 100;
		stagbuf.reset(graphics::Buffer::create(device, sizeof(T) * stag_capacity, graphics::BufferUsageTransferSrc, graphics::MemoryPropertyHost | graphics::MemoryPropertyCoherent));
		pstag = (T*)stagbuf->map();
	}

	template <class T>
	T* SparseBuffer<T>::alloc(uint n)
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

	template <class T>
	void SparseBuffer<T>::free(T* p)
	{

	}

	template <class T>
	void SparseBuffer<T>::upload(graphics::CommandBuffer* cb)
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

	template <class T>
	void StorageBuffer<T>::create(graphics::Device* device, graphics::BufferUsageFlags usage)
	{
		buf.reset(graphics::Buffer::create(device, sizeof(T), graphics::BufferUsageTransferDst | usage, graphics::MemoryPropertyDevice));
		stagbuf.reset(graphics::Buffer::create(device, sizeof(T), graphics::BufferUsageTransferSrc, graphics::MemoryPropertyHost | graphics::MemoryPropertyCoherent));
		pstag = (T*)stagbuf->map();
	}

	template <class T>
	void StorageBuffer<T>::upload(graphics::CommandBuffer* cb)
	{
		cb->copy_buffer(stagbuf.get(), buf.get(), cpies.size(), cpies.data());
		cb->buffer_barrier(buf.get(), graphics::AccessTransferWrite, graphics::AccessShaderRead);
		cpies.clear();
	}

	sRendererPrivate::sRendererPrivate(sRendererParms* _parms)
	{
		sRendererParms parms;
		if (_parms)
			parms = *_parms;
	}

	void sRendererPrivate::set_targets()
	{
		fb_tars.clear();
		auto count = swapchain->get_images_count();
		fb_tars.resize(count);
		for (auto i = 0; i < count; i++)
		{
			auto v = swapchain->get_image(i)->get_view();
			fb_tars[i].reset(graphics::Framebuffer::create(device, rp_rgba8c, 1, &v));
		}
		tar_size = swapchain->get_image(0)->get_size();
	}

	uint sRendererPrivate::element_render(uint layer, cElementPrivate* element)
	{
		auto e = element->entity;
		if (!e->global_visibility)
			return layer;

		element->parent_scissor = element_drawing_scissor;
		element->update_transform();
		auto culled = !element_drawing_scissor.overlapping(element->aabb);
		if (element->culled != culled)
		{
			element->culled = culled;
			e->component_data_changed(element, S<"culled"_h>);
		}
		if (culled)
			return layer;

		element->draw2(layer, this);

		auto clipping = false;
		Rect last_scissor;
		if (element->clipping && !(element_drawing_scissor == element->aabb))
		{
			element->layer_policy = 2;

			clipping = true;
			last_scissor = element_drawing_scissor;
			element_drawing_scissor = element->aabb;
			auto& info = element_drawing_layers[layer].emplace_back();
			info.type = ElementDrawCmd::Scissor;
			info.misc = element_drawing_scissor;
		}

		auto _layer = layer;
		for (auto& d : element->drawers2)
			_layer = max(_layer, d->call(layer, this));

		_layer++;
		auto max_layer = _layer;
		for (auto& c : e->children)
		{
			auto celement = c->get_component_t<cElementPrivate>();
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
			element_drawing_scissor = last_scissor;
			auto& info = element_drawing_layers[max_layer + 1].emplace_back();
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

		auto node = e->get_component_t<cNodePrivate>();
		if (node)
		{
			node->update_transform();
			for (auto& d : node->drawers2)
				d->call(this);
		}

		for (auto& c : e->children)
		{
			auto cnode = c->get_component_t<cNodePrivate>();
			if (cnode)
				node_render(cnode);
		}
	}

	void* sRendererPrivate::get_element_res(uint idx, ElementResType* type) const
	{
		auto& res = element_reses[idx];
		if (type)
			*type = res.type;
		return res.v;
	}

	int sRendererPrivate::set_element_res(int idx, ElementResType type, void* v)
	{
		auto iv_wht = img_wht->get_view(0);

		if (!v)
		{
			if (type != ElementResImage)
				return - 1;
			v = iv_wht;
		}

		if (idx == -1)
		{
			for (auto i = 1; i < element_reses.size(); i++)
			{
				if (element_reses[i].v == iv_wht)
				{
					idx = i;
					break;
				}
			}
		}
		if (idx == -1)
			return -1;

		auto& res = element_reses[idx];
		res.type = type;
		res.v = v;

		auto bd = graphics::DescriptorSetLayout::get(device, L"element/element.dsl")->find_binding("images");
		switch (type)
		{
		case ElementResImage:
			ds_element->set_image(bd, idx, (graphics::ImageView*)v, graphics::Sampler::get(device, graphics::FilterLinear, graphics::FilterLinear, false, graphics::AddressClampToEdge));
			break;
		case ElementResAtlas:
		{
			auto ia = (graphics::ImageAtlas*)v;
			ds_element->set_image(bd, idx, ia->get_image()->get_view(0), ia->get_border() ?
				graphics::Sampler::get(device, graphics::FilterLinear, graphics::FilterLinear, false, graphics::AddressClampToEdge) :
				graphics::Sampler::get(device, graphics::FilterNearest, graphics::FilterNearest, false, graphics::AddressClampToEdge));
		}
			break;
		case ElementResFont:
			ds_element->set_image(bd, idx, ((graphics::FontAtlas*)v)->get_view(), graphics::Sampler::get(device, graphics::FilterNearest, graphics::FilterNearest, false, graphics::AddressClampToEdge));
			break;
		}

		return idx;
	}

	int sRendererPrivate::find_element_res(void* v) const
	{
		for (auto i = 0; i < element_reses.size(); i++)
		{
			if (element_reses[i].v == v)
				return i;
		}
		return -1;
	}

	int sRendererPrivate::set_material_res(int idx, graphics::Material* mesh)
	{

		return idx;
	}

	int sRendererPrivate::find_material_res(graphics::Material* mesh) const
	{
		return -1;
	}

	int sRendererPrivate::set_mesh_res(int idx, graphics::Mesh* mesh)
	{
		if (idx == -1)
		{
			for (auto i = 0; i < mesh_reses.size(); i++)
			{
				if (!mesh_reses[i].mesh)
				{
					idx = i;
					break;
				}
			}
		}
		if (idx == -1)
			return -1;

		auto& dst = mesh_reses[idx];
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
				dst.vtx_off = buf_mesh_vtx.n1;
				auto pvtx = buf_mesh_vtx.alloc(dst.vtx_cnt);
				for (auto i = 0; i < dst.vtx_cnt; i++)
				{
					auto& vtx = pvtx[i];
					vtx.pos = apos[i];
					vtx.uv = auv ? auv[i] : vec2(0.f);
					vtx.normal = anormal ? anormal[i] : vec3(1.f, 0.f, 0.f);
				}

				dst.idx_off = buf_mesh_idx.n1;
				auto pidx = buf_mesh_idx.alloc(dst.idx_cnt);
				for (auto i = 0; i < dst.idx_cnt; i++)
					pidx[i] = dst.idx_off + aidx[i];

				buf_mesh_vtx.upload(cb.get());
				buf_mesh_idx.upload(cb.get());
			}
			else
			{
				auto n = buf_arm_mesh_idx.n1;
				auto pidx = buf_arm_mesh_idx.alloc(dst.idx_cnt);
				for (auto i = 0; i < dst.idx_cnt; i++)
					pidx[i] = n + aidx[i];
			}


		}

		return idx;
	}

	int sRendererPrivate::find_mesh_res(graphics::Mesh* mesh) const
	{
		for (auto i = 0; i < mesh_reses.size(); i++)
		{
			if (mesh_reses[i].mesh == mesh)
				return i;
		}
		return -1;
	}

	void sRendererPrivate::fill_rect(uint layer, cElementPrivate* element, const vec2& pos, const vec2& size, const cvec4& color)
	{
		auto& info = element_drawing_layers[layer].emplace_back();
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

	void sRendererPrivate::stroke_rect(uint layer, cElementPrivate* element, const vec2& pos, const vec2& size, float thickness, const cvec4& color)
	{
		auto& info = element_drawing_layers[layer].emplace_back();
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

	void sRendererPrivate::draw_text(uint layer, cElementPrivate* element, const vec2& pos, uint font_size, uint font_id, const wchar_t* text_beg, const wchar_t* text_end, const cvec4& color)
	{
		auto& res = element_reses[font_id];
		if (!res.type == ElementResFont)
			return;

		auto& info = element_drawing_layers[layer].emplace_back();
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

	void sRendererPrivate::record_element_drawing_commands(uint tar_idx, graphics::CommandBuffer* cb)
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
			c.d.b = Rect(vec2(0.f), tar_size);
		}

		Rect scissor;
		scissor.reset();
		for (auto i = 0; i < _countof(element_drawing_layers); i++)
		{
			for (auto& info : element_drawing_layers[i])
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
						auto pvtx = buf_element_vtx.stag(3);
						pvtx[0].set(info.points[0    ], vec2(0.5), info.color);
						pvtx[1].set(info.points[i + 1], vec2(0.5), info.color);
						pvtx[2].set(info.points[i + 2], vec2(0.5), info.color);

						auto pidx = buf_element_idx.stag(3);
						pidx[0] = c.d.a.vtx_cnt + 0;
						pidx[1] = c.d.a.vtx_cnt + 2;
						pidx[2] = c.d.a.vtx_cnt + 1;

						c.d.a.vtx_cnt += 3;
						c.d.a.idx_cnt += 3;
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

					auto vtx_cnt0 = c.d.a.vtx_cnt;
					for (auto i = 0; i < info.points.size() - 1; i++)
					{
						if (i == 0)
						{
							auto p0 = info.points[0];
							auto p1 = info.points[1];

							auto n0 = normals[0];
							auto n1 = normals[1];

							auto pvtx = buf_element_vtx.stag(4);
							pvtx[0].set(p0 + n0 * thickness, vec2(0.5), info.color);
							pvtx[1].set(p0 - n0 * thickness, vec2(0.5), info.color);
							pvtx[2].set(p1 + n1 * thickness, vec2(0.5), info.color);
							pvtx[3].set(p1 - n1 * thickness, vec2(0.5), info.color);

							auto pidx = buf_element_idx.stag(6);
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
							auto pidx = buf_element_idx.stag(6);
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

							auto pvtx = buf_element_vtx.stag(2);
							pvtx[0].set(p1 + n1 * thickness, vec2(0.5), info.color);
							pvtx[1].set(p1 - n1 * thickness, vec2(0.5), info.color);

							auto pidx = buf_element_idx.stag(6);
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

					auto atlas = (graphics::FontAtlas*)element_reses[info.res_id].v;
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

							auto pvtx = buf_element_vtx.stag(4);
							pvtx[0].set(pos + o * axes, uv0, info.color);
							pvtx[1].set(pos + o.x * axes[0] + (o.y - s.y) * axes[1], vec2(uv0.x, uv1.y), info.color);
							pvtx[2].set(pos + (o.x + s.x) * axes[0] + (o.y - s.y) * axes[1], uv1, info.color);
							pvtx[3].set(pos + (o.x + s.x) * axes[0] + o.y * axes[1], vec2(uv1.x, uv0.y), info.color);
							auto pidx = buf_element_idx.stag(6);
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

		buf_element_vtx.upload(cb);
		buf_element_idx.upload(cb);

		cb->set_viewport(Rect(vec2(0.f), tar_size));
		auto cv = vec4(1.f, 1.f, 1.f, 1.f);
		cb->begin_renderpass(nullptr, fb_tars[tar_idx].get(), &cv);
		cb->bind_pipeline(pl_element);
		cb->bind_vertex_buffer(buf_element_vtx.buf.get(), 0);
		cb->bind_index_buffer(buf_element_idx.buf.get(), graphics::IndiceTypeUint);
		cb->bind_descriptor_set(PLL_element_aec9::Binding_element, ds_element.get());
		cb->push_constant_t(0, PLL_element_aec9::PushConstant{ 2.f / tar_size });
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

	graphics::Pipeline* sRendererPrivate::get_material_pipeline(MaterialUsage usage, const std::filesystem::path& mat, const std::string& _defines)
	{
		auto defines = graphics::Shader::format_defines(_defines);

		for (auto& p : pl_mats[usage])
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

		std::string defines_str;
		for (auto& t : defines)
		{
			defines_str += t;
			defines_str += ' ';
		}
		std::string substitutes_str;
		for (auto& t : substitutes)
		{
			defines_str += t.first;
			defines_str += ' ';
			defines_str += t.second;
			defines_str += ' ';
		}

		switch (usage)
		{
		//case MaterialForMeshShadow:
		//	defines.push_back("SHADOW_PASS");
		case MaterialForMesh:
		{
			graphics::Shader* shaders[] = {
				graphics::Shader::get(device, L"mesh/defe_geom.vert", defines_str.c_str(), substitutes_str.c_str()),
				graphics::Shader::get(device, L"mesh/defe_geom.frag", defines_str.c_str(), substitutes_str.c_str())
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
			ret = graphics::Pipeline::create(device, _countof(shaders), shaders,
				graphics::PipelineLayout::get(device, L"mesh/defe_geom.pll"), info);
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
		pl_mats[usage].push_back(std::move(mp));
		return ret;
	}

	void sRendererPrivate::release_material_pipeline(MaterialUsage usage, graphics::Pipeline* pl)
	{

	}

	void sRendererPrivate::draw_mesh(cNodePrivate* node, uint mesh_id)
	{
		fassert(transform_idx < _countof(DSL_transform_9c0d::Transforms::transforms));

		node->update_transform();

		auto& data = (*buf_transform.pstag).transforms[transform_idx];
		data.mat = node->transform;
		data.nor = mat4(node->rot);

		node_drawing_meshes[0].emplace_back(transform_idx, mesh_id); // TODO: 0 -> mat id

		transform_idx++;
	}

	void sRendererPrivate::record_node_drawing_commands(uint tar_idx, graphics::CommandBuffer* cb)
	{
		if (!camera)
			return;

		{
			auto node = camera->node;
			node->update_transform();
			auto view = mat4(node->rot);
			view[3] = vec4(node->g_pos, 1.f);
			view = inverse(view);
			auto proj = perspective(radians(camera->fovy), tar_size.x / tar_size.y, camera->near, camera->far);
			proj[1][1] *= -1.f;
			auto& data = *(buf_render_data.pstag);
			data.proj_view = proj * view;

			graphics::BufferCopy cpy;
			cpy.size = sizeof(data);
			buf_render_data.cpies.push_back(cpy);
		}
		buf_render_data.upload(cb);

		{
			graphics::BufferCopy cpy;
			cpy.size = sizeof(DSL_transform_9c0d::Transform) * transform_idx;
			buf_transform.cpies.push_back(cpy);
			transform_idx = 0;
		}
		buf_transform.upload(cb);

		buf_indirs.stag_num = 0;
		for (auto mat_id = 0; mat_id < node_drawing_meshes.size(); mat_id++)
		{
			auto& vec = node_drawing_meshes[mat_id];
			if (!vec.empty())
			{
				auto indirs = buf_indirs.stag(vec.size());
				for (auto i = 0; i < vec.size(); i++)
				{
					auto& src = mesh_reses[vec[i].second];
					auto& dst = indirs[i];
					dst.vertex_offset = src.vtx_off;
					dst.first_index = src.idx_off;
					dst.index_count = src.idx_cnt;
					dst.first_instance = (vec[i].first << 16) + mat_id;
					dst.instance_count = 1;
				}
			}
		}
		buf_indirs.upload(cb);

		auto vp = Rect(vec2(0.f), tar_size);
		cb->set_viewport(vp);
		cb->set_scissor(vp);
		vec4 cvs[] = { 
			vec4(0.f, 0.f, 0.f, 0.f),
			vec4(0.f, 0.f, 0.f, 0.f),
			vec4(1.f, 0.f, 0.f, 0.f),
			vec4(0.f, 0.f, 0.f, 0.f)
		};
		cb->begin_renderpass(nullptr, fb_def.get(), cvs);
		cb->bind_pipeline(pl_mats[MaterialForMesh][0].pipeline.get());
		cb->bind_vertex_buffer(buf_mesh_vtx.buf.get(), 0);
		cb->bind_index_buffer(buf_mesh_idx.buf.get(), graphics::IndiceTypeUint);
		graphics::DescriptorSet* sets[PLL_defe_geom_25c1::Binding_Max];
		sets[PLL_defe_geom_25c1::Binding_render_data] = ds_render_data.get();
		sets[PLL_defe_geom_25c1::Binding_transform] = ds_transform.get();
		cb->bind_descriptor_sets(0, _countof(sets), sets);
		auto indir_off = 0;
		for (auto mat_id = 0; mat_id < node_drawing_meshes.size(); mat_id++)
		{
			auto& vec = node_drawing_meshes[mat_id];
			if (!vec.empty())
			{
				cb->draw_indexed_indirect(buf_indirs.buf.get(), indir_off, vec.size());
				indir_off += vec.size();
			}
		}
		cb->next_pass();
		cb->bind_pipeline(pl_defe_shad);
		cb->bind_descriptor_set(0, ds_defe_shad.get());
		cb->draw(3, 1, 0, 0);
		cb->end_renderpass();

		cb->image_barrier(img_back.get(), {}, graphics::ImageLayoutShaderReadOnly, graphics::ImageLayoutAttachment, graphics::AccessColorAttachmentWrite);
		cb->begin_renderpass(rp_rgba8, fb_tars[tar_idx].get());
		cb->bind_pipeline(pl_gamma);
		cb->bind_descriptor_set(0, ds_back.get());
		cb->draw(3, 1, 0, 0);
		cb->end_renderpass();
	}

	void sRendererPrivate::record_drawing_commands(uint tar_idx, graphics::CommandBuffer* cb)
	{
		record_node_drawing_commands(tar_idx, cb);
		//record_element_drawing_commands(tar_idx, cb);
	}

	void sRendererPrivate::on_added()
	{
		window = (Window*)world->find_object("flame::Window");
		window->add_resize_listener([](Capture& c, const uvec2&) {
			c.thiz<sRendererPrivate>()->set_targets();
		}, Capture().set_thiz(this));
		swapchain;

		device = graphics::Device::get_default();
		swapchain = (graphics::Swapchain*)world->find_object("flame::graphics::Swapchain");

		rp_rgba8c = graphics::Renderpass::get(device, L"rgba8c.rp");
		rp_rgba8 = graphics::Renderpass::get(device, L"rgba8.rp");
		set_targets();

		pl_element = graphics::Pipeline::get(device, L"element/element.pl");

		auto dsp = graphics::DescriptorPool::get_default(device);

		buf_element_vtx.create(device, graphics::BufferUsageVertex, 360000);
		buf_element_idx.create(device, graphics::BufferUsageIndex, 240000);
		img_wht.reset(graphics::Image::create(device, graphics::Format_R8G8B8A8_UNORM, uvec2(1), 1, 1, 
			graphics::SampleCount_1, graphics::ImageUsageTransferDst | graphics::ImageUsageSampled));
		img_wht->clear(graphics::ImageLayoutUndefined, graphics::ImageLayoutShaderReadOnly, cvec4(255));
		{
			auto dsl = graphics::DescriptorSetLayout::get(device, L"element/element.dsl");
			ds_element.reset(graphics::DescriptorSet::create(dsp, dsl));
			auto idx = dsl->find_binding("images");
			graphics::DescriptorBindingInfo bd;
			dsl->get_binding(idx, &bd);
			element_reses.resize(bd.count);
			auto iv_wht = img_wht->get_view(0);
			auto sp = graphics::Sampler::get(device, graphics::FilterLinear, graphics::FilterLinear, false, graphics::AddressClampToEdge);
			for (auto i = 0; i < bd.count; i++)
			{
				ds_element->set_image(idx, i, iv_wht, sp);

				auto& res = element_reses[i];
				res.type = ElementResImage;
				res.v = iv_wht;
			}
		}

		mat_reses.resize(128);
		mesh_reses.resize(64);
		node_drawing_meshes.resize(mat_reses.size());
		
		buf_indirs.create(device, graphics::BufferUsageIndirect, 65536);

		buf_mesh_vtx.create(device, graphics::BufferUsageVertex, 10000);
		buf_mesh_idx.create(device, graphics::BufferUsageIndex, 10000);
		buf_arm_mesh_vtx.create(device, graphics::BufferUsageVertex, 10000);
		buf_arm_mesh_idx.create(device, graphics::BufferUsageIndex, 10000);

		buf_render_data.create(device, graphics::BufferUsageUniform);
		{
			auto dsl = graphics::DescriptorSetLayout::get(device, L"render_data.dsl");
			ds_render_data.reset(graphics::DescriptorSet::create(dsp, dsl));
			ds_render_data->set_buffer(dsl->find_binding("RenderData"), 0, buf_render_data.buf.get());
		}

		buf_transform.create(device, graphics::BufferUsageStorage);
		{
			auto dsl = graphics::DescriptorSetLayout::get(device, L"transform.dsl");
			ds_transform.reset(graphics::DescriptorSet::create(dsp, dsl));
			ds_transform->set_buffer(dsl->find_binding("Transforms"), 0, buf_transform.buf.get());
		}

		buf_material.create(device, graphics::BufferUsageStorage);
		{
			auto dsl = graphics::DescriptorSetLayout::get(device, L"material.dsl");
			ds_material.reset(graphics::DescriptorSet::create(dsp, dsl));
			ds_material->set_buffer(dsl->find_binding("MaterialInfos"), 0, buf_material.buf.get());
		}

		img_back.reset(graphics::Image::create(device, graphics::Format_R16G16B16A16_SFLOAT, tar_size, 1, 1,
			graphics::SampleCount_1, graphics::ImageUsageSampled | graphics::ImageUsageAttachment));
		img_dep.reset(graphics::Image::create(device, graphics::Format_Depth16, tar_size, 1, 1, 
			graphics::SampleCount_1, graphics::ImageUsageSampled | graphics::ImageUsageAttachment));
		img_def_geo0.reset(graphics::Image::create(device, graphics::Format_R8G8B8A8_UNORM, tar_size, 1, 1,
			graphics::SampleCount_1, graphics::ImageUsageSampled | graphics::ImageUsageAttachment));
		img_def_geo1.reset(graphics::Image::create(device, graphics::Format_R8G8B8A8_UNORM, tar_size, 1, 1,
			graphics::SampleCount_1, graphics::ImageUsageSampled | graphics::ImageUsageAttachment));

		{
			graphics::ImageView* vs[4];
			vs[0] = img_def_geo0->get_view();
			vs[1] = img_def_geo0->get_view();
			vs[2] = img_dep->get_view();
			vs[3] = img_back->get_view();
			fb_def.reset(graphics::Framebuffer::create(device,
				graphics::Renderpass::get(device, L"deferred.rp"), _countof(vs), vs));
		}

		get_material_pipeline(MaterialForMesh, L"", "");

		pl_defe_shad = graphics::Pipeline::get(device, L"deferred/shade.pl");

		{
			auto dsl = graphics::DescriptorSetLayout::get(device, L"deferred/shade.dsl");
			ds_defe_shad.reset(graphics::DescriptorSet::create(dsp, dsl));
			auto sp = graphics::Sampler::get(device, graphics::FilterNearest, graphics::FilterNearest, false, graphics::AddressClampToEdge);
			ds_defe_shad->set_image(dsl->find_binding("image0"), 0, img_def_geo0->get_view(), sp);
			ds_defe_shad->set_image(dsl->find_binding("image1"), 0, img_def_geo1->get_view(), sp);
		}

		{
			auto dsl = graphics::DescriptorSetLayout::get(device, L"post/post.dsl");
			ds_back.reset(graphics::DescriptorSet::create(dsp, dsl));
			auto sp = graphics::Sampler::get(device, graphics::FilterNearest, graphics::FilterNearest, false, graphics::AddressClampToEdge);
			ds_back->set_image(dsl->find_binding("image"), 0, img_back->get_view(), sp);
		}
		pl_gamma = graphics::Pipeline::get(device, L"post/gamma.pl");
	}

	void sRendererPrivate::update()
	{
		if (!dirty && !always_update)
			return;

		element_drawing_scissor = Rect(vec2(0.f), tar_size);
		for (auto i = 0; i < _countof(element_drawing_layers); i++)
			element_drawing_layers[i].clear();
		buf_element_vtx.stag_num = 0;
		buf_element_idx.stag_num = 0;
		element_render(0, world->element_root->get_component_t<cElementPrivate>());

		for (auto i = 0; i < node_drawing_meshes.size(); i++)
			node_drawing_meshes[i].clear();
		node_render(world->node_root->get_component_t<cNodePrivate>());

		dirty = false;
	}

	sRenderer* sRenderer::create(void* parms)
	{
		return f_new<sRendererPrivate>((sRendererParms*)parms);
	}
}
