#include <flame/3d/camera.h>
#include "model_private.h"
#include "scene_private.h"

#include <flame/bitmap.h>
#include <flame/graphics/device.h>
#include <flame/graphics/buffer.h>
#include <flame/graphics/image.h>
#include <flame/graphics/renderpass.h>
#include <flame/graphics/framebuffer.h>
#include <flame/graphics/shader.h>
#include <flame/graphics/pipeline.h>
#include <flame/graphics/descriptor.h>
#include <flame/graphics/commandbuffer.h>
#include <flame/graphics/queue.h>

#include <assert.h>

namespace flame
{
	namespace _3d
	{
		static const int bake_wnd_size = 128;
		static Mat4 bake_window_proj;

		void ShareData::create(graphics::Device *_d)
		{
			d = _d;

			{
				graphics::RenderpassInfo info;
				info.attachments.resize(2);
				info.attachments[0].format$ = graphics::Format_R32G32B32A32_SFLOAT;
				info.attachments[0].clear$ = true;
				info.attachments[1].format$ = graphics::Format_Depth16;
				info.attachments[1].clear$ = true;
				info.subpasses[0].color_attachments.push_back(0);
				info.subpasses[0].depth_attachment = 1;
				rp_scene = graphics::Renderpass::get(d, info);
			}
			{
				graphics::RenderpassInfo info;
				info.attachments.resize(1);
				info.attachments[0].format$ = graphics::Format_R32G32B32A32_SFLOAT;
				info.subpasses[0].color_attachments.push_back(0);
				rp_one_att = graphics::Renderpass::get(d, info);
			}

			{
				graphics::GraphicsPipelineInfo info;
				info.shaders.resize(2);
				info.shaders[0].filename = L"fullscreen.vert";
				info.shaders[1].filename = L"sky/blue.frag";
				info.renderpass = rp_scene;
				pl_sky_blue = graphics::Pipeline::create(d, info);
			}
			{
				graphics::GraphicsPipelineInfo info;
				info.shaders.resize(2);
				info.shaders[0].filename = L"fullscreen.vert";
				info.shaders[0].prefix = "#define VIEW_CAMERA\n";
				info.shaders[1].filename = L"sky/brightsun.frag";
				info.renderpass = rp_scene;
				pl_sky_brightsun = graphics::Pipeline::create(d, info);
			}
			{
				graphics::GraphicsPipelineInfo info;
				info.shaders.resize(2);
				info.shaders[0].filename = L"forward.vert";
				info.shaders[0].prefix = "#define POS\n#define POS_LOCATION 0\n#define UV\n#define UV_LOCATION 1\n";
				info.shaders[1].filename = L"forward_lightmap.frag";
				info.vi_buffers.resize(2);
				info.vi_buffers[0] = graphics::VertexInputBufferInfo({ graphics::Format_R32G32B32_SFLOAT });
				info.vi_buffers[1] = graphics::VertexInputBufferInfo({ graphics::Format_R32G32_SFLOAT });
				info.depth_test = true;
				info.depth_write = true;
				info.renderpass = rp_scene;
				pl_lightmap = graphics::Pipeline::create(d, info);
			}
			{
				graphics::GraphicsPipelineInfo info;
				info.shaders.resize(2);
				info.shaders[0].filename = L"forward.vert";
				info.shaders[0].prefix = "#define POS\n#define POS_LOCATION 0\n#define NORMAL\n#define NORMAL_LOCATION 1\n";
				info.shaders[1].filename = L"forward_cameralight.frag";
				info.vi_buffers.resize(2);
				info.vi_buffers[0] = graphics::VertexInputBufferInfo({ graphics::Format_R32G32B32_SFLOAT });
				info.vi_buffers[1] = graphics::VertexInputBufferInfo({ graphics::Format_R32G32B32_SFLOAT });
				info.depth_test = true;
				info.depth_write = true;
				info.renderpass = rp_scene;
				pl_cameralight = graphics::Pipeline::create(d, info);
			}
			{
				graphics::GraphicsPipelineInfo info;
				info.shaders.resize(2);
				info.shaders[0].filename = L"forward.vert";
				info.shaders[0].prefix = "#define POS\n#define POS_LOCATION 0\n";
				info.shaders[1].filename = L"frame.frag";
				info.vi_buffers.resize(1);
				info.vi_buffers[0] = graphics::VertexInputBufferInfo({ graphics::Format_R32G32B32_SFLOAT });
				info.primitive_topology = graphics::PrimitiveTopologyLineList;
				info.renderpass = rp_scene;
				pl_frame = graphics::Pipeline::create(d, info);
			}

			auto hf_bake_wnd_size = bake_wnd_size / 2;
			auto n = Vec3(0.f, 0.f, -1.f);
			auto sum = 0.f;

			bk_fix_center = new float[bake_wnd_size * bake_wnd_size];
			for (auto y = 0; y < bake_wnd_size; y++)
			{
				for (auto x = 0; x < bake_wnd_size; x++)
				{
					auto vn = dot(Vec3((x + 0.5f - hf_bake_wnd_size) / hf_bake_wnd_size, (-y - 0.5f + hf_bake_wnd_size) / hf_bake_wnd_size, -1.f).get_normalized(), n);
					auto value = vn * vn;
					sum += value;
					bk_fix_center[y * bake_wnd_size + x] = value;
				}
			}

			bk_fix_left = new float[hf_bake_wnd_size * bake_wnd_size];
			for (auto y = 0; y < bake_wnd_size; y++)
			{
				for (auto x = 0; x < hf_bake_wnd_size; x++)
				{
					auto vn = dot(Vec3((x + 0.5f) / hf_bake_wnd_size, (-y - 0.5f + hf_bake_wnd_size) / hf_bake_wnd_size, -1.f).get_normalized(), n);
					auto dn = dot(Vec3(-1.f, (-y - 0.5f + hf_bake_wnd_size) / hf_bake_wnd_size, (-x - 0.5f) / hf_bake_wnd_size).get_normalized(), n);
					auto value = vn * dn;
					sum += value;
					bk_fix_left[y * hf_bake_wnd_size + x] = value;
				}
			}

			bk_fix_right = new float[hf_bake_wnd_size * bake_wnd_size];
			for (auto y = 0; y < bake_wnd_size; y++)
			{
				for (auto x = 0; x < hf_bake_wnd_size; x++)
				{
					auto vn = dot(Vec3((x + 0.5f - hf_bake_wnd_size) / hf_bake_wnd_size, (-y - 0.5f + hf_bake_wnd_size) / hf_bake_wnd_size, -1.f).get_normalized(), n);
					auto dn = dot(Vec3(1.f, (-y - 0.5f + hf_bake_wnd_size) / hf_bake_wnd_size, (x + 0.5f - hf_bake_wnd_size) / hf_bake_wnd_size).get_normalized(), n);
					auto value = vn * dn;
					sum += value;
					bk_fix_right[y * hf_bake_wnd_size + x] = value;
				}
			}

			bk_fix_top = new float[bake_wnd_size * hf_bake_wnd_size];
			for (auto y = 0; y < hf_bake_wnd_size; y++)
			{
				for (auto x = 0; x < bake_wnd_size; x++)
				{
					auto vn = dot(Vec3((x + 0.5f - hf_bake_wnd_size) / hf_bake_wnd_size, (-y - 0.5f) / hf_bake_wnd_size, -1.f).get_normalized(), n);
					auto dn = dot(Vec3((x + 0.5f - hf_bake_wnd_size) / hf_bake_wnd_size, 1.f, (-y - 0.5f) / hf_bake_wnd_size).get_normalized(), n);
					auto value = vn * dn;
					sum += value;
					bk_fix_top[y * bake_wnd_size + x] = value;
				}
			}

			bk_fix_bottom = new float[bake_wnd_size * hf_bake_wnd_size];
			for (auto y = 0; y < hf_bake_wnd_size; y++)
			{
				for (auto x = 0; x < bake_wnd_size; x++)
				{
					auto vn = dot(Vec3((x + 0.5f - hf_bake_wnd_size) / hf_bake_wnd_size, (-y - 0.5f + hf_bake_wnd_size) / hf_bake_wnd_size, -1.f).get_normalized(), n);
					auto dn = dot(Vec3((x + 0.5f - hf_bake_wnd_size) / hf_bake_wnd_size, -1.f, (y + 0.5f - hf_bake_wnd_size) / hf_bake_wnd_size).get_normalized(), n);
					auto value = vn * dn;
					sum += value;
					bk_fix_bottom[y * bake_wnd_size + x] = value;
				}
			}

			for (auto y = 0; y < bake_wnd_size; y++)
			{
				for (auto x = 0; x < bake_wnd_size; x++)
					bk_fix_center[y * bake_wnd_size + x] /= sum;
			}

			for (auto y = 0; y < bake_wnd_size; y++)
			{
				for (auto x = 0; x < hf_bake_wnd_size; x++)
					bk_fix_left[y * hf_bake_wnd_size + x] /= sum;
			}

			for (auto y = 0; y < bake_wnd_size; y++)
			{
				for (auto x = 0; x < hf_bake_wnd_size; x++)
					bk_fix_right[y * hf_bake_wnd_size + x] /= sum;
			}

			for (auto y = 0; y < hf_bake_wnd_size; y++)
			{
				for (auto x = 0; x < bake_wnd_size; x++)
					bk_fix_top[y * bake_wnd_size + x] /= sum;
			}

			for (auto y = 0; y < hf_bake_wnd_size; y++)
			{
				for (auto x = 0; x < bake_wnd_size; x++)
					bk_fix_bottom[y * bake_wnd_size + x] /= sum;
			}
		}

