#include "../world_private.h"
#include "../components/imgui_private.h"
#include "imgui_private.h"

namespace flame
{
	void sImguiPrivate::setup(graphics::Window* _window)
	{
		fassert(!window);

		window = _window;
	}

	struct BackendData
	{

	};

	void sImguiPrivate::on_added()
	{
		IMGUI_CHECKVERSION();

		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
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
	}

	void sImguiPrivate::render(uint tar_idx, graphics::CommandBuffer* cb)
	{

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

	}

	sImgui* sImgui::create(void* parms)
	{
		return new sImguiPrivate();
	}
}
