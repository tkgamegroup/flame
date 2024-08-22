#include "../json.h"
#include "../foundation/window.h"
#include "../foundation/system.h"
#include "gui.h"
#include "explorer.h"
#include "window_private.h"
#include "renderpass_private.h"
#include "font_private.h"
#include "shader_private.h"
#include "command_private.h"
#include "window_private.h"
#include "auxiliary.h"

namespace ImGui
{
	using namespace flame;

	std::vector<std::string> peeding_dialogs;
	std::vector<std::unique_ptr<Dialog>> dialogs;

	void Dialog::open(Dialog* dialog)
	{
		dialogs.emplace_back(dialog);
		peeding_dialogs.push_back(dialog->title);
	}

	void Dialog::close()
	{
		assert(!dialogs.empty());
#if USE_IMGUI
		ImGui::CloseCurrentPopup();
#endif
		add_event([this]() {
			graphics::Queue::get()->wait_idle();
			std::erase_if(dialogs, [&](const auto& i) {
				return i.get() == this;
			});
			return false;
		});
	}

	struct MessageDialog : Dialog
	{
		std::string message;

		void draw() override
		{
#if USE_IMGUI
			if (ImGui::BeginPopupModal(title.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoDocking))
			{
				ImGui::TextUnformatted(message.c_str());
				if (ImGui::Button("OK"))
					close();
				ImGui::EndPopup();
			}
#endif
		}
	};

	struct YesNoDialog : Dialog
	{
		std::string prompt;
		std::function<void(bool)> callback;

		void draw() override
		{
#if USE_IMGUI
			if (ImGui::BeginPopupModal(title.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoDocking))
			{
				ImGui::TextUnformatted(prompt.c_str());
				if (ImGui::Button("Yes"))
				{
					if (callback)
						callback(true);
					close();
				}
				ImGui::SameLine();
				if (ImGui::Button("No"))
				{
					if (callback)
						callback(false);
					close();
				}
				ImGui::EndPopup();
			}
#endif
		}
	};

	struct InputDialog : Dialog
	{
		std::string prompt;
		std::string text;
		std::function<void(bool, const std::string&)> callback;
		std::vector<std::string> history;
		bool archive = false;

		~InputDialog() override
		{
			if (archive)
			{
				std::ofstream file(L"input_dialog_" + wstr(sh(title.c_str())) + L".txt");
				for (auto& i : history)
					file << i << std::endl;
				file.close();
			}
		}

		void draw() override
		{
#if USE_IMGUI
			if (ImGui::BeginPopupModal(title.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoDocking))
			{
				ImGui::TextUnformatted(prompt.c_str());
				auto ok = false; 
				if (ImGui::InputText("##text", &text, ImGuiInputTextFlags_EnterReturnsTrue))
					ok = true;
				if (archive)
				{
					ImGui::SameLine();
					if (ImGui::BeginCombo("##combo", nullptr, ImGuiComboFlags_NoPreview | ImGuiComboFlags_PopupAlignLeft))
					{
						for (auto& i : history)
						{
							if (ImGui::Selectable(i.c_str()))
								text = i;
						}
						ImGui::EndCombo();
					}
				}
				if (ImGui::Button("OK"))
					ok = true;
				if (ok)
				{
					if (archive)
					{
						if (std::find(history.begin(), history.end(), text) == history.end())
							history.push_back(text);
					}
					if (callback)
						callback(true, text);
					close();
				}
				ImGui::SameLine();
				if (ImGui::Button("Cancel"))
				{
					if (callback)
						callback(false, "");
					close();
				}
				ImGui::EndPopup();
			}
#endif
		}
	};

	struct SelectDialog : Dialog
	{
		std::string prompt;
		std::vector<std::string> options;
		int index = -1;
		std::function<void(int)> callback;

		void draw() override
		{
#if USE_IMGUI
			if (ImGui::BeginPopupModal(title.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoDocking))
			{
				if (ImGui::BeginCombo(prompt.c_str(), index == -1 ? nullptr : options[index].c_str()))
				{
					for (auto i = 0; i < options.size(); i++)
					{
						if (ImGui::Selectable(options[i].c_str()))
							index = i;
					}
					ImGui::EndCombo();
				}
				if (ImGui::Button("OK"))
				{
					if (callback)
						callback(index);
					close();
				}
				ImGui::SameLine();
				if (ImGui::Button("Cancel"))
				{
					if (callback)
						callback(-1);
					close();
				}
				ImGui::EndPopup();
			}
#endif
		}
	};

	struct FileDialog : Dialog
	{
		graphics::Explorer explorer;
		std::string name;
		std::function<void(bool, const std::filesystem::path&)> callback;

