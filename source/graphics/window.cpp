#include "../foundation/window.h"
#include "device_private.h"
#include "image_private.h"
#include "swapchain_private.h"
#include "renderpass_private.h"
#include "shader_private.h"
#include "command_private.h"
#include "window_private.h"
#include "extension.h"

namespace flame
{
	namespace graphics
	{
		WindowPrivate::~WindowPrivate()
		{
			Queue::get(nullptr)->wait_idle();
		}

		void* WindowPrivate::imgui_context()
		{
			return ImGui::GetCurrentContext();
		}

		void WindowPrivate::imgui_new_frame()
		{
			if (swapchain->images.empty())
				return;

#if USE_IMGUI
			auto& io = ImGui::GetIO();
			io.DeltaTime = delta_time;

			ImGui::NewFrame();

			for (auto& l : imgui_callbacks.list)
				l.first();

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
#endif
		}

		void WindowPrivate::update()
		{
			if (!dirty || swapchain->images.empty())
				return;

			finished_fence->wait();
			commandbuffer->calc_executed_time();
			//printf("%lfms\n", (double)commandbuffer->last_executed_time / (double)1000000);

			auto img_idx = swapchain->acquire_image();
			auto curr_img = swapchain->images[img_idx].get();
			auto curr_fb = framebuffers[img_idx].get();

			commandbuffer->begin();

			for (auto& l : renderers.list)
				l.first(img_idx, commandbuffer.get());

#if USE_IMGUI
			ImGui::Render();
			{
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

						imgui_buf_vtx.upload(commandbuffer.get());
						imgui_buf_idx.upload(commandbuffer.get());
					}

					if (curr_img->levels[0].layers[0].layout != ImageLayoutAttachment)
					{
						auto cv = vec4(0.4f, 0.3f, 0.7f, 1);
						commandbuffer->begin_renderpass(renderpass_clear, curr_fb, &cv);
					}
					else
						commandbuffer->begin_renderpass(renderpass_load, curr_fb);
					commandbuffer->set_viewport(Rect(0, 0, fb_width, fb_height));

					commandbuffer->bind_pipeline(imgui_pl.get());
					commandbuffer->bind_vertex_buffer(imgui_buf_vtx.buf.get(), 0);
					commandbuffer->bind_index_buffer(imgui_buf_idx.buf.get(), sizeof(ImDrawIdx) == 2 ? IndiceTypeUshort : IndiceTypeUint);
					commandbuffer->bind_descriptor_set(0, imgui_ds.get());
					auto scale = 2.f / vec2(draw_data->DisplaySize.x, draw_data->DisplaySize.y);
					commandbuffer->push_constant_t(vec4(scale,
						-1.f - draw_data->DisplayPos.x * scale[0],
						-1.f - draw_data->DisplayPos.y * scale[1]));

					ImVec2 clip_off = draw_data->DisplayPos;
					ImVec2 clip_scale = draw_data->FramebufferScale;

					int global_vtx_offset = 0;
					int global_idx_offset = 0;
					Image* last_tex = nullptr;
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

							if (last_tex != pcmd->TextureId)
							{
								auto tex = (Image*)pcmd->TextureId;
								commandbuffer->bind_descriptor_set(0, tex ? tex->get_shader_read_src() : imgui_ds.get());
								last_tex = tex;
							}

							commandbuffer->set_scissor(Rect(clip_min.x, clip_min.y, clip_max.x, clip_max.y));
							commandbuffer->draw_indexed(pcmd->ElemCount, pcmd->IdxOffset + global_idx_offset, pcmd->VtxOffset + global_vtx_offset, 1, 0);
						}
						global_idx_offset += cmd_list->IdxBuffer.Size;
						global_vtx_offset += cmd_list->VtxBuffer.Size;
					}

					commandbuffer->end_renderpass();
				}
			}
