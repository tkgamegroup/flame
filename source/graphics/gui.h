#pragma once

#include "font.h"

#ifdef USE_IMGUI
#include <imgui.h>
#include <imgui_internal.h>
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
	FLAME_GRAPHICS_API void OpenYesNoDialog(const std::string title, const std::string& prompt, const std::function<void(bool)>& callback);
	FLAME_GRAPHICS_API void OpenInputDialog(const std::string title, const std::string& prompt, const std::function<void(bool, const std::string&)>& callback, const std::string default_value = "", bool archive = false);
	FLAME_GRAPHICS_API void OpenFileDialog(const std::string title, const std::function<void(bool, const std::filesystem::path&)>& callback, const std::filesystem::path& start_dir = L"");
}

namespace flame
{
	namespace graphics
	{
		FLAME_GRAPHICS_API extern Listeners<void()> gui_callbacks;
		FLAME_GRAPHICS_API void gui_initialize();
		FLAME_GRAPHICS_API void* gui_native_handle();
		FLAME_GRAPHICS_API void gui_set_clear(bool clear, const vec4& color);
		FLAME_GRAPHICS_API extern Listeners<CursorType(CursorType cursor)> gui_cursor_callbacks;
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

		FLAME_GRAPHICS_API Image* get_icon(const std::filesystem::path& path, uint desired_size = 64);
		FLAME_GRAPHICS_API void release_icon(const std::filesystem::path& path);

		struct GuiView
		{
			std::string name;
			bool opened = false;
			bool auto_size = false;

			inline GuiView(std::string_view name);
			inline virtual ~GuiView();

			void open()
			{
				if (opened)
					return;
				opened = true;

				graphics::gui_callbacks.add([this]() {
					draw();
					}, (uint)this);
			}

			void close()
			{
				if (!opened)
					return;
				opened = false;

				add_event([this]() {
					graphics::gui_callbacks.remove((uint)this);
				return false;
					});
			}

			void draw()
			{
				auto closed = on_begin();
				on_draw();
				on_end(closed);
			}

			virtual void init() {}
			virtual bool on_begin()
			{
				bool open = true;
#ifdef USE_IMGUI
				ImGui::Begin(name.c_str(), &open);
#endif
				return !open;
			}
			virtual void on_end(bool closed)
			{
#ifdef USE_IMGUI
				ImGui::End();
#endif

				if (closed)
					close();
			}
			virtual void on_draw() = 0;
		};

		FLAME_GRAPHICS_API extern std::list<GuiView*> gui_views;

		inline GuiView::GuiView(std::string_view name) :
			name(name)
		{
			gui_views.push_back(this);
		}

		inline GuiView::~GuiView()
		{
			std::erase_if(gui_views, [&](const auto& i) {
				return i == this;
				});
		}
	}
}
