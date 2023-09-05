#pragma once

#include "font.h"

#ifdef USE_IMGUI
#include <imgui.h>
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui_internal.h>
#include "../imgui_extension.h"
#include <misc/cpp/imgui_stdlib.h>
#if USE_IM_GUIZMO
#include <ImGuizmo.h>
#endif
#if USE_IMGUI_NODE_EDITOR
#include <imgui_node_editor.h>
#include <imgui_node_editor_internal.h>
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
	FLAME_GRAPHICS_API void OpenYesNoDialog(const std::string title, const std::string& prompt, const std::function<void(bool)>& callback);
	FLAME_GRAPHICS_API void OpenInputDialog(const std::string title, const std::string& prompt, const std::function<void(bool, const std::string&)>& callback, const std::string default_value = "", bool archive = false);
	FLAME_GRAPHICS_API void OpenSelectDialog(const std::string title, const std::string& prompt, const std::vector<std::string>& options, const std::function<void(int)>& callback);
	FLAME_GRAPHICS_API void OpenFileDialog(const std::string title, const std::function<void(bool, const std::filesystem::path&)>& callback, const std::filesystem::path& start_dir = L"");
}

namespace flame
{
	namespace graphics
	{
		FLAME_GRAPHICS_API extern Listeners<void()> gui_callbacks;
		FLAME_GRAPHICS_API void gui_initialize();
		FLAME_GRAPHICS_API void* gui_native_handle();
		FLAME_GRAPHICS_API ImagePtr gui_font_image();
		FLAME_GRAPHICS_API void gui_set_clear(bool clear, const vec4& color);
		FLAME_GRAPHICS_API extern Listeners<void(CursorType& cursor)> gui_cursor_callbacks;
		FLAME_GRAPHICS_API void gui_frame();
		FLAME_GRAPHICS_API void gui_clear_inputs();
		FLAME_GRAPHICS_API bool gui_want_mouse();
		FLAME_GRAPHICS_API bool gui_want_keyboard();
		// lod: 0 - 16 pts, 1 - 32 pts, 2 - 64 pts, 3 - 128 pts
		FLAME_GRAPHICS_API const std::vector<vec2>& get_circle_points(uint lod);

		struct GuiCircleDrawer
		{
			const std::vector<vec2>& pts;

			GuiCircleDrawer(uint lod) :
				pts(get_circle_points(lod))
			{
			}

			vec2 get_pt(int idx)
			{
				idx = idx % pts.size();
				if (idx < 0) idx += pts.size();
				return pts[idx];
			}

			int get_idx(float ang)
			{
				return ang / 360.f * pts.size();
			}
		};

		inline void gui_set_current()
		{
#ifdef USE_IMGUI
			ImGui::SetCurrentContext((ImGuiContext*)gui_native_handle());
#endif
		}

		FLAME_GRAPHICS_API wchar_t font_icon(uint name);
		FLAME_GRAPHICS_API std::string font_icon_str(uint name);
		FLAME_GRAPHICS_API Image* get_icon(const std::filesystem::path& path /*image file or .ext*/, uint desired_size = 64);
		FLAME_GRAPHICS_API void release_icon(Image* image);
	}
}