		void draw() override
		{
#if USE_IMGUI
			ImGui::SetNextWindowSize(vec2(800.f, 600.f), ImGuiCond_FirstUseEver);
			if (ImGui::BeginPopupModal(title.c_str(), nullptr, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoDocking))
			{
				ImGui::BeginChild("explorer", ImVec2(0, -ImGui::GetFontSize() * 2.f - ImGui::GetStyle().ItemSpacing.y * 5));
				explorer.draw();
				ImGui::EndChild();

				ImGui::InputText("Name", &name);
				if (ImGui::Button("OK"))
				{
					if (callback)
						callback(true, explorer.opened_folder ? explorer.opened_folder->path / name : name);
					close();
				}
				ImGui::SameLine();
				if (ImGui::Button("Cancel"))
				{
					if (callback)
						callback(false, L"");
					close();
				}
				ImGui::EndPopup();
			}
#endif
		}
	};

	void OpenMessageDialog(const std::string title, const std::string& message)
	{
		auto dialog = new MessageDialog;
		dialog->title = title;
		dialog->message = message;
		Dialog::open(dialog);
	}

	void OpenYesNoDialog(const std::string title, const std::string& prompt, const std::function<void(bool)>& callback)
	{
		auto dialog = new YesNoDialog;
		dialog->title = title;
		dialog->prompt = prompt;
		dialog->callback = callback;
		Dialog::open(dialog);
	}

	void OpenInputDialog(const std::string title, const std::string& prompt, const std::function<void(bool, const std::string&)>& callback, const std::string default_value, bool archive)
	{
		auto dialog = new InputDialog;
		dialog->title = title;
		dialog->prompt = prompt;
		dialog->text = default_value;
		dialog->callback = callback;
		dialog->archive = archive;
		if (archive)
		{
			std::ifstream file(L"input_dialog_" + wstr(sh(title.c_str())) + L".txt");
			if (file.good())
			{
				std::string line;
				while (!file.eof())
				{
					std::getline(file, line);
					if (!line.empty())
						dialog->history.push_back(line);
				}
				file.close();
			}
		}
		Dialog::open(dialog);
	}

	void OpenSelectDialog(const std::string title, const std::string& prompt, const std::vector<std::string>& options, const std::function<void(int)>& callback)
	{
		auto dialog = new SelectDialog;
		dialog->title = title;
		dialog->prompt = prompt;
		dialog->options = options;
		dialog->callback = callback;
		Dialog::open(dialog);
	}

	void OpenFileDialog(const std::string title, const std::function<void(bool, const std::filesystem::path&)>& callback, const std::filesystem::path& start_dir)
	{
		auto dialog = new FileDialog;
		dialog->title = title;
		dialog->callback = callback;
		dialog->explorer.reset(!start_dir.empty() ? start_dir : L"This Computer");
		dialog->explorer.peeding_open_node = { dialog->explorer.folder_tree.get(), false };
		dialog->explorer.select_callback = [dialog](const std::filesystem::path& path) {
			dialog->name = path.filename().string();
			dialog->explorer.selected_paths = { path };
		};
		Dialog::open(dialog);
	}
}

namespace flame
{
	namespace graphics
	{
		static WindowPtr main_window = nullptr;
		static RenderpassPtr imgui_rp = nullptr;
		static RenderpassPtr imgui_rp_load = nullptr;
		static std::unordered_map<WindowPtr, std::vector<std::unique_ptr<FramebufferT>>> imgui_fbs;
		static std::unique_ptr<ImageT> imgui_img_font;
		static VertexBuffer imgui_buf_vtx;
		static IndexBuffer<ushort> imgui_buf_idx;
		static std::unique_ptr<DescriptorSetT> imgui_ds;
		static GraphicsPipelinePtr imgui_pl;

		Listeners<void()> gui_callbacks;
		Listeners<void(CursorType &cursor)> gui_cursor_callbacks;

		static std::map<std::filesystem::path, std::pair<int, ImagePtr>> icons;

		static void gui_create_fbs(WindowPtr w)
		{
			if (!imgui_rp)
			{
				std::vector<std::string> defines;
				defines.push_back("col_fmt=" + TypeInfo::serialize_t(Swapchain::format));
				imgui_rp = Renderpass::get(L"flame\\shaders\\color.rp", defines);
				defines.push_back("load_op=Load");
				defines.push_back("initia_layout=Attachment");
				imgui_rp_load = Renderpass::get(L"flame\\shaders\\color.rp", defines);
			}
			imgui_fbs[w].clear();
			for (auto& img : w->swapchain->images)
				imgui_fbs[w].emplace_back(Framebuffer::create(imgui_rp, img->get_view()));
		}

