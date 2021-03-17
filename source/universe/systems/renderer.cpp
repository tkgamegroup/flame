#include <flame/graphics/device.h>
#include <flame/graphics/buffer.h>
#include <flame/graphics/image.h>
#include <flame/graphics/renderpass.h>
#include <flame/graphics/shader.h>
#include <flame/graphics/command.h>
#include <flame/graphics/swapchain.h>
#include <flame/graphics/font.h>
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
		}
		return graphics::AccessNone;
	}

	template <class T>
	void GeometryBuffer<T>::rebuild()
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
	void GeometryBuffer<T>::create(graphics::Device* d, graphics::BufferUsageFlags usage, uint _capacity)
	{
		capacity = _capacity;
		access = usage2access(usage);
		auto size = capacity * sizeof(T);
		buf.reset(graphics::Buffer::create(d, size, graphics::BufferUsageTransferDst | usage, graphics::MemoryPropertyDevice));
		stagbuf.reset(graphics::Buffer::create(d, size, graphics::BufferUsageTransferSrc, graphics::MemoryPropertyHost | graphics::MemoryPropertyCoherent));
		stagbuf->map();
		pstag = (T*)stagbuf->get_mapped();
	}

	template <class T>
	void GeometryBuffer<T>::push(uint cnt, const T* p)
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
	T* GeometryBuffer<T>::stag(uint cnt)
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
	void GeometryBuffer<T>::upload(graphics::CommandBuffer* cb)
	{
		graphics::BufferCopy cpy;
		cpy.size = stag_num * sizeof(T);
		cb->copy_buffer(stagbuf.get(), buf.get(), 1, &cpy);
		cb->buffer_barrier(buf.get(), graphics::AccessTransferWrite, access);
	}

	sRendererPrivate::sRendererPrivate(sRendererParms* _parms)
	{
		sRendererParms parms;
		if (_parms)
			parms = *_parms;
	}

	void sRendererPrivate::set_targets()
	{
		fb_targets.clear();
		auto count = swapchain->get_images_count();
		fb_targets.resize(count);
		for (auto i = 0; i < count; i++)
		{
			auto v = swapchain->get_image(i)->get_view();
			fb_targets[i].reset(graphics::Framebuffer::create(device, rp_rgba8c.get(), 1, &v));
		}
		tar_size = swapchain->get_image(0)->get_size();
	}

	struct ElementRenderStatus
	{
		sRendererPrivate* renderer;
		Rect scissor;
	};

	static uint element_render(ElementRenderStatus& status, uint layer, cElementPrivate* element)
	{
		auto e = element->entity;
		if (!e->global_visibility)
			return layer;

		element->parent_scissor = status.scissor;
		element->update_transform();
		auto culled = !status.scissor.overlapping(element->aabb);
		if (element->culled != culled)
		{
			element->culled = culled;
			e->component_data_changed(element, S<"culled"_h>);
		}
		if (culled)
			return layer;

		element->draw2(layer, status.renderer);

		auto clipping = false;
		Rect last_scissor;
		if (element->clipping && !(status.scissor == element->aabb))
		{
			element->layer_policy = 2;

			clipping = true;
			last_scissor = status.scissor;
			status.scissor = element->aabb;
			auto& info = status.renderer->draw_layers[layer].emplace_back();
			info.type = ElementDrawCmd::Scissor;
			info.misc = status.scissor;
		}

		auto _layer = layer;
		for (auto& d : element->drawers2)
			_layer = max(_layer, d->call(layer, status.renderer));

		_layer++;
		auto max_layer = _layer;
		for (auto& c : e->children)
		{
			auto celement = c->get_component_t<cElementPrivate>();
			if (celement)
			{
				max_layer = max(max_layer, element_render(status, _layer, celement));
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
			status.scissor = last_scissor;
			auto& info = status.renderer->draw_layers[max_layer + 1].emplace_back();
			info.type = ElementDrawCmd::Scissor;
			info.misc = last_scissor;
		}

		return max_layer;
	}

	void sRendererPrivate::render(EntityPrivate* e, bool element_culled, bool node_culled)
	{
		if (!e->global_visibility)
			return;

		auto last_scissor = canvas->get_scissor();

		if (!element_culled)
		{
			auto element = e->get_component_t<cElementPrivate>();
			if (element)
			{
				element->parent_scissor = last_scissor;
				element->update_transform();
				element_culled = !last_scissor.overlapping(element->aabb);
				if (element->culled != element_culled)
				{
					element->culled = element_culled;
					e->component_data_changed(element, S<"culled"_h>);
				}
				if (!element_culled)
				{
					element->draw(canvas);
					for (auto& d : element->drawers[0])
						d->call(canvas);
					if (element->clipping)
						canvas->set_scissor(element->aabb);
					for (auto& d : element->drawers[1])
						d->call(canvas);
				}
				if (last_element != element)
				{
					last_element = element;
					last_element_changed = true;
				}
			}
		}
		if (!node_culled)
		{
			auto node = e->get_component_t<cNodePrivate>();
			if (node)
			{
				node->update_transform();
				if (last_element_changed)
					canvas->set_viewport(last_element->aabb);
				for (auto& d : node->drawers)
					d->call(canvas);
			}
		}

		for (auto& c : e->children)
			render(c.get(), element_culled, node_culled);

		canvas->set_scissor(last_scissor);
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
		return idx;
	}

	int sRendererPrivate::find_mesh_res(graphics::Mesh* mesh) const
	{
		return -1;
	}

	void sRendererPrivate::fill_rect(uint layer, cElementPrivate* element, const vec2& pos, const vec2& size, const cvec4& color)
	{
		auto& info = draw_layers[layer].emplace_back();
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
		auto& info = draw_layers[layer].emplace_back();
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

		auto& info = draw_layers[layer].emplace_back();
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
		for (auto i = 0; i < size(draw_layers); i++)
		{
			for (auto& info : draw_layers[i])
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
		cb->begin_renderpass(nullptr, fb_targets[tar_idx].get(), &cv);
		cb->bind_pipeline(pl_element.get());
		cb->bind_vertex_buffer(buf_element_vtx.buf.get(), 0);
		cb->bind_index_buffer(buf_element_idx.buf.get(), graphics::IndiceTypeUint);
		cb->bind_descriptor_set(S<"element"_h>, ds_element.get());
		cb->push_constant_ht(S<"scale"_h>, 2.f / tar_size);
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

	void sRendererPrivate::record_node_drawing_commands(uint tar_idx, graphics::CommandBuffer* cb)
	{

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

		{
			graphics::RenderpassAttachmentInfo att;
			att.format = graphics::Format_R8G8B8A8_UNORM;
			att.load_op = graphics::AttachmentClear;
			att.initia_layout = graphics::ImageLayoutShaderReadOnly;
			graphics::RenderpassSubpassInfo sp;
			uint col_refs[] = {
				0
			};
			sp.color_attachments_count = 1;
			sp.color_attachments = col_refs;
			rp_rgba8c.reset(graphics::Renderpass::create(device, 1, &att, 1, &sp));
			att.format = graphics::Format_R16G16B16A16_SFLOAT;
			rp_rgba16c.reset(graphics::Renderpass::create(device, 1, &att, 1, &sp));
		}
		{
			graphics::RenderpassAttachmentInfo atts[2];
			atts[0].format = graphics::Format_R16G16B16A16_SFLOAT;
			atts[0].load_op = graphics::AttachmentClear;
			atts[0].initia_layout = graphics::ImageLayoutShaderReadOnly;
			atts[1].format = graphics::Format_Depth16;
			atts[1].load_op = graphics::AttachmentClear;
			atts[1].initia_layout = graphics::ImageLayoutAttachment;
			atts[1].final_layout = graphics::ImageLayoutAttachment;
			graphics::RenderpassSubpassInfo sp;
			uint col_refs[] = {
				0
			};
			sp.color_attachments_count = 1;
			sp.color_attachments = col_refs;
			sp.depth_attachment = 1;
			rp_mesh.reset(graphics::Renderpass::create(device, 2, atts, 1, &sp));
		}
		{
			graphics::RenderpassAttachmentInfo atts[2];
			atts[0].format = graphics::Format_R16_SFLOAT;
			atts[0].load_op = graphics::AttachmentClear;
			atts[0].initia_layout = graphics::ImageLayoutShaderReadOnly;
			atts[1].format = graphics::Format_Depth16;
			atts[1].load_op = graphics::AttachmentClear;
			atts[1].initia_layout = graphics::ImageLayoutAttachment;
			atts[1].final_layout = graphics::ImageLayoutAttachment;
			graphics::RenderpassSubpassInfo sp;
			uint col_refs[] = {
				0
			};
			sp.color_attachments_count = 1;
			sp.color_attachments = col_refs;
			sp.depth_attachment = 1;
			rp_esm.reset(graphics::Renderpass::create(device, 2, atts, 1, &sp));
		}

		set_targets(); 

		{
			graphics::Shader* shaders[] = {
				graphics::Shader::get(device, L"element/element.vert", "", ""),
				graphics::Shader::get(device, L"element/element.frag", "", "")
			};
			graphics::VertexAttributeInfo vias[3];
			vias[0].location = 0;
			vias[0].format = graphics::Format_R32G32_SFLOAT;
			vias[1].location = 1;
			vias[1].format = graphics::Format_R32G32_SFLOAT;
			vias[2].location = 2;
			vias[2].format = graphics::Format_R8G8B8A8_UNORM;
			graphics::VertexBufferInfo vib;
			vib.attributes_count = size(vias);
			vib.attributes = vias;
			graphics::VertexInfo vi;
			vi.buffers_count = 1;
			vi.buffers = &vib;
			graphics::BlendOption bo;
			bo.enable = true;
			bo.src_color = graphics::BlendFactorSrcAlpha;
			bo.dst_color = graphics::BlendFactorOneMinusSrcAlpha;
			bo.src_alpha = graphics::BlendFactorOne;
			bo.dst_alpha = graphics::BlendFactorZero;
			pl_element.reset(graphics::Pipeline::create(device, size(shaders), shaders, 
				graphics::PipelineLayout::get(device, L"element/element.pll"), rp_rgba8c.get(), 0, &vi, nullptr, nullptr, 1, &bo));
		}

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
			for (auto i = 0; i < bd.count; i++)
			{
				ds_element->set_image(idx, i, iv_wht,
					graphics::Sampler::get(device, graphics::FilterLinear, graphics::FilterLinear, 
					false, graphics::AddressClampToEdge));

				auto& res = element_reses[i];
				res.type = ElementResImage;
				res.v = iv_wht;
			}
		}

		canvas = (graphics::Canvas*)world->find_object("flame::graphics::Canvas");
	}

	void sRendererPrivate::update()
	{
		if (!dirty && !always_update)
			return;

		last_element = nullptr;
		last_element_changed = false;
		for (auto i = 0; i < size(draw_layers); i++)
			draw_layers[i].clear();
		buf_element_vtx.stag_num = 0;
		buf_element_idx.stag_num = 0;

		ElementRenderStatus status;
		status.renderer = this;
		status.scissor = Rect(vec2(0.f), tar_size);
		element_render(status, 0, world->root->get_component_t<cElementPrivate>());
		//render(world->root.get(), false, !camera);

		dirty = false;
	}

	sRenderer* sRenderer::create(void* parms)
	{
		return f_new<sRendererPrivate>((sRendererParms*)parms);
	}
}
