#include <flame/graphics/device.h>
#include <flame/graphics/buffer.h>
#include <flame/graphics/image.h>
#include <flame/graphics/renderpass.h>
#include <flame/graphics/shader.h>
#include <flame/graphics/command.h>
#include <flame/graphics/swapchain.h>
#include <flame/graphics/canvas.h>
#include "../world_private.h"
#include "../components/element_private.h"
#include "../components/node_private.h"
#include "../components/camera_private.h"
#include "renderer_private.h"

namespace flame
{
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
		buf.reset(graphics::Buffer::create(d, size, graphics::BufferUsageTransferDst | usage, graphics::MemoryPropertyDevice));
		stagbuf.reset(graphics::Buffer::create(d, size, graphics::BufferUsageTransferSrc, graphics::MemoryPropertyHost | graphics::MemoryPropertyCoherent));
		stagbuf->map();
		pstag = (T*)stagbuf->get_mapped();
		if (temp)
		{
			push(n, temp);
			delete[]temp;
		}
	}

	template <class T>
	void GeometryBuffer<T>::create(graphics::Device* _d, graphics::BufferUsageFlags _usage, uint _capacity, graphics::AccessFlags _access)
	{
		d = _d;
		usage = _usage;
		capacity = _capacity;
		access = _access;
		rebuild();
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

		hdr = parms.hdr;
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

	static void element_render(ElementRenderStatus& status, uint layer, cElementPrivate* element)
	{
		auto e = element->entity;
		if (!e->global_visibility)
			return;

		element->parent_scissor = status.scissor;
		element->update_transform();
		auto culled = !status.scissor.overlapping(element->aabb);
		if (element->culled != culled)
		{
			element->culled = culled;
			e->component_data_changed(element, S<"culled"_h>);
		}
		if (culled)
			return;

		element->draw2(layer, status.renderer);
		if (element->clipping)
		{
			if (!(status.scissor == element->aabb))
			{
				status.scissor = element->aabb;
				auto& info = status.renderer->draw_layers[layer].emplace_back();
				info.type = ElementDrawCmd::Scissor;
				info.scissor = status.scissor;
			}
		}
		for (auto& d : element->drawers2)
			d->call(layer, status.renderer);

		layer++;
		auto first_child = true;
		for (auto& c : e->children)
		{
			auto element = c->get_component_t<cElementPrivate>();
			if (element)
			{
				if (element->new_layer && !first_child)
					layer++;
				element_render(status, layer, element);
			}
			first_child = false;
		}
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

	void sRendererPrivate::fill_rect(uint layer, cElementPrivate* element, const vec2& pos, const vec2& size, const cvec4& color)
	{
		auto& info = draw_layers[layer].emplace_back();
		info.type = ElementDrawCmd::Fill;

		info.res_id = 0;
		info.points.resize(4);
		auto a = pos - element->pivot * size;
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
						pidx[1] = c.d.a.vtx_cnt + 1;
						pidx[2] = c.d.a.vtx_cnt + 2;

						c.d.a.vtx_cnt += 3;
						c.d.a.idx_cnt += 3;
					}
				}
					break;
				case ElementDrawCmd::Scissor:
					if (!cmds.back().b)
						cmds.emplace_back().b = true;
					break;
				}
			}
		}

		buf_element_vtx.upload(cb);
		buf_element_idx.upload(cb);

		cb->set_viewport(Rect(vec2(0.f), tar_size));
		auto cv = vec4(0.f, 0.f, 0.f, 1.f);
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
				cb->draw_indexed(c.d.a.idx_cnt, vtx_off, idx_off, 1, c.d.a.res_id);
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
		buf_element_idx.create(device, graphics::BufferUsageIndex, 240000, graphics::AccessIndexRead);
		img_wht.reset(graphics::Image::create(device, graphics::Format_R8G8B8A8_UNORM, uvec2(1), 1, 1, 
			graphics::SampleCount_1, graphics::ImageUsageTransferDst | graphics::ImageUsageSampled));
		img_wht->clear(graphics::ImageLayoutUndefined, graphics::ImageLayoutShaderReadOnly, cvec4(255));
		{
			auto dsl = graphics::DescriptorSetLayout::get(device, L"element/element.dsl");
			ds_element.reset(graphics::DescriptorSet::create(dsp, dsl));
			auto bd = dsl->find_binding("images");
			auto num = dsl->get_binding(bd)->get_count();
			auto iv_wht = img_wht->get_view(0);
			for (auto i = 0; i < num; i++)
			{
				ds_element->set_image(bd, i, iv_wht,
					graphics::Sampler::get(device, graphics::FilterLinear, graphics::FilterLinear, 
					false, graphics::AddressClampToEdge));
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