		void ShareData::destroy()
		{

			delete[]bk_fix_center;
			delete[]bk_fix_left;
			delete[]bk_fix_right;
			delete[]bk_fix_top;
			delete[]bk_fix_bottom;
		}

		ShareData share_data;

		void init(graphics::Device *d)
		{
			share_data.create(d);

			printf("3d initialized\n");
		}

		void deinit()
		{
			share_data.destroy();
		}

		inline ScenePrivate::ScenePrivate(const Ivec2 &resolution)
		{
			res = resolution;
			show_mode = ShowModeLightmap;
			show_frame = true;

			c = nullptr;

			bk_ratio = 0.f;
			bk_imgsize = Ivec2(0);
			bk_pen_pos = Ivec2(0);
			bk_pen_lineheight = 0;

			matrix_buf = graphics::Buffer::create(share_data.d, sizeof(Mat4) * 2 + sizeof(Vec4), graphics::BufferUsageUniform, graphics::MemPropHost);
			matrix_buf->map();

			col_image = graphics::Image::create(share_data.d, graphics::Format_R32G32B32A32_SFLOAT, resolution, 1, 1, graphics::SampleCount_1, graphics::ImageUsageSampled | graphics::ImageUsageAttachment, graphics::MemPropDevice);
			dep_image = graphics::Image::create(share_data.d, graphics::Format_Depth16, resolution, 1, 1, graphics::SampleCount_1, graphics::ImageUsageSampled | graphics::ImageUsageAttachment, graphics::MemPropDevice);

			bk_img = nullptr;

			graphics::FramebufferInfo fb_info;
			fb_info.rp = share_data.rp_scene;
			fb_info.views.resize(2);
			fb_info.views[0] = graphics::Imageview::get(col_image);
			fb_info.views[1] = graphics::Imageview::get(dep_image);
			framebuffer = graphics::Framebuffer::get(share_data.d, fb_info);

			clear_values = graphics::ClearValues::create(share_data.rp_scene);
			clear_values->set(0, Bvec4(0, 0, 0, 255));

			ds_skybrightsun = graphics::Descriptorset::create(share_data.d->dp, share_data.pl_sky_brightsun->layout()->dsl(0));
			ds_skybrightsun->set_uniformbuffer(0, 0, matrix_buf);
			
			ds_lightmap = graphics::Descriptorset::create(share_data.d->dp, share_data.pl_lightmap->layout()->dsl(0));
			ds_lightmap->set_uniformbuffer(0, 0, matrix_buf);

			ds_cameralight = graphics::Descriptorset::create(share_data.d->dp, share_data.pl_cameralight->layout()->dsl(0));
			ds_cameralight->set_uniformbuffer(0, 0, matrix_buf);

			ds_frame = graphics::Descriptorset::create(share_data.d->dp, share_data.pl_frame->layout()->dsl(0));
			ds_frame->set_uniformbuffer(0, 0, matrix_buf);

			cb = graphics::Commandbuffer::create(share_data.d->gcp);
		}