		static void gui_attach_window(WindowPtr w)
		{
#if USE_IMGUI
			auto native_window = w->native;
			gui_create_fbs(w);
			native_window->resize_listeners.add([w](const vec2&) {
				gui_create_fbs(w);
			});
			native_window->mouse_listeners.add([](MouseButton btn, bool down) {
				ImGuiIO& io = ImGui::GetIO();
				io.AddMouseButtonEvent(btn, down);
			});
			native_window->mouse_move_listeners.add([native_window](const ivec2& _pos) {
				ImGuiIO& io = ImGui::GetIO();
				auto pos = _pos;
				if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
					pos = native_window->local_to_global(pos);
				io.AddMousePosEvent(pos.x, pos.y);
			});
			native_window->mouse_scroll_listeners.add([](int scroll) {
				ImGuiIO& io = ImGui::GetIO();
				io.AddMouseWheelEvent(0.f, scroll);
			});
			native_window->key_listeners.add([](KeyboardKey key, bool down) {
				ImGuiIO& io = ImGui::GetIO();
				io.KeysDown[key] = down;
				if (key == Keyboard_Ctrl)
					io.KeyCtrl = down;
				if (key == Keyboard_Shift)
					io.KeyShift = down;
				if (key == Keyboard_Alt)
					io.KeyAlt = down;
			});
			native_window->char_listeners.add([](wchar_t ch) {
				ImGuiIO& io = ImGui::GetIO();
				io.AddInputCharacter(ch);
			});
			native_window->focus_listeners.add([](bool v) {
				if (!v)
					gui_clear_inputs();
			});
#endif
		}

		void* gui_native_handle()
		{
#if USE_IMGUI
			return ImGui::GetCurrentContext();
#endif
			return nullptr;
		}

		ImagePtr gui_font_image()
		{
			return imgui_img_font.get();
		}

		static bool clear_fb = false;
		static vec4 clear_col = vec4(0.f);

		void gui_set_clear(bool clear, const vec4& color)
		{
			clear_fb = clear;
			clear_col = color;
		}

		static bool want_mouse = false;
		static bool want_keyboard = false;

		void gui_frame()
		{
#if USE_IMGUI
			auto nw = main_window->native;
			auto sz = nw->cl_rect.size();

			auto& io = ImGui::GetIO();
			io.DeltaTime = delta_time;
			io.DisplaySize = ImVec2(sz);

			ImGui::NewFrame();

			if (sz.x > 0 && sz.y > 0)
			{
				gui_callbacks.call();

				if (!ImGui::peeding_dialogs.empty())
				{
					for (auto& name : ImGui::peeding_dialogs)
						ImGui::OpenPopup(name.c_str());
					ImGui::peeding_dialogs.clear();
				}
				for (auto& d : ImGui::dialogs)
					d->draw();
			}

			want_mouse = io.WantCaptureMouse;
			want_keyboard = io.WantCaptureKeyboard;

			CursorType cursor = CursorNone;
			switch (ImGui::GetMouseCursor())
			{
			case ImGuiMouseCursor_None:
				cursor = CursorNone;
				break;
			case ImGuiMouseCursor_Arrow:
				cursor = CursorArrow;
				break;
			case ImGuiMouseCursor_TextInput:
				cursor = CursorIBeam;
				break;
			case ImGuiMouseCursor_ResizeAll:
				cursor = CursorSizeAll;
				break;
			case ImGuiMouseCursor_ResizeNS:
				cursor = CursorSizeNS;
				break;
			case ImGuiMouseCursor_ResizeEW:
				cursor = CursorSizeWE;
				break;
			case ImGuiMouseCursor_ResizeNESW:
				cursor = CursorSizeNESW;
				break;
			case ImGuiMouseCursor_ResizeNWSE:
				cursor = CursorSizeNWSE;
				break;
			case ImGuiMouseCursor_Hand:
				cursor = CursorHand;
				break;
			case ImGuiMouseCursor_NotAllowed:
				cursor = CursorNo;
				break;
			}
			gui_cursor_callbacks.call<CursorType&>(cursor);
			nw->set_cursor(cursor);

			ImGui::EndFrame();
#endif
		}

		void gui_clear_inputs()
		{
#if USE_IMGUI
			auto& io = ImGui::GetIO();
			for (auto& btn : io.MouseDown)
				btn = false;
			io.MousePos = ImVec2(0, 0);
			io.MouseWheel = 0;
			io.KeyCtrl = false;
			io.KeyShift = false;
			io.KeyAlt = false;
			for (auto& key : io.KeysDown)
				key = false;
#endif
		}

		bool gui_want_mouse()
		{
			return want_mouse;
		}

		bool gui_want_keyboard()
		{
			return want_keyboard;
		}