#endif

			commandbuffer->image_barrier(curr_img, {}, ImageLayoutPresent);
			commandbuffer->end();

			auto queue = graphics::Queue::get(nullptr);
			queue->submit1(commandbuffer.get(), swapchain->image_avalible.get(), finished_semaphore.get(), finished_fence.get());
			queue->present(swapchain.get(), finished_semaphore.get());

			dirty = false;
		}

		std::vector<WindowPtr> windows;

		struct WindowCreate : Window::Create
		{
			WindowPtr operator()(DevicePtr device, NativeWindow* native) override
			{
				if (!device)
					device = current_device;

				auto ret = new WindowPrivate;
				ret->device = device;
				ret->native = native;

				ret->swapchain.reset(Swapchain::create(device, native));
				ret->commandbuffer.reset(CommandBuffer::create(CommandPool::get(device)));
				ret->commandbuffer->want_executed_time = true;
				ret->finished_fence.reset(Fence::create(device));
				ret->finished_semaphore.reset(Semaphore::create(device));

				auto fmt_str = "col_fmt=" + TypeInfo::serialize_t(&Swapchain::format);
				ret->renderpass_clear = Renderpass::get(device, L"flame\\shaders\\color.rp", { fmt_str });
				ret->renderpass_load = Renderpass::get(device, L"flame\\shaders\\color.rp", { fmt_str, "load_op=Load", "initia_layout=Attachment" });

#if USE_IMGUI
				ret->native->mouse_listeners.add([this](MouseButton btn, bool down) {
					ImGuiIO& io = ImGui::GetIO();
					io.MouseDown[btn] = down;
				});

				ret->native->mousemove_listeners.add([this](const ivec2& pos) {
					ImGuiIO& io = ImGui::GetIO();
					io.MousePos = ImVec2(pos.x, pos.y);
				});

				ret->native->scroll_listeners.add([this](int scroll) {
					ImGuiIO& io = ImGui::GetIO();
					io.MouseWheel = scroll;
				});

				ret->native->key_listeners.add([this](KeyboardKey key, bool down) {
					ImGuiIO& io = ImGui::GetIO();
					io.KeysDown[key] = down;
					if (key == Keyboard_Ctrl)
						io.KeyCtrl = down;
					if (key == Keyboard_Shift)
						io.KeyShift = down;
					if (key == Keyboard_Alt)
						io.KeyAlt = down;
				});

				ret->native->char_listeners.add([this](wchar_t ch) {
					ImGuiIO& io = ImGui::GetIO();
					io.AddInputCharacter(ch);
				});

				ret->imgui_pl.reset(GraphicsPipeline::get(device, L"flame\\shaders\\imgui.pipeline", 
					{ "rp=" + str(ret->renderpass_clear) }));
				ret->imgui_buf_vtx.create(sizeof(ImDrawVert), 360000);
				ret->imgui_buf_idx.create(sizeof(ImDrawIdx), 240000);
				ret->imgui_ds.reset(DescriptorSet::create(DescriptorPool::current(device), ret->imgui_pl->layout->dsls[0]));

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
					{
						std::filesystem::path font_path = L"c:\\Windows\\Fonts\\msyh.ttc";
						if (std::filesystem::exists(font_path))
							io.Fonts->AddFontFromFileTTF(font_path.string().c_str(), 16.f, nullptr, io.Fonts->GetGlyphRangesChineseFull());
					}

					uchar* img_data;
					int img_w, img_h;
					io.Fonts->GetTexDataAsAlpha8(&img_data, &img_w, &img_h);

					StagingBuffer stag(nullptr, image_pitch(img_w) * img_h, img_data);
					InstanceCB cb(nullptr);

					ret->imgui_img_font.reset(Image::create(nullptr, Format_R8_UNORM, uvec2(img_w, img_h), ImageUsageSampled | ImageUsageTransferDst));
					cb->image_barrier(ret->imgui_img_font.get(), {}, ImageLayoutTransferDst);
					BufferImageCopy cpy;
					cpy.img_ext = uvec2(img_w, img_h);
					cb->copy_buffer_to_image(stag.get(), ret->imgui_img_font.get(), { &cpy, 1 });
					cb->image_barrier(ret->imgui_img_font.get(), {}, ImageLayoutShaderReadOnly);
				}

				ret->imgui_ds->set_image(0, 0, ret->imgui_img_font->get_view({}, { SwizzleOne, SwizzleOne, SwizzleOne, SwizzleR }), Sampler::get(nullptr, FilterNearest, FilterNearest, false, AddressClampToEdge));
				ret->imgui_ds->update();

#endif

				auto resize = [ret]() {
					for (auto& img : ret->swapchain->images)
						ret->framebuffers.emplace_back(Framebuffer::create(ret->renderpass_clear, img->get_view()));

#if USE_IMGUI
					ImGuiIO& io = ImGui::GetIO();
					auto sz = ret->framebuffers.empty() ? vec2(-1) : vec2(ret->native->size);
					io.DisplaySize = ImVec2(sz.x, sz.y);
#endif
				};

				resize();

				native->resize_listeners.add([=](const vec2&) {
					ret->framebuffers.clear();
					resize();
				});

				native->destroy_listeners.add([ret]() {
					for (auto it = windows.begin(); it != windows.end(); it++)
					{
						if (*it == ret)
						{
							windows.erase(it);
							delete ret;
							return;
						}
					}
				});

				windows.emplace_back(ret);
				return ret;
			}
		}Window_create;
		Window::Create& Window::create = Window_create;

		struct WindowGetList : Window::GetList
		{
			const std::vector<WindowPtr>& operator()() override
			{
				return windows;
			}
		}Window_get_list;
		Window::GetList& Window::get_list = Window_get_list;
	}
}
