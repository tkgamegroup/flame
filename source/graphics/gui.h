#pragma once

#include "font.h"

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

		FLAME_GRAPHICS_API ImagePtr get_icon(const std::filesystem::path& path, uint desired_size = 64);
		FLAME_GRAPHICS_API void release_icon(const std::filesystem::path& path);
	}
}

namespace ImGui
{
	FLAME_GRAPHICS_API void OpenMessageDialog(const std::string title, const std::string& message);
	FLAME_GRAPHICS_API void OpenYesNoDialog(const std::string title, const std::string& message);
	FLAME_GRAPHICS_API void OpenInputDialog(const std::string title, const std::string& message);
	FLAME_GRAPHICS_API void OpenFileDialog(const std::string title, const std::function<void(bool, const std::filesystem::path&)>& callback);
}