		void gui_render(CommandBuffer* _cb, bool cleanup)
		{
			if (cleanup)
			{
				imgui_buf_vtx.reset();
				imgui_buf_idx.reset();
				return;
			}

			auto cb = (CommandBufferPtr)_cb;
			auto windows = Window::get_list(); // get window list here so that the newly created window will render next frame

#if USE_IMGUI
			ImGui::Render();
			ImGui::UpdatePlatformWindows();

			cb->begin_debug_label("Gui");

			int global_vtx_offset = 0;
			int global_idx_offset = 0;
			ImagePtr last_tex = nullptr;

			std::vector<ImGuiViewport*> viewports;
			for (auto w : windows)
			{
				if (auto viewport = ImGui::FindViewportByPlatformHandle(w); viewport)
					viewports.push_back(viewport);
			}

			for (auto viewport : viewports)
			{
				auto draw_data = viewport->DrawData;
				int fb_width = (int)(draw_data->DisplaySize.x * draw_data->FramebufferScale.x);
				int fb_height = (int)(draw_data->DisplaySize.y * draw_data->FramebufferScale.y);
				if (fb_width > 0 && fb_height > 0)
				{
					if (draw_data->TotalVtxCount > 0)
					{
						for (int i = 0; i < draw_data->CmdListsCount; i++)
						{
							auto cmd_list = draw_data->CmdLists[i];
							imgui_buf_vtx.add(cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size);
							imgui_buf_idx.add(cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size);
						}
					}

					for (int n = 0; n < draw_data->CmdListsCount; n++)
					{
						const auto cmd_list = draw_data->CmdLists[n];
						for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
						{
							const auto pcmd = &cmd_list->CmdBuffer[cmd_i];
							if (auto tex = (ImagePtr)pcmd->TextureId; tex)
								cb->image_barrier(tex, { 0, tex->n_levels, 0, tex->n_layers }, graphics::ImageLayoutShaderReadOnly);
						}
					}
				}
			}

			imgui_buf_vtx.upload(cb);
			imgui_buf_vtx.reset();
			imgui_buf_idx.upload(cb);
			imgui_buf_idx.reset();

			for (auto viewport : viewports)
			{
				auto w = (WindowPtr)viewport->PlatformUserData;
				auto sc = w->swapchain.get();
				if (sc->images.empty())
					continue;
				auto img_idx = sc->image_index;
				auto curr_img = sc->images[img_idx].get();
				auto curr_fb = imgui_fbs[w][img_idx].get();

				auto draw_data = viewport->DrawData;
				int fb_width = (int)(draw_data->DisplaySize.x * draw_data->FramebufferScale.x);
				int fb_height = (int)(draw_data->DisplaySize.y * draw_data->FramebufferScale.y);
				if (fb_width > 0 && fb_height > 0)
				{
					if (clear_fb)
						cb->begin_renderpass(imgui_rp, curr_fb, &clear_col);
					else
					{
						cb->image_barrier(curr_img, {}, ImageLayoutAttachment);
						cb->begin_renderpass(imgui_rp_load, curr_fb);
					}
					cb->set_viewport(Rect(0, 0, fb_width, fb_height));

					cb->bind_pipeline(imgui_pl);
					cb->bind_vertex_buffer(imgui_buf_vtx.buf.get(), 0, sizeof(ImDrawVert));
					cb->bind_index_buffer(imgui_buf_idx.buf.get(), sizeof(ImDrawIdx) == 2 ? IndiceTypeUshort : IndiceTypeUint);
					cb->bind_descriptor_set(0, imgui_ds.get());
					auto scale = 2.f / vec2(draw_data->DisplaySize.x, draw_data->DisplaySize.y);
					auto translate = vec2(-1.f) - draw_data->DisplayPos * scale;
					vec2 view_range(0.f, 1.f);
					float pc_data[6];
					memcpy(pc_data + 0, &scale, sizeof(vec2));
					memcpy(pc_data + 2, &translate, sizeof(vec2));
					memcpy(pc_data + 4, &view_range, sizeof(vec2));
					cb->push_constant(0, sizeof(pc_data), pc_data);

					ImVec2 clip_off = draw_data->DisplayPos;
					ImVec2 clip_scale = draw_data->FramebufferScale;

					ImGui::ImageViewArguments last_view_args;
					for (int n = 0; n < draw_data->CmdListsCount; n++)
					{
						const auto cmd_list = draw_data->CmdLists[n];
						for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
						{
							const auto pcmd = &cmd_list->CmdBuffer[cmd_i];

							ImVec2 clip_min((pcmd->ClipRect.x - clip_off.x) * clip_scale.x, (pcmd->ClipRect.y - clip_off.y) * clip_scale.y);
							ImVec2 clip_max((pcmd->ClipRect.z - clip_off.x) * clip_scale.x, (pcmd->ClipRect.w - clip_off.y) * clip_scale.y);

							if (clip_min.x < 0.0f) { clip_min.x = 0.0f; }
							if (clip_min.y < 0.0f) { clip_min.y = 0.0f; }
							if (clip_max.x > fb_width) { clip_max.x = (float)fb_width; }
							if (clip_max.y > fb_height) { clip_max.y = (float)fb_height; }
							if (clip_max.x < clip_min.x || clip_max.y < clip_min.y)
								continue;

							ImGui::ImageViewArguments view_args;
							memcpy(&view_args, &pcmd->UserCallbackData, sizeof(view_args));
							if (last_tex != pcmd->TextureId || last_view_args != view_args)
							{
								static auto sp_nearest = Sampler::get(FilterNearest, FilterNearest, false, AddressClampToEdge);

								auto tex = (ImagePtr)pcmd->TextureId;
								if (tex)
								{
									ImageSwizzle swizzle;
									SamplerPtr sampler;
									switch (view_args.swizzle)
									{
									case ImGui::ImageViewR: swizzle = { SwizzleR, SwizzleZero, SwizzleZero, SwizzleOne }; break;
									case ImGui::ImageViewG: swizzle = { SwizzleZero, SwizzleG, SwizzleZero, SwizzleOne }; break;
									case ImGui::ImageViewB: swizzle = { SwizzleZero, SwizzleZero, SwizzleB, SwizzleOne }; break;
									case ImGui::ImageViewA: swizzle = { SwizzleA, SwizzleA, SwizzleA, SwizzleOne }; break;
									case ImGui::ImageViewRGB: swizzle = { SwizzleR, SwizzleG, SwizzleB, SwizzleOne }; break;
									case ImGui::ImageViewRRR: swizzle = { SwizzleR, SwizzleR, SwizzleR, SwizzleOne }; break;
									case ImGui::ImageViewGGG: swizzle = { SwizzleG, SwizzleG, SwizzleG, SwizzleOne }; break;
									case ImGui::ImageViewBBB: swizzle = { SwizzleB, SwizzleB, SwizzleB, SwizzleOne }; break;
									case ImGui::ImageViewAAA: swizzle = { SwizzleA, SwizzleA, SwizzleA, SwizzleOne }; break;
									}
									switch (view_args.sampler)
									{
									case ImGui::ImageViewLinear:
										sampler = nullptr;
										break;
									case ImGui::ImageViewNearest:
										sampler = sp_nearest;
										break;
									}

									cb->bind_descriptor_set(0, tex->get_shader_read_src(view_args.level, view_args.layer, sampler, swizzle));

									if (!(view_args.range_min == 0 && (view_args.range_max == 0 || view_args.range_max == 15360)))
									{
										vec2 range;
										range.x = unpackHalf1x16(view_args.range_min);
										range.y = unpackHalf1x16(view_args.range_max);
										cb->push_constant(sizeof(float) * 4, sizeof(range), &range);
									}
									else
										cb->push_constant_t(vec2(0.f, 1.f), sizeof(float) * 4);
								}
								else
								{
									cb->bind_descriptor_set(0, imgui_ds.get());
									cb->push_constant_t(vec2(0.f, 1.f), sizeof(float) * 4);
								}
								last_tex = tex;
								last_view_args = view_args;
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

			cb->end_debug_label();
#endif
		}

		static std::unordered_map<uint, std::pair<wchar_t, std::string>> font_icons;

#if USE_IMGUI
		static void ImGui_CreateWindow(ImGuiViewport* viewport)
		{
			auto styles = WindowStyleInvisible;
			if (!(viewport->Flags & ImGuiViewportFlags_NoDecoration))
				styles = styles | WindowStyleFrame;
			if (viewport->Flags & ImGuiViewportFlags_TopMost)
				styles = styles | WindowStyleTopmost;
			auto native_window = NativeWindow::create("", (vec2)viewport->Size, styles);
			auto window = graphics::Window::create(native_window);
			viewport->PlatformUserData = window;
			viewport->PlatformHandle = window;
			viewport->PlatformHandleRaw = native_window->get_hwnd();
			viewport->RendererUserData = window;

			gui_attach_window(window);
		}

		static void ImGui_DestroyWindow(ImGuiViewport* viewport)
		{
			auto window = (WindowPtr)viewport->PlatformUserData;
			add_event([window]() {
				window->native->close();
				return false;
			});
			viewport->PlatformUserData = nullptr;
			viewport->PlatformHandle = nullptr;
			viewport->RendererUserData = nullptr;
		}

		static void ImGui_ShowWindow(ImGuiViewport* viewport)
		{
			auto window = (WindowPtr)viewport->PlatformUserData;
			window->native->show(WindowNormal);
		}

		static ImVec2 ImGui_GetWindowPos(ImGuiViewport* viewport)
		{
			auto window = (WindowPtr)viewport->PlatformUserData;
			return window->native->cl_rect.a;
		}

		static void ImGui_SetWindowPos(ImGuiViewport* viewport, ImVec2 pos)
		{
			auto window = (WindowPtr)viewport->PlatformUserData;
			window->native->set_pos((vec2)pos);
		}

		static ImVec2 ImGui_GetWindowSize(ImGuiViewport* viewport)
		{
			auto window = (WindowPtr)viewport->PlatformUserData;
			return (vec2)window->native->size;
		}

		static void ImGui_SetWindowSize(ImGuiViewport* viewport, ImVec2 size)
		{
			auto window = (WindowPtr)viewport->PlatformUserData;
			window->native->set_size((vec2)size);
		}

		static bool ImGui_GetWindowFocus(ImGuiViewport* viewport)
		{
			auto window = (WindowPtr)viewport->PlatformUserData;
			return window->native->focused;
		}

		static void ImGui_SetWindowFocus(ImGuiViewport* viewport)
		{
			auto window = (WindowPtr)viewport->PlatformUserData;
			window->native->set_focus();
		}

		static bool ImGui_GetWindowMinimized(ImGuiViewport* viewport)
		{
			auto window = (WindowPtr)viewport->PlatformUserData;
			return window->native->state == WindowMinimized;
		}

		static void ImGui_SetWindowTitle(ImGuiViewport* viewport, const char* title)
		{
			auto window = (WindowPtr)viewport->PlatformUserData;
			window->native->set_title(title);
		}

		static void ImGui_Renderer_CreateWindow(ImGuiViewport* viewport)
		{
			// already processed in ImGui_CreateWindow
		}

		static void ImGui_Renderer_DestroyWindow(ImGuiViewport* viewport)
		{
			// already processed in ImGui_DestroyWindow
		}

		static void ImGui_Renderer_SetWindowSize(ImGuiViewport* viewport, ImVec2)
		{
			// already processed in Window
		}
#endif

		void gui_initialize()
		{
#if USE_IMGUI
			assert(!windows.empty());
			main_window = windows.front();

			gui_attach_window(main_window);

			imgui_pl = GraphicsPipeline::get(L"flame\\shaders\\imgui.pipeline", { "rp=" + str(imgui_rp) });
			imgui_buf_vtx.create(sizeof(ImDrawVert), 360000);
			imgui_buf_idx.create(240000);
			imgui_ds.reset(DescriptorSet::create(nullptr, imgui_pl->layout->dsls[0]));

			IMGUI_CHECKVERSION();

			ImGui::CreateContext();

			ImGuiStyle& style = ImGui::GetStyle();
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;

			for (int i = 0; i <= ImGuiCol_COUNT; i++)
			{
				ImVec4& col = style.Colors[i];
				float H, S, V;
				ImGui::ColorConvertRGBtoHSV(col.x, col.y, col.z, H, S, V);
				H += 0.1f;
				ImGui::ColorConvertHSVtoRGB(H, S, V, col.x, col.y, col.z);
			}

			ImGuiIO& io = ImGui::GetIO();
			//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
			io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
			io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

			assert(!io.BackendPlatformUserData);

			io.BackendPlatformName = "imgui_impl_flame";
			io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
			io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;
			io.BackendFlags |= ImGuiBackendFlags_PlatformHasViewports;
			io.BackendFlags |= ImGuiBackendFlags_RendererHasViewports;

			io.KeyMap[ImGuiKey_Tab] = Keyboard_Tab;
			io.KeyMap[ImGuiKey_LeftArrow] = Keyboard_Left;
			io.KeyMap[ImGuiKey_RightArrow] = Keyboard_Right;
			io.KeyMap[ImGuiKey_UpArrow] = Keyboard_Up;
			io.KeyMap[ImGuiKey_DownArrow] = Keyboard_Down;
			io.KeyMap[ImGuiKey_PageUp] = Keyboard_PgUp;
			io.KeyMap[ImGuiKey_PageDown] = Keyboard_PgDn;
			io.KeyMap[ImGuiKey_Home] = Keyboard_Home;
			io.KeyMap[ImGuiKey_End] = Keyboard_End;
			io.KeyMap[ImGuiKey_Insert] = Keyboard_Ins;
			io.KeyMap[ImGuiKey_Delete] = Keyboard_Del;
			io.KeyMap[ImGuiKey_Backspace] = Keyboard_Backspace;
			io.KeyMap[ImGuiKey_Space] = Keyboard_Space;
			io.KeyMap[ImGuiKey_Enter] = Keyboard_Enter;
			io.KeyMap[ImGuiKey_Escape] = Keyboard_Esc;
			io.KeyMap[ImGuiKey_A] = Keyboard_A;
			io.KeyMap[ImGuiKey_C] = Keyboard_C;
			io.KeyMap[ImGuiKey_V] = Keyboard_V;
			io.KeyMap[ImGuiKey_X] = Keyboard_X;
			io.KeyMap[ImGuiKey_Y] = Keyboard_Y;
			io.KeyMap[ImGuiKey_Z] = Keyboard_Z;

			ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();
			platform_io.Platform_CreateWindow = ImGui_CreateWindow;
			platform_io.Platform_DestroyWindow = ImGui_DestroyWindow;
			platform_io.Platform_ShowWindow = ImGui_ShowWindow;
			platform_io.Platform_GetWindowPos = ImGui_GetWindowPos;
			platform_io.Platform_SetWindowPos = ImGui_SetWindowPos;
			platform_io.Platform_GetWindowSize = ImGui_GetWindowSize;
			platform_io.Platform_SetWindowSize = ImGui_SetWindowSize;
			platform_io.Platform_GetWindowFocus = ImGui_GetWindowFocus;
			platform_io.Platform_SetWindowFocus = ImGui_SetWindowFocus;
			platform_io.Platform_GetWindowMinimized = ImGui_GetWindowMinimized;
			platform_io.Platform_SetWindowTitle = ImGui_SetWindowTitle;
			platform_io.Renderer_CreateWindow = ImGui_Renderer_CreateWindow;
			platform_io.Renderer_DestroyWindow = ImGui_Renderer_DestroyWindow;
			platform_io.Renderer_SetWindowSize = ImGui_Renderer_SetWindowSize;

			ImGuiViewport* main_viewport = ImGui::GetMainViewport();
			main_viewport->PlatformUserData = main_window;
			main_viewport->PlatformHandle = main_window;
			main_viewport->PlatformHandleRaw = main_window->native;
			main_viewport->RendererUserData = main_window;

			platform_io.Monitors.resize(0);
			for (auto& src : get_monitors())
			{
				ImGuiPlatformMonitor dst;
				dst.MainPos = ImVec2(src.main_pos);
				dst.MainSize = ImVec2(src.main_size);
				dst.WorkPos = ImVec2(src.work_pos);
				dst.WorkSize = ImVec2(src.work_size);
				dst.DpiScale = src.dpi_scale;
				dst.PlatformHandle = src.handle;
				platform_io.Monitors.push_back(dst);
			}

			{
				auto expand_range = [](std::vector<ivec2>& ranges, wchar_t ch) {
					auto range_added = false;
					for (auto& r : ranges)
					{
						if (r.x - 1 == ch)
						{
							r.x = ch;
							range_added = true;
							break;
						}
						if (r.y + 1 == ch)
						{
							r.y = ch;
							range_added = true;
							break;
						}
					}
					if (!range_added)
						ranges.push_back({ ch, ch });
				};

				auto expand_icon_range = [](std::vector<ivec2>& ranges, wchar_t ch) {
					if (ch == '+')
						ranges.push_back({ ch, ch });
					else if (ch > 255)
					{
						ranges.front().x = min(ranges.front().x, (int)ch);
						ranges.front().y = max(ranges.front().y, (int)ch);
					}
				};

				auto to_im_ranges = [](const std::vector<ivec2>& ranges) {
					std::vector<ImWchar> ret;
					for (auto& r : ranges)
					{
						ret.push_back(r[0]);
						ret.push_back(r[1]);
					}
					ret.push_back(0);
					return ret;
				};

				std::vector<ivec2> icon_ranges;
				icon_ranges.emplace_back(65536, 256);

				if (auto icons_txt_path = std::filesystem::path(FONT_AWESOME_DIR) / L"metadata/icons.txt"; std::filesystem::exists(icons_txt_path))
				{
					std::ifstream icons_file(icons_txt_path);
					while (!icons_file.eof())
					{
						std::string hash_s, code_s;
						std::getline(icons_file, hash_s);
						std::getline(icons_file, code_s);
						if (!hash_s.empty() && !code_s.empty())
						{
							auto ch = s2t<uint>(code_s);
							font_icons[s2t<uint>(hash_s)] = { ch, w2s(std::wstring(1, ch)) };
							expand_icon_range(icon_ranges, ch);
						}
					}
					icons_file.close();
				}
				else
				{
					auto icons_json_path = std::filesystem::path(FONT_AWESOME_DIR) / L"metadata/icons.json";
					nlohmann::json icons_json;
					std::ifstream icons_json_file(icons_json_path);
					icons_json_file >> icons_json;
					icons_json_file.close();
					for (auto it = icons_json.begin(); it != icons_json.end(); it++)
					{
						auto name = it.key();
						auto ch = s2u_hex<uint>(it.value()["unicode"].get<std::string>());
						font_icons[sh(name.c_str())] = { ch, w2s(std::wstring(1, ch)) };
						expand_icon_range(icon_ranges, ch);
					}

					std::ofstream icons_file(icons_txt_path);
					for (auto it : font_icons)
					{
						icons_file << it.first << std::endl;
						icons_file << (uint)it.second.first << std::endl;
					}
					icons_file.close();
				}

				std::vector<ivec2> default_ranges;
				for (auto ch = 0x0020; ch <= 0x00FF; ch++)
				{
					auto skip = false;
					for (auto& r : icon_ranges)
					{
						if (r.x <= ch && ch <= r.y)
						{
							skip = true;
							break;
						}
					}
					if (skip)
						continue;
					expand_range(default_ranges, ch);
				}

				ImFontConfig default_font_config;
				auto default_im_ranges = to_im_ranges(default_ranges);
				default_font_config.GlyphRanges = default_im_ranges.data();
				io.Fonts->AddFontDefault(&default_font_config);
#ifdef USE_FONT_AWESOME
				const wchar_t* font_awesome_fonts[] = {
					L"otfs/Font Awesome 6 Free-Solid-900.otf",
					L"otfs/Font Awesome 6 Free-Regular-400.otf",
					L"otfs/Font Awesome 6 Brands-Regular-400.otf"
				};

				auto icon_im_ranges = to_im_ranges(icon_ranges);
				for (auto i = 0; i < countof(font_awesome_fonts); i++)
				{
					auto font_path = std::filesystem::path(FONT_AWESOME_DIR) / font_awesome_fonts[i];
					if (std::filesystem::exists(font_path))
					{
						ImFontConfig font_config;
						font_config.MergeMode = true;
						io.Fonts->AddFontFromFileTTF(font_path.string().c_str(), 13.f, &font_config, icon_im_ranges.data());
					}
				}
#endif

				uchar* img_data;
				int img_w, img_h;
				io.Fonts->GetTexDataAsAlpha8(&img_data, &img_w, &img_h);

				StagingBuffer stag(image_pitch(img_w) * img_h, img_data);

				InstanceCommandBuffer cb;
				imgui_img_font.reset(Image::create(Format_R8_UNORM, uvec3(img_w, img_h, 1), ImageUsageSampled | ImageUsageTransferDst));
				cb->image_barrier(imgui_img_font.get(), {}, ImageLayoutTransferDst);
				BufferImageCopy cpy;
				cpy.img_ext = uvec3(img_w, img_h, 1);
				cb->copy_buffer_to_image(stag.get(), imgui_img_font.get(), { &cpy, 1 });
				cb->image_barrier(imgui_img_font.get(), {}, ImageLayoutShaderReadOnly);
				cb.excute();
			}

			imgui_ds->set_image_i(0, 0, imgui_img_font->get_view({}, { SwizzleOne, SwizzleOne, SwizzleOne, SwizzleR }), Sampler::get(FilterNearest, FilterNearest, false, AddressClampToEdge));
			imgui_ds->update();
#endif
		}

		static std::vector<vec2> circle_pts0;
		static std::vector<vec2> circle_pts1;
		static std::vector<vec2> circle_pts2;
		static std::vector<vec2> circle_pts3;
		const std::vector<vec2>& get_circle_points(uint lod)
		{
			auto get_points = [](uint divides) {
				std::vector<vec2> ret;
				ret.resize(divides);
				auto step = radians(360.f / divides);
				for (auto i = 0; i < divides; i++)
					ret[i] = vec2(cos(step * i), -sin(step * i));
				return ret;
			};
			switch (lod)
			{
			case 0:
				if (circle_pts0.empty())
					circle_pts0 = get_points(16);
				return circle_pts0;
			case 1:
				if (circle_pts1.empty())
					circle_pts1 = get_points(32);
				return circle_pts1;
			case 2:
				if (circle_pts2.empty())
					circle_pts2 = get_points(64);
				return circle_pts2;
			default:
			case 3:
				if (circle_pts3.empty())
					circle_pts3 = get_points(128);
				return circle_pts3;
			}
		}

		wchar_t font_icon(uint name)
		{
			auto it = font_icons.find(name);
			if (it != font_icons.end())
				return it->second.first;
			return 0;
		}

		std::string font_icon_str(uint name)
		{
			auto it = font_icons.find(name);
			if (it != font_icons.end())
				return it->second.second;
			return "";
		}

		Image* get_icon(const std::filesystem::path& path, uint desired_size)
		{
			std::filesystem::path p;
			auto ext = path.extension();
			auto is_image = is_image_file(ext);
			if (is_image)
				p = path;
			else
				p = ext;

			auto it = icons.find(p);
			if (it != icons.end())
			{
				if (it->second.first >= 0)
					it->second.first++;
				return it->second.second;
			}

			if (is_image)
			{
				auto d = get_thumbnail(desired_size, path);
				if (d.second)
				{
					auto image = graphics::Image::create(graphics::Format_B8G8R8A8_UNORM, uvec3(d.first, 1), d.second.get());
					icons.emplace(path, std::make_pair(1, image));
					return image;
				}
			}
			else
			{
				auto d = get_sys_icon(path, nullptr);
				if (d.second)
				{
					auto image = graphics::Image::create(graphics::Format_B8G8R8A8_UNORM, uvec3(d.first, 1), d.second.get());
					icons.emplace(ext, std::make_pair(-1, image));
					return image;
				}
			}

			return nullptr;
		}

		static std::vector<ImagePtr> dead_icons;
		static void* ev_clear_dead_icons = nullptr;

		void release_icon(Image* image)
		{
			for (auto it = icons.begin(); it != icons.end(); it++)
			{
				if (it->second.second == image)
				{
					if (it->second.first > 0)
					{
						it->second.first--;
						if (it->second.first == 0)
						{
							if (!ev_clear_dead_icons)
							{
								ev_clear_dead_icons = add_event([]() {
									for (auto img : dead_icons)
										delete img;
									dead_icons.clear();
									return true;
								});
							}
							dead_icons.push_back((ImagePtr)image);
							icons.erase(it);
						}
					}
					break;
				}
			}
		}
	}
}
