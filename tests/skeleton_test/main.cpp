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

#include <flame/time.h>
#include <flame/system.h>
#include <flame/surface.h>
#include <flame/image.h>
#include <flame/math.h>
#include <flame/model/model.h>
#include <flame/graphics/device.h>
#include <flame/graphics/swapchain.h>
#include <flame/graphics/renderpass.h>
#include <flame/graphics/pipeline.h>
#include <flame/graphics/descriptor.h>
#include <flame/graphics/buffer.h>
#include <flame/graphics/texture.h>
#include <flame/graphics/sampler.h>
#include <flame/graphics/framebuffer.h>
#include <flame/graphics/commandbuffer.h>
#include <flame/graphics/semaphore.h>
#include <flame/graphics/queue.h>

#include <algorithm>

int main(int argc, char **args)
{
	using namespace flame;
	using namespace graphics;
	using namespace glm;

	auto near_plane = 0.1f;
	auto far_plane = 1000.f;
	auto fovy = 60.f;

	vec2 res(1280, 720);

	auto aspect = (float)res.x / res.y;

	auto sm = create_surface_manager();
	auto s = sm->create_surface(res.x, res.y, SurfaceStyleFrame,
		"Hello");

	auto d = create_device(true);

	auto sc = create_swapchain(d, s->get_win32_handle(), s->cx, s->cy);

	auto q = create_queue(d);
	auto cp = create_commandpool(d);

	auto ub = create_buffer(d, sizeof(mat4) * 3, BufferUsageUniformBuffer, 
		MemPropHost | MemPropHostCoherent);
	ub->map();

	struct UBO
	{
		mat4 proj;
		mat4 view;
		mat4 model;
	};

	auto ubo = (UBO*)ub->mapped;
	ubo->proj = mat4(
		vec4(1.f, 0.f, 0.f, 0.f),
		vec4(0.f, -1.f, 0.f, 0.f),
		vec4(0.f, 0.f, 1.f, 0.f),
		vec4(0.f, 0.f, 0.f, 1.f)
	) * perspective(radians(fovy), aspect, near_plane, far_plane);
	ubo->view = lookAt(vec3(0.f, 0.f, 20.f), vec3(0.f), vec3(0.f, 1.f, 0.f));

	Format depth_format;
	depth_format.v = Format::Depth16;

	auto depth_tex = create_texture(d, res.x, res.y, 1, 1, depth_format, 
		TextureUsageAttachment, MemPropDevice);
	auto depth_tex_view = create_textureview(d, depth_tex);

	auto rp = create_renderpass(d);
	rp->add_attachment_swapchain(sc, true);
	rp->add_attachment(depth_format, true);
	rp->add_subpass({0}, 1);
	rp->build();

	auto pipeline = create_pipeline(d, rp, 0);
	pipeline->set_vertex_attributes({{
			VertexAttributeFloat3, 
			VertexAttributeFloat2,
			VertexAttributeFloat3,
			VertexAttributeFloat4,
			VertexAttributeFloat4
	}});
	pipeline->set_size(res.x, res.y);
	pipeline->set_depth_test(true);
	pipeline->set_depth_write(true);
	pipeline->add_shader("test/test_skeleton.vert", {});
	pipeline->add_shader("test/test.frag", {});
	pipeline->build_graphics();

	auto pipeline_line = create_pipeline(d, rp, 0);
	pipeline_line->set_vertex_attributes({{
			VertexAttributeFloat2
		}});
	pipeline_line->set_polygon_mode(PolygonModeLine);
	pipeline_line->set_size(res.x, res.y);
	pipeline_line->set_cull_mode(CullModeNone);
	pipeline_line->add_shader("test/line2d.vert", {});
	pipeline_line->add_shader("test/line2d.frag", {});
	pipeline_line->build_graphics();

	auto dp = create_descriptorpool(d);
	auto ds = dp->create_descriptorset(pipeline, 0);
	ds->set_uniformbuffer(0, 0, ub);

	auto m = load_model("../../Vulkan/data/models/goblin.dae");
	m->root_node->calc_global_matrix();
	auto global_inverse = inverse(m->root_node->global_matrix);

	auto vb = create_buffer(d, m->vertex_count * m->vertex_buffers[0].size * sizeof(float), BufferUsageVertexBuffer | 
		BufferUsageTransferDst, MemPropDevice);
	auto ib = create_buffer(d, m->indice_count * sizeof(int), BufferUsageIndexBuffer |
		BufferUsageTransferDst, MemPropDevice);
	auto sb = create_buffer(d, vb->size + ib->size, BufferUsageTransferSrc, 
		MemPropHost | MemPropHostCoherent);
	sb->map();
	{
		auto c = cp->create_commandbuffer();
		c->begin(true);
		memcpy(sb->mapped, m->vertex_buffers[0].pVertex, vb->size);
		BufferCopy r1 = {0, 0, vb->size};
		c->copy_buffer(sb, vb, 1, &r1);
		memcpy((unsigned char*)sb->mapped + vb->size, m->pIndices, ib->size);
		BufferCopy r2 = {vb->size, 0, ib->size};
		c->copy_buffer(sb, ib, 1, &r2);
		c->end();
		q->submit(c, nullptr, nullptr);
		q->wait_idle();
		cp->destroy_commandbuffer(c);
	}
	sb->unmap();
	destroy_buffer(d, sb);

	auto vb_bone_pos = create_buffer(d, m->bone_count * 3 * sizeof(vec2), BufferUsageVertexBuffer,
		MemPropHost | MemPropHostCoherent);
	vb_bone_pos->map();
	auto bone_pos = (vec2*)vb_bone_pos->mapped;

	auto ub_bone = create_buffer(d, sizeof(mat4) * m->bone_count, BufferUsageUniformBuffer,
		MemPropHost | MemPropHostCoherent);
	ub_bone->map();
	auto bone_matrix = (mat4*)ub_bone->mapped;
	for (auto i = 0; i < m->bone_count; i++)
		bone_matrix[i] = m->bones[i]->pNode->global_matrix * m->bones[i]->offset_matrix;

	ds->set_uniformbuffer(2, 0, ub_bone);

	auto sampler = create_sampler(d, FilterLinear, FilterLinear,
		false);

	//auto m_map = create_texture_from_file(d, cp, q, "../../Vulkan/data/textures/goblin_bc3_unorm.ktx");
	//auto m_map_view = create_textureview(d, m_map);
	//ds->set_texture(1, 0, m_map_view, sampler);

	Framebuffer *fbs[2];
	Commandbuffer *cbs[2];
	for (auto i = 0; i < 2; i++)
	{
		fbs[i] = create_framebuffer(d, res.x, res.y, rp);
		fbs[i]->set_view_swapchain(0, sc, i);
		fbs[i]->set_view(1, depth_tex_view);
		fbs[i]->build();

		cbs[i] = cp->create_commandbuffer();
		cbs[i]->begin();
		cbs[i]->begin_renderpass(rp, fbs[i]);
		cbs[i]->bind_pipeline(pipeline);
		cbs[i]->bind_descriptorset(ds);
		cbs[i]->bind_vertexbuffer(vb);
		cbs[i]->bind_indexbuffer(ib, IndiceTypeUint);
		cbs[i]->draw_indexed(m->indice_count, 0, 1, 0);
		cbs[i]->bind_pipeline(pipeline_line);
		cbs[i]->bind_vertexbuffer(vb_bone_pos);
		cbs[i]->draw(m->bone_count * 3, 1, 0);
		cbs[i]->end_renderpass();
		cbs[i]->end();
	}

	auto image_avalible = create_semaphore(d);
	auto render_finished = create_semaphore(d);

	auto x_ang = 0.f;
	auto view_changed = true;
	s->add_mousemove_listener([&](Surface *s, int, int){
		if (s->mouse_buttons[0] & KeyStateDown)
		{
			x_ang += s->mouse_disp_x;
			view_changed = true;
		}
	});

	struct Channel
	{
		std::vector<ModelPositionKey> position_keys;
		std::vector<ModelRotationKey> rotation_keys;
	};

	auto anim = m->animations[0];
	auto total_time = (float)anim->total_ticks / anim->ticks_per_second;

	struct AnimationPlayer
	{
		float time;
		ModelNode *root_bone;
		glm::mat4 *bone_matrix;
		std::vector<std::unique_ptr<Channel>> channels;

		void update_node(ModelNode *n)
		{
			ModelBone *b = nullptr;
			if (n->type == ModelNodeBone)
				b = (ModelBone*)n->p;
			if (b)
			{
				auto ch = channels[b->id].get();
				if (ch)
				{
					vec4 quat = vec4(0.f, 0.f, 0.f, 1.f);
					vec3 position = vec3(0.f);

					if (ch->rotation_keys.size() == 1)
						quat = ch->rotation_keys[0].value;
					else if (ch->rotation_keys.size() > 0)
					{
						auto curr_frame = std::lower_bound(ch->rotation_keys.begin() + 1, 
							ch->rotation_keys.end(), time, 
							[](const ModelRotationKey &a, float v){
							return a.time < v;
						});
						auto next_frame = ch->rotation_keys.begin() + ((std::distance(ch->rotation_keys.begin(), curr_frame) + 1)
							% ch->rotation_keys.size());
						auto t = (time - next_frame->time) / (curr_frame->time - next_frame->time);

						quat = (1 - t) * next_frame->value + t * curr_frame->value;
						quat = normalize(quat);
					}

					if (ch->position_keys.size() == 1)
						position = ch->position_keys[0].value;
					else if (ch->position_keys.size() > 0)
					{
						auto curr_frame = std::lower_bound(ch->position_keys.begin() + 1,
							ch->position_keys.end(), time,
							[](const ModelPositionKey &a, float v){
							return a.time < v;
						});
						auto next_frame = ch->position_keys.begin() + ((std::distance(ch->position_keys.begin(), curr_frame) + 1)
							% ch->position_keys.size());
						auto t = (time - next_frame->time) / (curr_frame->time - next_frame->time);

						position = (1 - t) * next_frame->value + t * curr_frame->value;
					}

					n->global_matrix = n->parent->global_matrix * translate(position) * mat4(quat_to_mat3(quat));
				}

				bone_matrix[b->id] = n->global_matrix * b->offset_matrix;
			}
			else
				n->global_matrix = n->parent->global_matrix * transpose(n->local_matrix);

			auto c = n->first_child;
			while (c)
			{
				update_node(c);
				c = c->next_sibling;
			}
		}

		void update(float _time)
		{
			time = _time;
			update_node(root_bone);
		}
	}anim_player;
	anim_player.root_bone = m->root_bone;
	anim_player.bone_matrix = bone_matrix;
	anim_player.channels.resize(m->bone_count);
	for (auto i = 0; i < m->bone_count; i++)
	{
		auto iMo = anim->find_motion(m->bones[i]->name);
		if (iMo == -1)
			anim_player.channels[i] = nullptr;
		else
		{
			anim_player.channels[i] = std::make_unique<Channel>();
			auto mo = &anim->motions[iMo];
			auto ch = anim_player.channels[i].get();
			ch->position_keys.resize(mo->position_key_count);
			for (auto j = 0; j < mo->position_key_count; j++)
			{
				ch->position_keys[j].time = mo->position_keys[j].time;
				ch->position_keys[j].value = mo->position_keys[j].value;
			}
			ch->rotation_keys.resize(mo->rotation_key_count);
			for (auto j = 0; j < mo->rotation_key_count; j++)
			{
				ch->rotation_keys[j].time = mo->rotation_keys[j].time;
				ch->rotation_keys[j].value = mo->rotation_keys[j].value;
			}
		}
	}

	sm->run([&](){
		auto update_bone_pos = [&]() {
			for (auto i = 0; i < m->bone_count; i++)
			{
				auto b = m->bones[i];

				if (b->pNode->parent)
				{
					auto p0 = b->pNode->parent->global_matrix[3];
					auto p1 = b->pNode->global_matrix[3];

					p0 = ubo->model * p0;
					p1 = ubo->model * p1;

					p0 = ubo->proj * ubo->view * p0;
					p0 /= p0.w;
					p1 = ubo->proj * ubo->view * p1;
					p1 /= p1.w;

					if (glm::length(p0 - p1) > 0.001f)
					{
						auto w = glm::normalize(
							glm::cross(glm::vec3(p1) - glm::vec3(p0), glm::vec3(0.f, 0.f, 1.f)));
						w *= 5.f / glm::length(glm::vec2(w.x * res.x, w.y * res.y));
						bone_pos[i * 3 + 0] = vec2(p0) + vec2(w);
						bone_pos[i * 3 + 1] = vec2(p0) - vec2(w);
						bone_pos[i * 3 + 2] = p1;

						continue;
					}
				}

				bone_pos[i * 3 + 0] = vec2(-10.f, -10.f);
				bone_pos[i * 3 + 1] = vec2(-10.f, -10.f);
				bone_pos[i * 3 + 2] = vec2(-10.f, -10.f);
			}
		};

		auto need_update_bone_pos = false;

		if (view_changed)
		{
			ubo->model = rotate(radians(x_ang), vec3(0.f, 1.f, 0.f)) * scale(vec3(0.1f));
			need_update_bone_pos = true;

			view_changed = false;
		}
		
		static float time = 0.f;
		static long long last_ns = 0;
		auto t = get_now_ns();
		if (t - last_ns >= 41666666)
		{
			anim_player.update(time);
			need_update_bone_pos = true;
			time += 0.041666;
			time = fmod(time, total_time);

			last_ns = t;
		}

		if (need_update_bone_pos)
			update_bone_pos();

		auto index = sc->acquire_image(image_avalible);
		q->submit(cbs[index], image_avalible, render_finished);
		q->present(index, sc, render_finished);

		//static long long last_ns = 0;
		//auto t = get_now_ns();
		//if (t - last_ns >= 10000000)
		//{
		//	static int B = 0;
		//	vec4 color(0.5f, 0.7f, (B / 255.f), 1.f);
		//	memcpy(ub->mapped, &color, sizeof(vec4));
		//	B++;
		//	if (B == 256)
		//		B = 0;
		//	last_ns = t;
		//}

		static long long last_fps = 0;
		if (last_fps != sm->fps)
			printf("%lld\n", sm->fps);
		last_fps = sm->fps;
	});

	return 0;
}