		inline void ScenePrivate::register_model(ModelPrivate *m)
		{
			RegisteredModel r;
			r.m = m;
			r.vc = 0;
			r.vc_frame = 0;

			std::vector<Vec3> positions;
			std::vector<Vec3> normals;
			std::vector<Vec3> frame_lines;
			for (auto &p : m->prims)
			{
				switch (p.pt)
				{
				case PrimitiveTopologyPlane:
				{
					auto p0 = p.p;
					auto p1 = p.p + p.vx;
					auto p2 = p.p + p.vx + p.vz;
					auto p3 = p.p + p.vz;

					positions.push_back(p0);
					positions.push_back(p2);
					positions.push_back(p1);
					positions.push_back(p0);
					positions.push_back(p3);
					positions.push_back(p2);

					auto n = -cross(p.vx, p.vz).get_normalized();

					normals.push_back(n);
					normals.push_back(n);
					normals.push_back(n);
					normals.push_back(n);
					normals.push_back(n);
					normals.push_back(n);

					r.vc += 6;

					frame_lines.push_back(p0);
					frame_lines.push_back(p1);
					frame_lines.push_back(p1);
					frame_lines.push_back(p2);
					frame_lines.push_back(p2);
					frame_lines.push_back(p3);
					frame_lines.push_back(p3);
					frame_lines.push_back(p0);

					r.vc_frame += 8;
				}
					break;
				}
			}

			r.pos_buf = graphics::Buffer::create(share_data.d, positions.size() * sizeof(Vec3), graphics::BufferUsageTransferDst | graphics::BufferUsageVertex, graphics::MemPropDevice, false, positions.data());
			r.uv_buf = nullptr;
			r.normal_buf = graphics::Buffer::create(share_data.d, normals.size() * sizeof(Vec3), graphics::BufferUsageTransferDst | graphics::BufferUsageVertex, graphics::MemPropDevice, false, normals.data());
			r.frame_buf = graphics::Buffer::create(share_data.d, frame_lines.size() * sizeof(Vec3), graphics::BufferUsageTransferDst | graphics::BufferUsageVertex, graphics::MemPropDevice, false, frame_lines.data());

			ms.push_back(r);
		}

