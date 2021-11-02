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

		window->get_native()->add_resize_listener(set_targets_from_swapchain, Capture().set_thiz(this));

		window->add_renderer([](Capture& c, uint img_idx, CommandBuffer* commandbuffer) {
			c.thiz<sImguiPrivate>()->render(img_idx, commandbuffer);
		}, Capture().set_thiz(this));
	}

	struct BackendData
	{

	};

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
		
		IM_ASSERT(io.BackendPlatformUserData == NULL && "Already initialized a platform backend!");

		auto bd = new BackendData;
		io.BackendPlatformUserData = bd;
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

		cb->begin_renderpass(rp_bgra8l, fb_tars[tar_idx].get());

		cb->bind_pipeline(rd.pl);
		cb->bind_vertex_buffer(rd.buf_vtx.buf.get(), 0);
		cb->bind_index_buffer(rd.buf_idx.buf.get(), IndiceTypeUshort);
		cb->bind_descriptor_set(0, rd.ds.get());

		cb->end_renderpass();
	}

	void sImguiPrivate::update()
	{
#if USE_IMGUI
		ImGui::NewFrame();

		std::function<void(EntityPrivate*)> draw;
		draw = [&](EntityPrivate* e) {
			auto c = e->get_component_t<cImguiPrivate>();
			if (c && c->callback)
				c->callback->call();
			for (auto& c : e->children)
				draw(c.get());
		};
		draw(world->root.get());
#endif

		if (window)
			window->mark_dirty();
	}

	sImgui* sImgui::create(void* parms)
	{
		return new sImguiPrivate();
	}
}
