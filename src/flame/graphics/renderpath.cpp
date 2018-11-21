// MIT License
// 
// Copyright (c) 2018 wjs
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "device.h"
#include "buffer.h"
#include "image.h"
#include "renderpass.h"
#include "framebuffer.h"
#include "shader.h"
#include "pipeline.h"
#include "descriptor.h"
#include "renderpath.h"

#include <flame/file.h>
#include <flame/blueprint.h>
#include <flame/typeinfo.h>

#include <tuple>

namespace flame
{
	namespace graphics
	{
		struct RenderpathPrivate : Renderpath
		{
			blueprint::Scene *s;

			inline RenderpathPrivate(Device *d, const wchar_t *filename)
			{
		//		s = blueprint::Scene::create();
		//		s->load(filename);

		//		for (auto i = 0; i < s->node_count(); i++)
		//		{
		//			auto n = s->node(i);
		//			if (n->name_hash == cH("Shader"))
		//			{
		//				MediumString filename;
		//				MediumString defines;
		//				n->inputslot(1)->get_str(&filename);
		//				n->inputslot(2)->get_str(&defines);

		//				auto s = Shader::get(d,
		//					s2w(filename.data).c_str(),
		//					defines.data);

		//				n->outputslot(0)->p_val = s;
		//			}
		//		}

		//		for (auto i = 0; i < s->node_count(); i++)
		//		{
		//			auto n = s->node(i);
		//			if (n->name_hash == cH("Shader Resource"))
		//			{
		//				MediumString resource_name;
		//				n->inputslot(2)->get_str(&resource_name);
		//				auto sr = ((Shader*)n->inputslot(1)->get_link_node()->outputslot(0)->p_val)->get_resource(
		//					resource_name.data);
		//				n->outputslot(0)->i_val[0] = sr->set;
		//				n->outputslot(1)->i_val[0] = sr->binding;
		//				n->outputslot(2)->i_val[0] = sr->var_type.offset;
		//				n->outputslot(3)->i_val[0] = sr->var_type.size;
		//				n->outputslot(4)->i_val[0] = sr->var_type.count;
		//			}
		//		}

		//		for (auto i = 0; i < s->node_count(); i++)
		//		{
		//			auto n = s->node(i);
		//			if (n->name_hash == cH("Buffer"))
		//			{
		//				auto b = Buffer::create(d,
		//					n->inputslot(1)->get_int(),
		//					n->inputslot(2)->get_int(),
		//					n->inputslot(3)->get_int(),
		//					n->inputslot(4)->get_bool());

		//				n->outputslot(0)->p_val = b;
		//			}
		//		}

		//		for (auto i = 0; i < s->node_count(); i++)
		//		{
		//			auto n = s->node(i);
		//			if (n->name_hash == cH("Image"))
		//			{
		//				auto t = Image::create(d,
		//					(Format)n->inputslot(1)->get_int(),
		//					n->inputslot(2)->get_int2(),
		//					n->inputslot(3)->get_int(),
		//					n->inputslot(4)->get_int(),
		//					(SampleCount)n->inputslot(5)->get_int(),
		//					n->inputslot(6)->get_int(),
		//					n->inputslot(8)->get_int());

		//				n->outputslot(0)->p_val = t;
		//			}
		//		}

		//		for (auto i = 0; i < s->node_count(); i++)
		//		{
		//			auto n = s->node(i);
		//			if (n->name_hash == cH("Render Pass"))
		//			{
		//				struct AttachmentDesc
		//				{
		//					blueprint::Node *n;
		//					Image *img;
		//					bool clear;
		//				};

		//				struct SubpassDesc
		//				{
		//					std::vector<int> color_atts;
		//					std::vector<int> resolve_atts;
		//					int depth_att;
		//				};

		//				std::vector<AttachmentDesc> atts;
		//				std::vector<SubpassDesc> sps;

		//				auto sps_node = n->inputslot(1)->get_link_node();
		//				sps.resize(sps_node->inputslot_count());
		//				for (auto j = 0; j < sps_node->inputslot_count(); i++)
		//				{
		//					auto sp_node = sps_node->inputslot(j)->get_link_node();

		//					auto col_atts_node = sp_node->inputslot(0)->get_link_node();
		//					for (auto k = 0; k < col_atts_node->inputslot_count(); k++)
		//					{

		//					}
		//				}
		//				//sps.resize(n->input_slots[1].int_val);

		//				//auto add_att = [&](int sl_idx, int arr_idx) {
		//				//	if (n->input_slots[sl_idx].get_split(arr_idx)->get_link_count() != 1)
		//				//		return -1;
		//				//	auto l = n->input_slots[sl_idx].get_split(arr_idx)->get_link(0);
		//				//	auto in = l->out_node;

		//				//	for (auto i = 0; i < atts.size(); i++)
		//				//	{
		//				//		if (std::get<0>(atts[i]) == in)
		//				//			return i;
		//				//	}

		//				//	auto img = r->get_image_by_node(in);
		//				//	if (!img)
		//				//		return -1;

		//				//	auto clear = n->input_slots[sl_idx + 1].get_split(arr_idx)->bool_val;
		//				//	atts.emplace_back(in, img, clear);

		//				//	return int(atts.size() - 1);
		//				//};

		//				//for (auto j = 0; j < sps.size(); j++)
		//				//{
		//				//	for (auto k = 0; k < 4; k++)
		//				//	{
		//				//		auto idx = add_att(2 + k * 2, j);
		//				//		if (idx == -1)
		//				//			break;
		//				//		sps[j].col_atts.push_back(idx);
		//				//	}
		//				//	auto idx = add_att(10, j);
		//				//	sps[j].dep_att = idx;
		//				//}

		//				//std::vector<RenderpassAttachmentInfo> att_infos;
		//				//att_infos.resize(atts.size());
		//				//for (auto i = 0; i < atts.size(); i++)
		//				//{
		//				//	att_infos[i].format = std::get<1>(atts[i])->format;
		//				//	att_infos[i].clear = std::get<2>(atts[i]);
		//				//}
		//				//std::vector<RenderpassSubpassInfo> sp_infos;
		//				//sp_infos.resize(sps.size());
		//				//for (auto i = 0; i < sps.size(); i++)
		//				//{
		//				//	sp_infos[i].color_attachments = sps[i].col_atts.data();
		//				//	sp_infos[i].color_attachment_count = sps[i].col_atts.size();
		//				//	sp_infos[i].depth_attachment = sps[i].dep_att;
		//				//}

		//				//RenderpassInfo rp_info;
		//				//rp_info.attachment_count = att_infos.size();
		//				//rp_info.attachments = att_infos.data();
		//				//rp_info.subpass_count = sp_infos.size();
		//				//rp_info.subpasses = sp_infos.data();
		//				//auto rp = get_renderpass(d, &rp_info);

		//				//std::vector<Imageview*> views;
		//				//views.resize(atts.size());
		//				//for (auto i = 0; i < atts.size(); i++)
		//				//	views[i] = std::get<1>(atts[i])->get_view();
		//				//auto fb = get_framebuffer(d, std::get<1>(atts[0])->size, rp, views.size(), views.data());

		//				//r->_priv->rps.emplace_back(n, rp);
		//				//r->_priv->fbs.emplace_back(n, fb);
		//			}
		//		}

		//		//for (auto i = 0; i < n_cnt; i++)
		//		//{
		//		//	auto n = s->get_node(i);
		//		//	if (n->name_hash == cH("Pipeline"))
		//		//	{
		//		//		auto p = create_pipeline(d);

		//		//		{
		//		//			auto in = n->input_slots[1].get_link(0)->out_node;
		//		//			std::vector<VertexInputBindingInfo> vib;
		//		//			vib.resize(in->input_slots[0].int_val);
		//		//			for (auto j = 0; j < vib.size(); j++)
		//		//			{
		//		//				vib[j].binding = in->input_slots[1].get_split(j)->int_val;
		//		//				vib[j].stride = in->input_slots[2].get_split(j)->int_val;
		//		//				vib[j].rate = (VertexInputRate)get_enum_item_value(cH("graphics::VertexInputRate"),
		//		//					in->input_slots[3].get_split(j)->int_val);
		//		//			}
		//		//			p->set_vertex_input_bindings(vib.size(), vib.data());
		//		//		}

		//		//		{
		//		//			auto in = n->input_slots[2].get_link(0)->out_node;
		//		//			std::vector<VertexInputAttribInfo> via;
		//		//			via.resize(in->input_slots[0].int_val);
		//		//			for (auto j = 0; j < via.size(); j++)
		//		//			{
		//		//				via[j].location = in->input_slots[1].get_split(j)->int_val;
		//		//				via[j].binding = in->input_slots[2].get_split(j)->int_val;
		//		//				via[j].offset = in->input_slots[3].get_split(j)->int_val;
		//		//				via[j].format = (Format)get_enum_item_value(cH("graphics::Format"),
		//		//					in->input_slots[4].get_split(j)->int_val);
		//		//			}
		//		//			p->set_vertex_input_attribs(via.size(), via.data());
		//		//		}

		//		//		p->set_patch_control_points(n->input_slots[3].int_val);
		//		//		p->set_primitive_topology((PrimitiveTopology)get_enum_item_value(cH("graphics::PrimitiveTopology"),
		//		//			n->input_slots[4].int_val));
		//		//		p->set_polygon_mode((PolygonMode)get_enum_item_value(cH("graphics::PolygonMode"),
		//		//			n->input_slots[5].int_val));
		//		//		p->set_depth_test(n->input_slots[6].bool_val);
		//		//		p->set_depth_compare_op((CompareOp)get_enum_item_value(cH("graphics::CompareOp"),
		//		//			n->input_slots[7].int_val));
		//		//		p->set_depth_write(n->input_slots[8].bool_val);
		//		//		p->set_depth_clamp(n->input_slots[9].bool_val);
		//		//		p->set_cull_mode((CullMode)get_enum_item_value(cH("graphics::CullMode"),
		//		//			n->input_slots[10].int_val));

		//		//		{
		//		//			auto l = n->input_slots[11].get_link(0);
		//		//			auto in = l->out_node;
		//		//			p->set_renderpass(r->get_renderpass_by_node(in), l->out_idx - 1);
		//		//			p->set_size(r->get_framebuffer_by_node(in)->size);
		//		//		}

		//		//		auto out_cnt = n->input_slots[12].int_val;
		//		//		p->set_output_attachment_count(out_cnt);
		//		//		for (auto j = 0; j < out_cnt; j++)
		//		//		{
		//		//			p->set_blend_state(j,
		//		//				n->input_slots[13].get_split(j)->bool_val,
		//		//				(BlendFactor)get_enum_item_value(cH("graphics::BlendFactor"), n->input_slots[14].int_val),
		//		//				(BlendFactor)get_enum_item_value(cH("graphics::BlendFactor"), n->input_slots[15].int_val),
		//		//				(BlendFactor)get_enum_item_value(cH("graphics::BlendFactor"), n->input_slots[16].int_val),
		//		//				(BlendFactor)get_enum_item_value(cH("graphics::BlendFactor"), n->input_slots[17].int_val)
		//		//			);
		//		//		}

		//		//		std::vector<DynamicState> dss;
		//		//		dss.resize(n->input_slots[19].int_val);
		//		//		for (auto j = 0; j < dss.size(); j++)
		//		//		{
		//		//			dss.push_back((DynamicState)get_enum_item_value(cH("graphics::DynamicState"),
		//		//				n->input_slots[20].int_val));
		//		//		}
		//		//		p->set_dynamic_state(dss.size(), dss.data());

		//		//		auto sd_cnt = n->input_slots[22].get_link_count();
		//		//		for (auto j = 0; j < sd_cnt; j++)
		//		//		{
		//		//			auto in = n->input_slots[22].get_link(j)->out_node;
		//		//			p->add_shader(r->get_shader_by_node(in));
		//		//		}

		//		//		p->build_graphics();

		//		//		r->_priv->pls.emplace_back(n, p);
		//		//	}
		//		//}

		//		//for (auto i = 0; i < n_cnt; i++)
		//		//{
		//		//	auto n = s->get_node(i);
		//		//	if (n->name_hash == cH("Descriptor Set"))
		//		//	{
		//		//		auto l = n->input_slots[1].get_link(0);
		//		//		auto pl = r->get_pipeline_by_node(l->out_node);
		//		//		auto ds = d->dp->create_descriptorset(pl, l->out_idx - 1);
		//		//		auto cnt = n->input_slots[2].int_val;
		//		//		for (auto j = 0; j < cnt; j++)
		//		//		{
		//		//			auto in = n->input_slots[7].get_split(j)->get_link(0)->out_node;
		//		//			if (in->name_hash == cH("Buffer"))
		//		//			{
		//		//				auto binding = n->input_slots[3].get_split(j)->int_val;
		//		//				if (n->input_slots[4].get_split(j)->get_link_count() == 1)
		//		//				{
		//		//					auto l = n->input_slots[4].get_split(j)->get_link(0);
		//		//					binding = l->out_node->output_slots[l->out_idx].int_val;
		//		//				}
		//		//				ds->set_uniformbuffer(binding, n->input_slots[5].get_split(j)->int_val, 
		//		//					r->get_buffer_by_node(in));
		//		//			}
		//		//		}

		//		//		r->_priv->dss.emplace_back(n, ds);
		//		//	}
		//		//}
			}

