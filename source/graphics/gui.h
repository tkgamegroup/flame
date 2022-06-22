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

namespace ImGui
{
	struct Dialog
	{
		std::string title;

		virtual ~Dialog() {}
		FLAME_GRAPHICS_API static void open(Dialog* dialog);
		FLAME_GRAPHICS_API void close();
		virtual void draw() = 0;
	};

	FLAME_GRAPHICS_API void OpenMessageDialog(const std::string title, const std::string& message);
	FLAME_GRAPHICS_API void OpenYesNoDialog(const std::string title, const std::function<void(bool)>& callback);
	FLAME_GRAPHICS_API void OpenInputDialog(const std::string title, const std::function<void(bool, const std::string&)>& callback, bool archive = false);
	FLAME_GRAPHICS_API void OpenFileDialog(const std::string title, const std::function<void(bool, const std::filesystem::path&)>& callback, const std::filesystem::path& start_dir = L"");
}

namespace flame
{
	namespace graphics
	{
		FLAME_GRAPHICS_API extern Listeners<void()> gui_callbacks;
		FLAME_GRAPHICS_API void gui_initialize();
		FLAME_GRAPHICS_API void* gui_native_handle();
		// lod: 0 - 16 pts, 1 - 32 pts, 2 - 64 pts, 3 - 128 pts
		FLAME_GRAPHICS_API std::vector<vec2> get_circle_points(uint lod);

		inline void gui_set_current()
		{
			ImGui::SetCurrentContext((ImGuiContext*)gui_native_handle());
		}

		FLAME_GRAPHICS_API Image* get_icon(const std::filesystem::path& path, uint desired_size = 64);
		FLAME_GRAPHICS_API void release_icon(const std::filesystem::path& path);
	}
}