		inline void ScenePrivate::set_bake_props(float ratio, const Ivec2 &imgsize)
		{
			bk_ratio = ratio;
			bk_imgsize = imgsize;

			if (bk_img)
				graphics::Image::destroy(bk_img);
			bk_img = graphics::Image::create(share_data.d, graphics::Format_R32G32B32A32_SFLOAT, bk_imgsize, 1, 1, graphics::SampleCount_1, graphics::ImageUsageSampled | graphics::ImageUsageTransferDst, graphics::MemPropDevice);
			//r.bk_img->init(Bvec4(200, 100, 50, 255));
			bk_img->init(Bvec4(0, 0, 0, 255));
			//for (auto y = 0; y < m->bk_imgsize_.y; y++)
			//{
			//	for (auto x = 0; x < m->bk_imgsize_.x; x++)
			//	{
			//		auto b_or_w = (y % 2 == 0) ^ (x % 2 == 0);
			//		r.bk_img->set_pixel_col(x, y, b_or_w == 0 ? Bvec4(0, 0, 0, 255) : Bvec4(255, 255, 255, 255));
			//	}
			//}
			ds_lightmap->set_imageview(1, 0, graphics::Imageview::get(bk_img), share_data.d->sp_bi_linear);

			auto one_over_ratio = 1.f / ratio;
			bake_window_proj = get_proj_mat(ANG_RAD * 90.f, 1.f, one_over_ratio * 0.5f, 1000.f);

			bk_pen_pos = Ivec2(0);
			for (auto &r : ms)
			{
				r.bk_units.clear();

				std::vector<Vec2> uvs;

				for (auto &p : r.m->prims)
				{
					switch (p.pt)
					{
					case PrimitiveTopologyPlane:
					{
						auto normal = -cross(p.vx, p.vz).get_normalized();

						auto x_ext = p.vx.length();
						auto z_ext = p.vz.length();
						auto nx = p.vx.get_normalized();
						auto nz = p.vz.get_normalized();

						auto isize = Ivec2(ceil(x_ext * bk_ratio), ceil(z_ext * bk_ratio));

						auto uv0 = Vec2(bk_pen_pos) / Vec2(bk_imgsize);
						auto uv2 = uv0 + Vec2(x_ext * bk_ratio, z_ext * bk_ratio) / Vec2(bk_imgsize);
						auto uv1 = Vec2(uv2.x, uv0.y);
						auto uv3 = Vec2(uv0.x, uv2.y);

						uvs.push_back(uv0);
						uvs.push_back(uv2);
						uvs.push_back(uv1);
						uvs.push_back(uv0);
						uvs.push_back(uv3);
						uvs.push_back(uv2);

						bk_pen_lineheight = max(bk_pen_lineheight, isize.y);
						assert(bk_pen_pos.y + bk_pen_lineheight < bk_imgsize.y);
						if (bk_pen_pos.x + isize.x > bk_imgsize.x)
						{
							bk_pen_pos.x = 0;
							bk_pen_pos.y += bk_pen_lineheight;
							bk_pen_lineheight = 0;
						}

						auto i = r.bk_units.size();
						r.bk_units.resize(r.bk_units.size() + isize.x * isize.y);
						for (auto x = 0; x < isize.x; x++)
						{
							for (auto z = 0; z < isize.y; z++)
							{
								BakeUnit u;
								u.pos = nx * ((x + 0.5f) / bk_ratio) + nz * ((z + 0.5f) / bk_ratio) + p.p;
								u.normal = normal;
								u.up = -nz;
								u.pixel_coord = bk_pen_pos + Ivec2(x, z);
								r.bk_units[i] = u;
								i++;
							}
						}

						bk_pen_pos.x += isize.x;
					}
						break;
					}
				}

				if (r.uv_buf)
					graphics::Buffer::destroy(r.uv_buf);
				r.uv_buf = graphics::Buffer::create(share_data.d, uvs.size() * sizeof(Vec2), graphics::BufferUsageTransferDst | graphics::BufferUsageVertex, graphics::MemPropDevice, false, uvs.data());
			}
		}

