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
		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
		//io.ConfigViewportsNoAutoMerge = true;
		//io.ConfigViewportsNoTaskBarIcon = true;

		// Setup Dear ImGui style
		ImGui::StyleColorsDark();
		//ImGui::StyleColorsClassic();

		// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}
		
		IM_ASSERT(io.BackendPlatformUserData == NULL && "Already initialized a platform backend!");

		// Setup backend capabilities flags
		auto bd = new BackendData;
		io.BackendPlatformUserData = bd;
		io.BackendPlatformName = "imgui_impl_flame";
		io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;         // We can honor GetMouseCursor() values (optional)
		io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;          // We can honor io.WantSetMousePos requests (optional, rarely used)
		io.BackendFlags |= ImGuiBackendFlags_PlatformHasViewports;    // We can create multi-viewports on the Platform side (optional)
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