		//	inline ~RenderpathPrivate()
		//	{
		//		for (auto i = 0; i < s->node_count(); i++)
		//		{
		//			auto n = s->node(i);
		//			switch (n->name_hash)
		//			{
		//			case cH("Shader"):
		//				Shader::release((Shader*)n->outputslot(0)->p_val);
		//				break;
		//			case cH("Buffer"):
		//				Buffer::destroy((Buffer*)n->outputslot(0)->p_val);
		//				break;
		//			case cH("Image"):
		//				Image::destroy((Image*)n->outputslot(0)->p_val);
		//				break;
		//			case cH("Render Pass"):
		//				Renderpass::release((Renderpass*)n->outputslot(0)->p_val);
		//				break;
		//			case cH("Pipeline"):
		//				Pipeline::destroy((Pipeline*)n->outputslot(0)->p_val);
		//				break;
		//			case cH("Descriptor Set"):
		//				Descriptorset::destroy((Descriptorset*)n->outputslot(0)->p_val);
		//				break;
		//			}
		//		}

		//		blueprint::Scene::destroy(s);
		//	}

		//	blueprint::Node *get_node_by_tag(const char *tag) const
		//	{
		//		for (auto i = 0; i < s->node_count(); i++)
		//		{
		//			auto n = s->node(i);
		//			if (strcmp(n->get_tag(), tag) == 0)
		//				return n;
		//		}
		//		return nullptr;
		//	}
		};

		//blueprint::Node *Renderpath::get_node_by_tag(const char *tag) const
		//{
		//	return ((RenderpathPrivate*)this)->get_node_by_tag(tag);
		//}

		Renderpath *Renderpath::create(Device *d, const wchar_t *filename)
		{
			return new RenderpathPrivate(d, filename);
		}

		void Renderpath::destroy(Renderpath *r)
		{
			delete(RenderpathPrivate*)r;
		}
	}
}