		inline void ScenePrivate::begin(float elp_time)
		{
			elp_time_ = elp_time;
		}

		inline void ScenePrivate::end()
		{
			c->update(elp_time_);

			auto proj = c->proj();
			auto view = c->view();
			auto camera_pos = Vec4(c->pos, 1.f);
			memcpy(matrix_buf->mapped, &proj, sizeof(Mat4));
			memcpy((char*)matrix_buf->mapped + sizeof(Mat4), &view, sizeof(Mat4));
			memcpy((char*)matrix_buf->mapped + sizeof(Mat4) * 2, &camera_pos, sizeof(Vec4));
			matrix_buf->flush();
		}

		void ScenePrivate::draw_scene(graphics::Commandbuffer *cb, const Vec2 &camera_props)
		{
			cb->bind_pipeline(share_data.pl_sky_brightsun);
			cb->bind_descriptorset(ds_skybrightsun, 0);
			cb->push_constant(0, sizeof(Vec2), &camera_props);
			cb->draw(3, 1, 0, 0);

			cb->bind_pipeline(share_data.pl_lightmap);
			cb->bind_descriptorset(ds_lightmap, 0);
			for (auto &m : ms)
			{
				cb->bind_vertexbuffer(m.pos_buf, 0);
				cb->bind_vertexbuffer(m.uv_buf, 1);
				cb->draw(m.vc, 1, 0, 0);
			}
		}

