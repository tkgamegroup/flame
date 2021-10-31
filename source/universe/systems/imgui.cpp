#include "../world_private.h"
#include "../components/imgui_private.h"
#include "imgui_private.h"

namespace flame
{
	void sImguiPrivate::on_added()
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
