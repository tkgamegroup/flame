#pragma once

#include "graphics.h"

#if USE_IMGUI
#include <imgui.h>
#include "../imgui_extension.h"
#include <misc/cpp/imgui_stdlib.h>
	#if USE_IM_GUIZMO
	#include <ImGuizmo.h>
	#endif
#endif

namespace flame
{
	namespace graphics
	{
		FLAME_GRAPHICS_API extern Listeners<void()> gui_callbacks;
		FLAME_GRAPHICS_API void gui_initialize();
		FLAME_GRAPHICS_API void* gui_native_handle();

		inline void gui_set_current()
		{
			ImGui::SetCurrentContext((ImGuiContext*)gui_native_handle());
		}
	}
}

namespace ImGui
{

}