		inline void ScenePrivate::record_cb()
		{
			cb->begin();
			cb->begin_renderpass(share_data.rp_scene, framebuffer, clear_values);

			auto vp = Rect(Vec2(0.f), Vec2(res));
			cb->set_viewport(vp);
			cb->set_scissor(vp);

			switch (show_mode)
			{
			case ShowModeLightmap:
				draw_scene(cb, Vec2(0.5773503f, (float)res.x / (float)res.y));
				break;
			case ShowModeCameraLight:
				cb->bind_pipeline(share_data.pl_sky_blue);
				cb->draw(3, 1, 0, 0);

				cb->bind_pipeline(share_data.pl_cameralight);
				cb->bind_descriptorset(ds_cameralight, 0);
				for (auto &m : ms)
				{
					cb->bind_vertexbuffer(m.pos_buf, 0);
					cb->bind_vertexbuffer(m.normal_buf, 1);
					cb->draw(m.vc, 1, 0, 0);
				}
				break;
			}

			if (show_frame)
			{
				cb->bind_pipeline(share_data.pl_frame);
				cb->bind_descriptorset(ds_frame, 0);
				for (auto &m : ms)
				{
					cb->bind_vertexbuffer(m.frame_buf, 0);
					cb->draw(m.vc_frame, 1, 0, 0);
				}
			}

			cb->end_renderpass();
			cb->end();
		}

