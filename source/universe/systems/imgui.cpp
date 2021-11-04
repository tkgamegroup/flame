#include "../../graphics/buffer.h"
#include "../../graphics/buffer_ext.h"
#include "../../graphics/image.h"
#include "../../graphics/renderpass.h"
#include "../../graphics/shader.h"
#include "../../graphics/command.h"
#include "../../graphics/swapchain.h"
#include "../../graphics/window.h"
#include "../world_private.h"
#include "../components/imgui_private.h"
#include "imgui_private.h"

namespace flame
{
	using namespace graphics;

	struct RenderData
	{
		UniPtr<Image>				img_font;
		SequentialBuffer<Vertex>	buf_vtx;
		SequentialBuffer<ushort>	buf_idx;
		UniPtr<DescriptorSet>		ds;
		Pipeline* pl;
	};

	sImguiPrivate::sImguiPrivate()
	{
		_rd = new RenderData;
	}

	sImguiPrivate::~sImguiPrivate()
	{
		delete _rd;
	}

	void sImguiPrivate::setup(Window* _window)
	{
		fassert(!window);
		window = _window;

		auto set_targets_from_swapchain = [](Capture& c, const uvec2& size) {
			auto thiz = c.thiz<sImguiPrivate>();
			std::vector<ImageView*> views;

			auto swapchain = thiz->window->get_swapchain();
			views.resize(swapchain->get_images_count());
			for (auto i = 0; i < views.size(); i++)
				views[i] = swapchain->get_image(i)->get_view();

			thiz->set_targets(views);
		};

		set_targets_from_swapchain(Capture().set_thiz(this), uvec2(0));

		auto native_window = window->get_native();

		native_window->add_resize_listener(set_targets_from_swapchain, Capture().set_thiz(this));

		window->add_renderer([](Capture& c, uint img_idx, CommandBuffer* commandbuffer) {
			c.thiz<sImguiPrivate>()->render(img_idx, commandbuffer);
		}, Capture().set_thiz(this));

		native_window->add_mouse_left_down_listener([](Capture& c, const ivec2& pos) {
			auto thiz = c.thiz<sDispatcherPrivate>();
			ImGuiIO& io = ImGui::GetIO();
			io.MouseDown[0] = true;
		}, Capture().set_thiz(this));

		native_window->add_mouse_left_up_listener([](Capture& c, const ivec2& pos) {
			auto thiz = c.thiz<sDispatcherPrivate>();
			ImGuiIO& io = ImGui::GetIO();
			io.MouseDown[0] = false;
		}, Capture().set_thiz(this));

		native_window->add_mouse_move_listener([](Capture& c, const ivec2& pos) {
			auto thiz = c.thiz<sImguiPrivate>();
			ImGuiIO& io = ImGui::GetIO();
			io.MousePos = ImVec2(pos.x, pos.y);
		}, Capture().set_thiz(this));

		native_window->add_mouse_scroll_listener([](Capture& c, int scroll) {
			auto thiz = c.thiz<sImguiPrivate>();
			ImGuiIO& io = ImGui::GetIO();
			io.MouseWheel = scroll;
		}, Capture().set_thiz(this));
	}

	void sImguiPrivate::on_added()
	{
		rp_bgra8 = Renderpass::get(nullptr, L"bgra8.rp");
		rp_bgra8l = Renderpass::get(nullptr, L"bgra8l.rp");
		rp_bgra8c = Renderpass::get(nullptr, L"bgra8c.rp");

		auto& rd = *_rd;

		rd.buf_vtx.create(BufferUsageVertex, 360000);
		rd.buf_idx.create(BufferUsageIndex, 240000);
		rd.ds.reset(DescriptorSet::create(nullptr, DescriptorSetLayout::get(nullptr, L"imgui/imgui.dsl")));
		rd.pl = Pipeline::get(nullptr, L"imgui/imgui.pipeline");

#if USE_IMGUI
		IMGUI_CHECKVERSION();

		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

		ImGui::StyleColorsDark();

		ImGuiStyle& style = ImGui::GetStyle();
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		
		fassert(!io.BackendPlatformUserData);

		io.BackendPlatformName = "imgui_impl_flame";
		io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
		io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;
		io.BackendFlags |= ImGuiBackendFlags_PlatformHasViewports;

		io.KeyMap[ImGuiKey_Space] = Keyboard_Space;

		{
			uchar* img_data;
			int img_w, img_h;
			io.Fonts->GetTexDataAsAlpha8(&img_data, &img_w, &img_h);

			StagingBuffer stag(nullptr, image_pitch(img_w) * img_h, img_data);
			InstanceCB cb(nullptr);

			rd.img_font.reset(Image::create(nullptr, Format_R8_UNORM, uvec2(img_w, img_h), 1, 1, SampleCount_1, ImageUsageSampled | ImageUsageTransferDst));
			cb->image_barrier(rd.img_font.get(), {}, ImageLayoutUndefined, ImageLayoutTransferDst);
			BufferImageCopy cpy;
			cpy.img_ext = uvec2(img_w, img_h);
			cb->copy_buffer_to_image(stag.get(), rd.img_font.get(), 1, &cpy);
			cb->image_barrier(rd.img_font.get(), {}, ImageLayoutTransferDst, ImageLayoutShaderReadOnly);
		}

		rd.ds->set_image(0, 0, rd.img_font->get_view({}, { SwizzleOne, SwizzleOne, SwizzleOne, SwizzleR }), Sampler::get(nullptr, FilterNearest, FilterNearest, false, AddressClampToEdge));
		rd.ds->update();
#endif
	}

