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

#include <flame/foundation/foundation.h>
#include <flame/foundation/window.h>
#include <flame/graphics/device.h>
#include <flame/graphics/commandbuffer.h>
#include <flame/graphics/swapchain.h>

#include <algorithm>

using namespace flame;
using namespace graphics;

extern "C" __declspec(dllexport) int main()
{
	Ivec2 res(1280, 720);

	auto app = Application::create();
	auto w = Window::create(app, "Graphics Test", res, WindowFrame);

	auto d = Device::create(true);
	auto sc = Swapchain::create(d, w);

	//auto near_plane = 0.1f;
	//auto far_plane = 1000.f;
	//auto fovy = 60.f;

	//auto aspect = (float)res.x / res.y;

	//auto q = create_queue(d);
	//auto cp = create_commandpool(d);

	//struct UBO_matrix
	//{
	//	mat4 proj;
	//	mat4 view;
	//};

	//auto ub_matrix = create_buffer(d, sizeof(UBO_matrix), BufferUsageTransferDst |
	//	BufferUsageUniformBuffer, MemPropDevice);

	//struct UBO_matrix_ins
	//{
	//	mat4 model[65536];
	//};

	//auto ub_matrix_ins = create_buffer(d, sizeof(UBO_matrix_ins), BufferUsageTransferDst |
	//	BufferUsageStorageBuffer, MemPropDevice);

	//auto ub_stag = create_buffer(d, ub_matrix->size + ub_matrix_ins->size, BufferUsageTransferSrc,
	//	MemPropHost | MemPropHostCoherent);
	//ub_stag->map();

	//struct CopyBufferUpdate
	//{
	//	Buffer *src;
	//	Buffer *dst;
	//	std::vector<BufferCopy> ranges;
	//};

	//std::vector<CopyBufferUpdate> updates;

	//auto ubo_matrix = (UBO_matrix*)ub_stag->mapped;
	//ubo_matrix->proj = mat4(
	//	vec4(1.f, 0.f, 0.f, 0.f),
	//	vec4(0.f, -1.f, 0.f, 0.f),
	//	vec4(0.f, 0.f, 1.f, 0.f),
	//	vec4(0.f, 0.f, 0.f, 1.f)
	//) * perspective(radians(fovy), aspect, near_plane, far_plane);
	//ubo_matrix->view = lookAt(vec3(0.f, 3.f, 10.f), vec3(0.f, 3.f, 0.f), vec3(0.f, 1.f, 0.f));

	//CopyBufferUpdate upd;
	//upd.src = ub_stag;
	//upd.dst = ub_matrix;
	//upd.ranges.push_back({0, 0, ub_matrix->size});
	//updates.push_back(upd);

	//auto ubo_matrix_ins = (UBO_matrix_ins*)((unsigned char*)ub_stag->mapped + ub_matrix->size);

	////auto m = load_model("../../Vulkan/data/models/voyager/voyager.dae");
	//auto m = load_model("d:/my_models/robot.dae");
	//m->root_node->calc_global_matrix();

	//Format depth_format;
	//depth_format.v = Format::Depth16;

	//auto depth_tex = create_texture(d, res.x, res.y, 1, 1, depth_format, 
	//	TextureUsageAttachment, MemPropDevice);
	//auto depth_tex_view = create_textureview(d, depth_tex);

	//auto rp = create_renderpass(d);
	//rp->add_attachment_swapchain(sc, true);
	//rp->add_attachment(depth_format, true);
	//rp->add_subpass({0}, 1);
	//rp->build();

	//auto p = create_pipeline(d, rp, 0);
	//p->set_vertex_attributes({{
	//		VertexAttributeFloat3, 
	//		//VertexAttributeFloat2,
	//		VertexAttributeFloat3
	//}});
	//p->set_size(res.x, res.y);
	//p->set_depth_test(true);
	//p->set_depth_write(true);
	//p->add_shader("test/test.vert", {});
	//p->add_shader("test/test.frag", {});
	//p->build_graphics();

	//auto dp = create_descriptorpool(d);
	//auto ds = dp->create_descriptorset(p, 0);
	//ds->set_uniformbuffer(0, 0, ub_matrix);
	//ds->set_storagebuffer(1, 0, ub_matrix_ins);

	//auto vb = create_buffer(d, m->vertex_count * m->vertex_buffers[0].size * sizeof(float), BufferUsageVertexBuffer | 
	//	BufferUsageTransferDst, MemPropDevice);
	//auto ib = create_buffer(d, m->indice_count * sizeof(int), BufferUsageIndexBuffer |
	//	BufferUsageTransferDst, MemPropDevice);
	//auto sb = create_buffer(d, vb->size + ib->size, BufferUsageTransferSrc, 
	//	MemPropHost | MemPropHostCoherent);
	//sb->map();
	//{
	//	auto c = cp->create_commandbuffer();
	//	c->begin(true);
	//	memcpy(sb->mapped, m->vertex_buffers[0].pVertex, vb->size);
	//	BufferCopy r1 = {0, 0, vb->size};
	//	c->copy_buffer(sb, vb, 1, &r1);
	//	memcpy((unsigned char*)sb->mapped + vb->size, m->pIndices, ib->size);
	//	BufferCopy r2 = {vb->size, 0, ib->size};
	//	c->copy_buffer(sb, ib, 1, &r2);
	//	c->end();
	//	q->submit(c, nullptr, nullptr);
	//	q->wait_idle();
	//	cp->destroy_commandbuffer(c);
	//}
	//sb->unmap();
	//destroy_buffer(d, sb);

	//auto sampler = create_sampler(d, FilterLinear, FilterLinear,
	//	false);

	////auto m_map = create_texture_from_file(d, cp, q, "../../Vulkan/data/models/voyager/voyager_bc3_unorm.ktx");
	////auto m_map_view = create_textureview(d, m_map);
	////ds->set_texture(1, 0, m_map_view, sampler);

	//Framebuffer *fbs[2];
	//Commandbuffer *cbs[2];
	//for (auto i = 0; i < 2; i++)
	//{
	//	fbs[i] = create_framebuffer(d, res.x, res.y, rp);
	//	fbs[i]->set_view_swapchain(0, sc, i);
	//	fbs[i]->set_view(1, depth_tex_view);
	//	fbs[i]->build();

	//	cbs[i] = cp->create_commandbuffer();
	//	cbs[i]->begin();
	//	cbs[i]->begin_renderpass(rp, fbs[i]);
	//	cbs[i]->bind_pipeline(p);
	//	cbs[i]->bind_descriptorset(ds);
	//	cbs[i]->bind_vertexbuffer(vb);
	//	cbs[i]->bind_indexbuffer(ib, IndiceTypeUint);
	//	for (auto j = 0; j < m->mesh_count; j++)
	//		cbs[i]->draw_indexed(m->meshes[j]->indice_count, m->meshes[j]->indice_base, 1, j);
	//	//cbs[i]->draw(m->get_vertex_count());
	//	cbs[i]->end_renderpass();
	//	cbs[i]->end();
	//}

	//auto cb_update = cp->create_commandbuffer();

	//auto image_avalible = create_semaphore(d);
	//auto render_finished = create_semaphore(d);

	//auto x_ang = 0.f;
	//auto view_changed = true;
	//s->add_mousemove_listener([&](Surface *s, int, int){
	//	if (s->mouse_buttons[0] & KeyStateDown)
	//	{
	//		x_ang += s->mouse_disp_x;
	//		view_changed = true;
	//	}
	//});

	//sm->run([&](){
	//	if (view_changed)
	//	{
	//		for (auto i = 0; i < m->mesh_count; i++)
	//		{
	//			ubo_matrix_ins->model[i] = rotate(radians(x_ang), vec3(0.f, 1.f, 0.f));
	//			if (m->meshes[i]->pNode)
	//				ubo_matrix_ins->model[i] = ubo_matrix_ins->model[i] * m->meshes[i]->pNode->global_matrix;
	//			//ubo_matrix_ins->model[i] = transpose(ubo_matrix_ins->model[i]);
	//		}

	//		CopyBufferUpdate upd;
	//		upd.src = ub_stag;
	//		upd.dst = ub_matrix_ins;
	//		upd.ranges.push_back({ub_matrix->size, 0, int(m->mesh_count * sizeof(mat4))});
	//		updates.push_back(upd);

	//		view_changed = false;
	//	}

	//	if (!updates.empty())
	//	{
	//		cb_update->begin(true);
	//		for (auto &u : updates)
	//			cb_update->copy_buffer(u.src, u.dst, u.ranges.size(), u.ranges.data());
	//		cb_update->end();
	//		q->submit(cb_update, nullptr, nullptr);
	//		q->wait_idle();
	//		updates.clear();
	//	}

	//	auto index = sc->acquire_image(image_avalible);
	//	q->submit(cbs[index], image_avalible, render_finished);
	//	q->present(index, sc, render_finished);

	//	//static long long last_ns = 0;
	//	//auto t = get_now_ns();
	//	//if (t - last_ns >= 10000000)
	//	//{
	//	//	static int B = 0;
	//	//	vec4 color(0.5f, 0.7f, (B / 255.f), 1.f);
	//	//	memcpy(ub->mapped, &color, sizeof(vec4));
	//	//	B++;
	//	//	if (B == 256)
	//	//		B = 0;
	//	//	last_ns = t;
	//	//}

	//	static long long last_fps = 0;
	//	if (last_fps != sm->fps)
	//		printf("%lld\n", sm->fps);
	//	last_fps = sm->fps;
	//});

	return 0;
}