		inline void ScenePrivate::bake(int pass)
		{
			auto cb = graphics::Commandbuffer::create(share_data.d->gcp);
			auto img_col = graphics::Image::create(share_data.d, graphics::Format_R32G32B32A32_SFLOAT, Ivec2(bake_wnd_size), 1, 1, graphics::SampleCount_1, graphics::ImageUsageSampled | graphics::ImageUsageAttachment | graphics::ImageUsageTransferSrc, graphics::MemPropDevice);
			auto img_dep = graphics::Image::create(share_data.d, graphics::Format_Depth16, Ivec2(bake_wnd_size), 1, 1, graphics::SampleCount_1, graphics::ImageUsageSampled | graphics::ImageUsageAttachment, graphics::MemPropDevice);
			graphics::FramebufferInfo fb_info;
			fb_info.rp = share_data.rp_scene;
			fb_info.views.resize(2);
			fb_info.views[0] = graphics::Imageview::get(img_col);
			fb_info.views[1] = graphics::Imageview::get(img_dep);
			auto fb = graphics::Framebuffer::get(share_data.d, fb_info);
			auto img_sum = graphics::Image::create(share_data.d, graphics::Format_R32G32B32A32_SFLOAT, Ivec2(1), 1, 1, graphics::SampleCount_1, graphics::ImageUsageStorage | graphics::ImageUsageTransferSrc, graphics::MemPropDevice);

			auto total = 0;
			for (auto &m : ms)
				total += m.bk_units.size();

			memcpy(matrix_buf->mapped, &bake_window_proj, sizeof(Mat4));
			auto hf_bake_wnd_size = bake_wnd_size / 2;
			auto wnd_xy = bake_wnd_size * bake_wnd_size;
			auto img_xy = bk_img->size.x * bk_img->size.y;
			auto stag = new Vec4[img_xy];
			auto pixels = new Vec4[wnd_xy];

			for (auto i_pass = 0; i_pass < pass; i_pass++)
			{
				printf("pass: %d/%d\n", i_pass + 1, pass);

				memset(stag, 0, img_xy * sizeof(Vec4));

				auto i = 0;
				for (auto &m : ms)
				{
					for (auto &u : m.bk_units)
					{
						auto col = Vec3(0.f);

						// front
						{
							auto view = get_view_mat(u.pos, u.pos + u.normal, u.up);
							memcpy((char*)matrix_buf->mapped + sizeof(Mat4), &view, sizeof(Mat4));
							matrix_buf->flush();

							cb->begin();
							cb->begin_renderpass(share_data.rp_scene, fb, clear_values);

							auto vp = Rect(Vec2(0.f), Vec2(bake_wnd_size));
							cb->set_viewport(vp);
							cb->set_scissor(vp);

							draw_scene(cb, Vec2(1.f));

							cb->end_renderpass();
							cb->end();
							share_data.d->gq->submit(cb, nullptr, nullptr);
							share_data.d->gq->wait_idle();

							img_col->get_pixels(pixels);
							for (auto y = 0; y < bake_wnd_size; y++)
							{
								for (auto x = 0; x < bake_wnd_size; x++)
								{
									auto idx = y * bake_wnd_size + x;
									auto &p = pixels[idx];
									col += Vec3(p) * share_data.bk_fix_center[idx];
								}
							}

							//img_col->save_png((L"d:/baked/" + std::to_wstring(i) + L"_c.png").c_str());
						}

						// left
						{
							auto view = get_view_mat(u.pos, u.pos - cross(u.normal, u.up), u.up);
							memcpy((char*)matrix_buf->mapped + sizeof(Mat4), &view, sizeof(Mat4));
							matrix_buf->flush();

							cb->begin();
							cb->begin_renderpass(share_data.rp_scene, fb, clear_values);

							cb->set_viewport(Rect(Vec2(0.f), Vec2(bake_wnd_size)));
							cb->set_scissor(Rect(Vec2(hf_bake_wnd_size, 0.f), Vec2(bake_wnd_size)));

							draw_scene(cb, Vec2(1.f));

							cb->end_renderpass();
							cb->end();
							share_data.d->gq->submit(cb, nullptr, nullptr);
							share_data.d->gq->wait_idle();

							img_col->get_pixels(pixels);
							for (auto y = 0; y < bake_wnd_size; y++)
							{
								for (auto x = 0; x < hf_bake_wnd_size; x++)
								{
									auto &p = pixels[y * bake_wnd_size + x + hf_bake_wnd_size];
									col += Vec3(to_f32(p.x), to_f32(p.y), to_f32(p.z)) * share_data.bk_fix_left[y * hf_bake_wnd_size + x];
								}
							}

							//img_col->save_png((L"d:/baked/" + std::to_wstring(i) + L"_l.png").c_str());
						}

						// right
						{
							auto view = get_view_mat(u.pos, u.pos + cross(u.normal, u.up), u.up);
							memcpy((char*)matrix_buf->mapped + sizeof(Mat4), &view, sizeof(Mat4));
							matrix_buf->flush();

							cb->begin();
							cb->begin_renderpass(share_data.rp_scene, fb, clear_values);

							cb->set_viewport(Rect(Vec2(0.f), Vec2(bake_wnd_size)));
							cb->set_scissor(Rect(Vec2(0.f), Vec2(hf_bake_wnd_size, bake_wnd_size)));

							draw_scene(cb, Vec2(1.f));

							cb->end_renderpass();
							cb->end();
							share_data.d->gq->submit(cb, nullptr, nullptr);
							share_data.d->gq->wait_idle();

							img_col->get_pixels(pixels);
							for (auto y = 0; y < bake_wnd_size; y++)
							{
								for (auto x = 0; x < hf_bake_wnd_size; x++)
								{
									auto &p = pixels[y * bake_wnd_size + x];
									col += Vec3(p) * share_data.bk_fix_right[y * hf_bake_wnd_size + x];
								}
							}

							//img_col->save_png((L"d:/baked/" + std::to_wstring(i) + L"_r.png").c_str());
						}

						// top
						{
							auto view = get_view_mat(u.pos, u.pos + u.up, -u.normal);
							memcpy((char*)matrix_buf->mapped + sizeof(Mat4), &view, sizeof(Mat4));
							matrix_buf->flush();

							cb->begin();
							cb->begin_renderpass(share_data.rp_scene, fb, clear_values);

							cb->set_viewport(Rect(Vec2(0.f), Vec2(bake_wnd_size)));
							cb->set_scissor(Rect(Vec2(0.f, hf_bake_wnd_size), Vec2(bake_wnd_size)));

							draw_scene(cb, Vec2(1.f));

							cb->end_renderpass();
							cb->end();
							share_data.d->gq->submit(cb, nullptr, nullptr);
							share_data.d->gq->wait_idle();

							img_col->get_pixels(pixels);
							for (auto y = 0; y < hf_bake_wnd_size; y++)
							{
								for (auto x = 0; x < bake_wnd_size; x++)
								{
									auto &p = pixels[(y + hf_bake_wnd_size) * bake_wnd_size + x];
									col += Vec3(p) * share_data.bk_fix_top[y * bake_wnd_size + x];
								}
							}

							//img_col->save_png((L"d:/baked/" + std::to_wstring(i) + L"_t.png").c_str());
						}

						// bottom
						{
							auto view = get_view_mat(u.pos, u.pos - u.up, u.normal);
							memcpy((char*)matrix_buf->mapped + sizeof(Mat4), &view, sizeof(Mat4));
							matrix_buf->flush();

							cb->begin();
							cb->begin_renderpass(share_data.rp_scene, fb, clear_values);

							cb->set_viewport(Rect(Vec2(0.f), Vec2(bake_wnd_size)));
							cb->set_scissor(Rect(Vec2(0.f), Vec2(bake_wnd_size, hf_bake_wnd_size)));

							draw_scene(cb, Vec2(1.f));

							cb->end_renderpass();
							cb->end();
							share_data.d->gq->submit(cb, nullptr, nullptr);
							share_data.d->gq->wait_idle();

							img_col->get_pixels(pixels);
							for (auto y = 0; y < hf_bake_wnd_size; y++)
							{
								for (auto x = 0; x < bake_wnd_size; x++)
								{
									auto &p = pixels[y * bake_wnd_size + x];
									col += Vec3(p) * share_data.bk_fix_bottom[y * bake_wnd_size + x];
								}
							}

							//img_col->save_png((L"d:/baked/" + std::to_wstring(i) + L"_b.png").c_str());
						}

						stag[u.pixel_coord.y * bk_img->size.x + u.pixel_coord.x] = Vec4(col, 1.f);

						printf("%d/%d\r", i + 1, total);
						i++;
					}
				}
				printf("\n");

				bk_img->set_pixels(stag);
			}
			delete[]stag;
			delete[]pixels;

			graphics::Framebuffer::release(fb);
			graphics::Image::destroy(img_col);
			graphics::Image::destroy(img_dep);
			graphics::Commandbuffer::destroy(cb);
		}
	}
}