	void sImguiPrivate::set_targets(const std::vector<ImageView*>& views)
	{
		img_tars.resize(views.size());
		for (auto i = 0; i < views.size(); i++)
			img_tars[i] = views[i]->get_image();

		fb_tars.clear();
		fb_tars.resize(views.size());
		for (auto i = 0; i < views.size(); i++)
			fb_tars[i].reset(Framebuffer::create(rp_bgra8, 1, &views[i]));

		if (views.empty())
			return;

		tar_sz = views[0]->get_image()->get_size();

#if USE_IMGUI
		ImGuiIO& io = ImGui::GetIO();
		io.DisplaySize = ImVec2(tar_sz.x, tar_sz.y);
#endif
	}

	void sImguiPrivate::render(uint tar_idx, CommandBuffer* cb)
	{
#if USE_IMGUI
		ImGui::Render();
#endif

		auto& rd = *_rd;

		auto draw_data = ImGui::GetDrawData();
		int fb_width = (int)(draw_data->DisplaySize.x * draw_data->FramebufferScale.x);
		int fb_height = (int)(draw_data->DisplaySize.y * draw_data->FramebufferScale.y);
		if (fb_width > 0 || fb_height > 0)
		{
			if (draw_data->TotalVtxCount > 0)
			{
				auto pvtx = rd.buf_vtx.stag(draw_data->TotalVtxCount);
				auto pidx = rd.buf_idx.stag(draw_data->TotalIdxCount);

				for (int n = 0; n < draw_data->CmdListsCount; n++)
				{
					const ImDrawList* cmd_list = draw_data->CmdLists[n];
					memcpy(pvtx, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
					memcpy(pidx, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
					pvtx += cmd_list->VtxBuffer.Size;
					pidx += cmd_list->IdxBuffer.Size;
				}

				rd.buf_vtx.upload(cb);
				rd.buf_idx.upload(cb);
				rd.buf_vtx.stag_num = 0;
				rd.buf_idx.stag_num = 0;
			}

			if (clear_color.a == 0.f)
				cb->begin_renderpass(rp_bgra8l, fb_tars[tar_idx].get());
			else
				cb->begin_renderpass(rp_bgra8c, fb_tars[tar_idx].get(), &clear_color);

			cb->set_viewport(Rect(0, 0, fb_width, fb_height));

			cb->bind_pipeline(rd.pl);
			cb->bind_vertex_buffer(rd.buf_vtx.buf.get(), 0);
			cb->bind_index_buffer(rd.buf_idx.buf.get(), IndiceTypeUshort);
			cb->bind_descriptor_set(0, rd.ds.get());
			auto scale = 2.f / vec2(draw_data->DisplaySize.x, draw_data->DisplaySize.y);
			cb->push_constant_t(vec4(scale,
				-1.f - draw_data->DisplayPos.x * scale[0],
				-1.f - draw_data->DisplayPos.y * scale[1]));

			ImVec2 clip_off = draw_data->DisplayPos;
			ImVec2 clip_scale = draw_data->FramebufferScale;

			int global_vtx_offset = 0;
			int global_idx_offset = 0;
			Image* last_tex = nullptr;
			for (int n = 0; n < draw_data->CmdListsCount; n++)
			{
				const ImDrawList* cmd_list = draw_data->CmdLists[n];
				for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
				{
					const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];

					ImVec2 clip_min((pcmd->ClipRect.x - clip_off.x) * clip_scale.x, (pcmd->ClipRect.y - clip_off.y) * clip_scale.y);
					ImVec2 clip_max((pcmd->ClipRect.z - clip_off.x) * clip_scale.x, (pcmd->ClipRect.w - clip_off.y) * clip_scale.y);

					if (clip_min.x < 0.0f) { clip_min.x = 0.0f; }
					if (clip_min.y < 0.0f) { clip_min.y = 0.0f; }
					if (clip_max.x > fb_width) { clip_max.x = (float)fb_width; }
					if (clip_max.y > fb_height) { clip_max.y = (float)fb_height; }
					if (clip_max.x < clip_min.x || clip_max.y < clip_min.y)
						continue;

					if (last_tex != pcmd->TextureId)
					{
						auto tex = (Image*)pcmd->TextureId;
						cb->bind_descriptor_set(0, tex ? tex->get_shader_read_src() : rd.ds.get());
						last_tex = tex;
					}

					cb->set_scissor(Rect(clip_min.x, clip_min.y, clip_max.x, clip_max.y));
					cb->draw_indexed(pcmd->ElemCount, pcmd->IdxOffset + global_idx_offset, pcmd->VtxOffset + global_vtx_offset, 1, 0);
				}
				global_idx_offset += cmd_list->IdxBuffer.Size;
				global_vtx_offset += cmd_list->VtxBuffer.Size;
			}

			cb->end_renderpass();
		}
	}

	void sImguiPrivate::update()
	{
#if USE_IMGUI
		ImGui::NewFrame();

		auto ctx = ImGui::GetCurrentContext();

		std::function<void(EntityPrivate*)> draw;
		draw = [&](EntityPrivate* e) {
			auto c = e->get_component_i<cImguiPrivate>(0);
			if (c && c->callback)
			{
				c->callback->c._current = ctx;
				c->callback->call();
			}
			for (auto& c : e->children)
				draw(c.get());
		};
		draw(world->first_imgui);

		auto& io = ImGui::GetIO();
		mouse_consumed = io.WantCaptureMouse;
		keyboard_consumed = io.WantCaptureKeyboard;
#endif

		if (window)
			window->mark_dirty();
	}

	sImgui* sImgui::create(void* parms)
	{
		return new sImguiPrivate();
	}
}
