#include "../foundation/window.h"
#include "../foundation/system.h"
#include "gui.h"
#include "explorer_abstract.h"
#include "window_private.h"
#include "renderpass_private.h"
#include "font_private.h"
#include "shader_private.h"
#include "command_private.h"
#include "window_private.h"
#include "extension.h"

namespace ImGui
{
	std::vector<std::unique_ptr<Dialog>> dialogs;

	void Dialog::open(Dialog* dialog)
	{
		dialogs.emplace_back(dialog);
		ImGui::OpenPopup(dialog->title.c_str());
	}

	void Dialog::close()
	{
		assert(!dialogs.empty());
		ImGui::CloseCurrentPopup();
		flame::add_event([this]() {
			flame::graphics::Queue::get()->wait_idle();
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
			if (ImGui::BeginPopupModal(title.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize))
			{
				ImGui::TextUnformatted(message.c_str());
				if (ImGui::Button("OK"))
					close();
				ImGui::EndPopup();
			}
		}
	};

	struct YesNoDialog : Dialog
	{
		std::function<void(bool)> callback;

		void draw() override
		{
			if (ImGui::BeginPopupModal(title.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize))
			{
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
		}
	};

	struct InputDialog : Dialog
	{
		std::string text;
		std::function<void(bool, const std::string&)> callback;

		void draw() override
		{
			if (ImGui::BeginPopupModal(title.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize))
			{
				ImGui::InputText("##text", &text);
				if (ImGui::Button("OK"))
				{
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
		}
	};

	struct FileDialog : Dialog
	{
		flame::graphics::ExplorerAbstract explorer;
		std::filesystem::path path;
		std::function<void(bool, const std::filesystem::path&)> callback;

		void draw() override
		{
			if (ImGui::BeginPopupModal(title.c_str()))
			{
				ImGui::BeginChild("explorer", ImVec2(0, -ImGui::GetFontSize() - ImGui::GetStyle().ItemSpacing.y * 3));
				explorer.selected_path = path;
				explorer.draw();
				ImGui::EndChild();
				if (ImGui::Button("OK"))
				{
					if (callback)
						callback(true, path);
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
		}
	};

	void OpenMessageDialog(const std::string title, const std::string& message)
	{
		auto dialog = new MessageDialog;
		if (title == "[RestructurePrefabInstanceWarnning]")
		{
			dialog->title = "Cannot restructure Prefab Instance";
			dialog->message = "You cannot add/remove/reorder entity or component in Prefab Instance\n"
				"Edit it in that prefab";
		}
		else
		{
			dialog->title = title;
			dialog->message = message;
		}
		Dialog::open(dialog);
	}

	void OpenYesNoDialog(const std::string title, const std::function<void(bool)>& callback)
	{
		auto dialog = new YesNoDialog;
		dialog->title = title;
		dialog->callback = callback;
		Dialog::open(dialog);
	}

	void OpenInputDialog(const std::string title, const std::function<void(bool, const std::string&)>& callback)
	{
		auto dialog = new InputDialog;
		dialog->title = title;
		dialog->callback = callback;
		Dialog::open(dialog);
	}

	void OpenFileDialog(const std::string title, const std::function<void(bool, const std::filesystem::path&)>& callback, const std::filesystem::path& start_dir)
	{
		auto dialog = new FileDialog;
		dialog->title = title;
		dialog->callback = callback;
		dialog->explorer.reset(start_dir);
		dialog->explorer.peeding_open_node = { dialog->explorer.folder_tree.get(), false };
		dialog->explorer.select_callback = [dialog](const std::filesystem::path& path) {
			dialog->path = path;
		};
		dialog->explorer.dbclick_callback = [dialog](const std::filesystem::path& path) {
			if (dialog->callback)
				dialog->callback(true, path);
			dialog->close();
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
		static std::vector<std::unique_ptr<FramebufferT>> imgui_fbs;
		static std::unique_ptr<ImageT> imgui_img_font;
		static StorageBuffer<FLAME_UID, BufferUsageVertex> imgui_buf_vtx;
		static StorageBuffer<FLAME_UID, BufferUsageIndex> imgui_buf_idx;
		static std::unique_ptr<DescriptorSetT> imgui_ds;
		static GraphicsPipelinePtr imgui_pl;

		static Listeners<void()> gui_callbacks;

		static std::map<std::filesystem::path, std::pair<int, ImagePtr>> icons;

		static void gui_create_fbs()
		{
			if (!imgui_rp)
			{
				std::vector<std::string> defines;
				defines.push_back("col_fmt=" + TypeInfo::serialize_t(&Swapchain::format));
				imgui_rp = Renderpass::get(L"flame\\shaders\\color.rp", defines);
				defines.push_back("load_op=Load");
				defines.push_back("initia_layout=Attachment");
				imgui_rp_load = Renderpass::get(L"flame\\shaders\\color.rp", defines);
			}
			imgui_fbs.clear();
			for (auto& img : main_window->swapchain->images)
				imgui_fbs.emplace_back(Framebuffer::create(imgui_rp, img->get_view()));
		}

		static void gui_render(uint img_idx, CommandBufferPtr cb)
		{
#if USE_IMGUI
			auto native = main_window->native;
			auto curr_img = main_window->swapchain->images[img_idx].get();
			auto curr_fb = imgui_fbs[img_idx].get();

			auto& io = ImGui::GetIO();
			io.DeltaTime = delta_time;
			io.DisplaySize = ImVec2(main_window->native->size);

			ImGui::NewFrame();

			for (auto& l : gui_callbacks.list)
				l.first();

			for (auto& d : ImGui::dialogs)
				d->draw();

			auto mouse_consumed = io.WantCaptureMouse;
			auto keyboard_consumed = io.WantCaptureKeyboard;

			switch (ImGui::GetMouseCursor())
			{
			case ImGuiMouseCursor_None:
				native->set_cursor(CursorNone);
				break;
			case ImGuiMouseCursor_Arrow:
				native->set_cursor(CursorArrow);
				break;
			case ImGuiMouseCursor_TextInput:
				native->set_cursor(CursorIBeam);
				break;
			case ImGuiMouseCursor_ResizeAll:
				native->set_cursor(CursorSizeAll);
				break;
			case ImGuiMouseCursor_ResizeNS:
				native->set_cursor(CursorSizeNS);
				break;
			case ImGuiMouseCursor_ResizeEW:
				native->set_cursor(CursorSizeWE);
				break;
			case ImGuiMouseCursor_ResizeNESW:
				native->set_cursor(CursorSizeNESW);
				break;
			case ImGuiMouseCursor_ResizeNWSE:
				native->set_cursor(CursorSizeNWSE);
				break;
			case ImGuiMouseCursor_Hand:
				native->set_cursor(CursorHand);
				break;
			case ImGuiMouseCursor_NotAllowed:
				native->set_cursor(CursorNo);
				break;
			}

			ImGui::EndFrame();
			ImGui::Render();

			auto draw_data = ImGui::GetDrawData();
			int fb_width = (int)(draw_data->DisplaySize.x * draw_data->FramebufferScale.x);
			int fb_height = (int)(draw_data->DisplaySize.y * draw_data->FramebufferScale.y);
			if (fb_width > 0 || fb_height > 0)
			{
				if (draw_data->TotalVtxCount > 0)
				{
					for (int i = 0; i < draw_data->CmdListsCount; i++)
					{
						auto cmd_list = draw_data->CmdLists[i];
						imgui_buf_vtx.push(cmd_list->VtxBuffer.Size, cmd_list->VtxBuffer.Data);
						imgui_buf_idx.push(cmd_list->IdxBuffer.Size, cmd_list->IdxBuffer.Data);
					}

					imgui_buf_vtx.upload(cb);
					imgui_buf_idx.upload(cb);
				}

				if (curr_img->levels[0].layers[0].layout != ImageLayoutAttachment)
				{
					auto cv = vec4(0.4f, 0.3f, 0.7f, 1);
					cb->begin_renderpass(imgui_rp, curr_fb, &cv);
				}
				else
					cb->begin_renderpass(imgui_rp_load, curr_fb);
				cb->set_viewport(Rect(0, 0, fb_width, fb_height));

				cb->bind_pipeline(imgui_pl);
				cb->bind_vertex_buffer(imgui_buf_vtx.buf.get(), 0);
				cb->bind_index_buffer(imgui_buf_idx.buf.get(), sizeof(ImDrawIdx) == 2 ? IndiceTypeUshort : IndiceTypeUint);
				cb->bind_descriptor_set(0, imgui_ds.get());
				auto scale = 2.f / vec2(draw_data->DisplaySize.x, draw_data->DisplaySize.y);
				cb->push_constant_t(vec4(scale,
					-1.f - draw_data->DisplayPos.x * scale[0],
					-1.f - draw_data->DisplayPos.y * scale[1]));

				ImVec2 clip_off = draw_data->DisplayPos;
				ImVec2 clip_scale = draw_data->FramebufferScale;

				int global_vtx_offset = 0;
				int global_idx_offset = 0;
				ImagePtr last_tex = nullptr;
				auto last_view_type = ImGui::ImageViewRGBA;
				for (int n = 0; n < draw_data->CmdListsCount; n++)
				{
					const ImDrawList* cmd_list = draw_data->CmdLists[n];
					for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
					{
						const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];

						ImVec2 clip_min((pcmd->ClipRect.x - clip_off.x) * clip_scale.x, (pcmd->ClipRect.y - clip_off.y) * clip_scale.y);
						ImVec2 clip_max((pcmd->ClipRect.z - clip_off.x) * clip_scale.x, (pcmd->ClipRect.w - clip_off.y) * clip_scale.y);

						if (clip_min.x < 0.0f) { clip_min.x = 0.0f; }
						if (clip_min.y < 0.0f) { clip_min.y = 0.0f; }
						if (clip_max.x > fb_width) { clip_max.x = (float)fb_width; }
						if (clip_max.y > fb_height) { clip_max.y = (float)fb_height; }
						if (clip_max.x < clip_min.x || clip_max.y < clip_min.y)
							continue;

						auto view_type = (ImGui::ImageViewType)(uint)pcmd->UserCallbackData;
						if (last_tex != pcmd->TextureId || last_view_type != view_type)
						{
							auto tex = (ImagePtr)pcmd->TextureId;
							ImageSwizzle swizzle;
							switch (view_type)
							{
							case ImGui::ImageViewR: swizzle = { SwizzleR, SwizzleZero, SwizzleZero, SwizzleOne }; break;
							case ImGui::ImageViewG: swizzle = { SwizzleZero, SwizzleG, SwizzleZero, SwizzleOne }; break;
							case ImGui::ImageViewB: swizzle = { SwizzleZero, SwizzleZero, SwizzleB, SwizzleOne }; break;
							case ImGui::ImageViewA: swizzle = { SwizzleA, SwizzleA, SwizzleA, SwizzleOne }; break;
							case ImGui::ImageViewRGB: swizzle = { SwizzleR, SwizzleG, SwizzleB, SwizzleOne }; break;
							}
							cb->bind_descriptor_set(0, tex ? tex->get_shader_read_src(0, 0, nullptr, swizzle) : imgui_ds.get());
							last_tex = tex;
							last_view_type = view_type;
						}

						cb->set_scissor(Rect(clip_min.x, clip_min.y, clip_max.x, clip_max.y));
						cb->draw_indexed(pcmd->ElemCount, pcmd->IdxOffset + global_idx_offset, pcmd->VtxOffset + global_vtx_offset, 1, 0);
					}
					global_idx_offset += cmd_list->IdxBuffer.Size;
					global_vtx_offset += cmd_list->VtxBuffer.Size;
				}

				cb->end_renderpass();
			}
#endif
		}

		void gui_initialize()
		{
#if USE_IMGUI
			assert(!windows.empty());
			main_window = windows.front();

			gui_create_fbs();

			auto native = main_window->native;
			native->mouse_listeners.add([](MouseButton btn, bool down) {
				ImGuiIO& io = ImGui::GetIO();
				io.MouseDown[btn] = down;
			});
			native->mousemove_listeners.add([](const ivec2& pos) {
				ImGuiIO& io = ImGui::GetIO();
				io.MousePos = ImVec2(pos.x, pos.y);
			});
			native->scroll_listeners.add([](int scroll) {
				ImGuiIO& io = ImGui::GetIO();
				io.MouseWheel = scroll;
			});
			native->key_listeners.add([](KeyboardKey key, bool down) {
				ImGuiIO& io = ImGui::GetIO();
				io.KeysDown[key] = down;
				if (key == Keyboard_Ctrl)
					io.KeyCtrl = down;
				if (key == Keyboard_Shift)
					io.KeyShift = down;
				if (key == Keyboard_Alt)
					io.KeyAlt = down;
			});
			native->char_listeners.add([](wchar_t ch) {
				ImGuiIO& io = ImGui::GetIO();
				io.AddInputCharacter(ch);
			});
			native->resize_listeners.add([](const vec2&) {
				gui_create_fbs();
			});

			imgui_pl = GraphicsPipeline::get(L"flame\\shaders\\imgui.pipeline",
				{ "rp=" + str(imgui_rp) });
			imgui_buf_vtx.create(sizeof(ImDrawVert), 360000);
			imgui_buf_idx.create(sizeof(ImDrawIdx), 240000);
			imgui_ds.reset(DescriptorSet::create(DescriptorPool::current(), imgui_pl->layout->dsls[0]));

			IMGUI_CHECKVERSION();

			ImGui::CreateContext();
			ImGuiIO& io = ImGui::GetIO();
			io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
			io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

			ImGui::StyleColorsDark();

			ImGuiStyle& style = ImGui::GetStyle();
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;

			assert(!io.BackendPlatformUserData);

			io.BackendPlatformName = "imgui_impl_flame";
			io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
			io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;
			io.BackendFlags |= ImGuiBackendFlags_PlatformHasViewports;

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

			{
				std::filesystem::path font_path = L"c:\\Windows\\Fonts\\msyh.ttc";
				if (std::filesystem::exists(font_path))
					io.Fonts->AddFontFromFileTTF(font_path.string().c_str(), 16.f, nullptr, io.Fonts->GetGlyphRangesChineseFull());
#ifdef USE_FONT_AWESOME
				const wchar_t* font_awesome_fonts[] = {
					L"otfs/Font Awesome 6 Brands-Regular-400.otf",
					L"otfs/Font Awesome 6 Free-Regular-400.otf",
					L"otfs/Font Awesome 6 Free-Solid-900.otf"
				};
				auto icons_range = FontAtlas::icons_range();
				for (auto i = 0; i < countof(font_awesome_fonts); i++)
				{
					font_path = std::filesystem::path(FONT_AWESOME_DIR) / font_awesome_fonts[i];
					if (std::filesystem::exists(font_path))
					{
						ImWchar ranges[] =
						{
							icons_range[0], icons_range[1],
							0,
						};
						ImFontConfig config;
						config.MergeMode = true;
						io.Fonts->AddFontFromFileTTF(font_path.string().c_str(), 16.f, &config, &ranges[0]);
					}
				}
#endif

				uchar* img_data;
				int img_w, img_h;
				io.Fonts->GetTexDataAsAlpha8(&img_data, &img_w, &img_h);

				StagingBuffer stag(image_pitch(img_w) * img_h, img_data);
				InstanceCB cb;

				imgui_img_font.reset(Image::create(Format_R8_UNORM, uvec2(img_w, img_h), ImageUsageSampled | ImageUsageTransferDst));
				cb->image_barrier(imgui_img_font.get(), {}, ImageLayoutTransferDst);
				BufferImageCopy cpy;
				cpy.img_ext = uvec2(img_w, img_h);
				cb->copy_buffer_to_image(stag.get(), imgui_img_font.get(), { &cpy, 1 });
				cb->image_barrier(imgui_img_font.get(), {}, ImageLayoutShaderReadOnly);
			}

			imgui_ds->set_image(0, 0, imgui_img_font->get_view({}, { SwizzleOne, SwizzleOne, SwizzleOne, SwizzleR }), Sampler::get(FilterNearest, FilterNearest, false, AddressClampToEdge));
			imgui_ds->update();

			main_window->renderers.add(gui_render);
#endif

			if (auto image = Image::get(L"flame/icon_model.png"); image)
				icons.emplace(L"model", std::make_pair(-1, image));
			if (auto image = Image::get(L"flame/icon_armature.png"); image)
				icons.emplace(L"armature", std::make_pair(-1, image));
			if (auto image = Image::get(L"flame/icon_mesh.png"); image)
				icons.emplace(L"mesh", std::make_pair(-1, image));
		}

		void* gui_native_handle()
		{
#if USE_IMGUI
			return ImGui::GetCurrentContext();
#endif
			return nullptr;
		}

		std::filesystem::path parse_icon_path(const std::filesystem::path& path)
		{
			auto ext = path.extension();
			if (is_image_file(ext))
				return path;
			else
			{
				auto sp = SUW::split(path, '#');
				if (sp.size() < 2)
				{
					if (ext == L".fmod")
						return L"model";
				}
				else
				{
					if (sp[1].starts_with(L"armature"))
						return L"armature";
					else if (sp[1].starts_with(L"mesh"))
						return L"mesh";
				}

				int id;
				get_sys_icon(path, &id);
				return wstr(id);
			}
			return L"";
		}

		Image* get_icon(const std::filesystem::path& _path, uint desired_size)
		{
			auto path = parse_icon_path(_path);

			auto it = icons.find(path);
			if (it != icons.end())
			{
				if (it->second.first >= 0)
					it->second.first++;
				return it->second.second;
			}

			if (path == _path)
			{
				auto d = get_thumbnail(desired_size, path);
				if (d.second)
				{
					auto image = graphics::Image::create(graphics::Format_B8G8R8A8_UNORM, d.first, d.second.get());
					icons.emplace(path, std::make_pair(1, image));
					return image;
				}
			}
			else
			{
				auto d = get_sys_icon(_path.c_str(), nullptr);
				if (d.second)
				{
					auto image = graphics::Image::create(graphics::Format_B8G8R8A8_UNORM, d.first, d.second.get());
					icons.emplace(path, std::make_pair(-1, image));
					return image;
				}
			}

			return nullptr;
		}

		void release_icon(const std::filesystem::path& _path)
		{
			auto path = parse_icon_path(_path);
			if (path == _path)
			{
				auto it = icons.find(path);
				if (it != icons.end())
				{
					if (it->second.first >= 0)
					{
						it->second.first--;
						if (it->second.first == 0)
						{
							delete it->second.second;
							icons.erase(it);
						}
					}
				}
			}
		}
	}
}