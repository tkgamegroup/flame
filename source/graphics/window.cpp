#include "../foundation/window.h"
#include "device_private.h"
#include "image_private.h"
#include "swapchain_private.h"
#include "renderpass_private.h"
#include "shader_private.h"
#include "command_private.h"
#include "command_ext.h"
#include "window_private.h"

auto imgui_pl_str = R"^^^(
layout
  @pll
shaders
  @vert
 ---
  @frag
renderpass
  {rp}
vertex_buffers
  attributes
    location
      0
    format
      R32G32_SFLOAT
   ---
    location
      1
    format
      R32G32_SFLOAT
   ---
    location
      2
    format
      R8G8B8A8_UNORM
cull_mode
  None
blend_options
  enable
    true
  src_color
    SrcAlpha
  dst_color
    OneMinusSrcAlpha
  src_alpha
    Zero
  dst_alpha
    Zero

@pll
layout (set = SET, binding = 0) uniform sampler2D image;

layout(push_constant) uniform PushConstant
{
	vec2 scale;
	vec2 translate;
}pc;
@

@vert
layout (location = 0) in vec2 i_pos;
layout (location = 1) in vec2 i_uv;
layout (location = 2) in vec4 i_col;

layout (location = 0) out vec4 o_col;
layout (location = 1) out vec2 o_uv;

void main()
{
	o_col = i_col;
	o_uv = i_uv;
	gl_Position = vec4(i_pos * pc.scale + pc.translate, 0, 1);
}
@

@frag
layout (location = 0) in vec4 i_col;
layout (location = 1) in vec2 i_uv;

layout (location = 0) out vec4 o_col;

void main()
{
	o_col = i_col * texture(image, i_uv);
}
@
)^^^";

namespace flame
{
	namespace graphics
	{
		WindowPrivate::~WindowPrivate()
		{
			Queue::get(nullptr)->wait_idle();
		}

		void WindowPrivate::imgui_new_frame()
		{
#if USE_IMGUI
			ImGui::NewFrame();

			auto ctx = ImGui::GetCurrentContext();
			for (auto& cb : imgui_callbacks.list)
				cb(ctx);

			auto& io = ImGui::GetIO();
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

			submit_fence->wait();

			auto img_idx = swapchain->acquire_image();

			commandbuffer->begin();

			for (auto& r : renderers.list)
				r(img_idx, commandbuffer.get());

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

					if (renderers.list.empty())
					{
						auto cv = vec4(0.4f, 0.3f, 0.7f, 1);
						commandbuffer->begin_renderpass(renderpass_clear.get(), framebuffers[img_idx].get(), &cv);
					}
					else
						commandbuffer->begin_renderpass(renderpass_load.get(), framebuffers[img_idx].get());
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

			commandbuffer->image_barrier(swapchain->images[img_idx].get(), {}, ImageLayoutAttachment, ImageLayoutPresent);
			commandbuffer->end();

			auto queue = graphics::Queue::get(nullptr);
			queue->submit1(commandbuffer.get(), swapchain->image_avalible.get(), render_finished.get(), submit_fence.get());
			queue->present(swapchain.get(), render_finished.get());

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
				ret->submit_fence.reset(Fence::create(device));
				ret->render_finished.reset(Semaphore::create(device));

				{
					RenderpassInfo info;
					auto& att = info.attachments.emplace_back();
					att.format = Swapchain::format;
					auto& sp = info.subpasses.emplace_back();
					sp.color_attachments.push_back(0);
					ret->renderpass_clear.reset(Renderpass::create(device, info));
					att.load_op = AttachmentLoadLoad;
					att.initia_layout = ImageLayoutAttachment;
					ret->renderpass_load.reset(Renderpass::create(device, info));
				}

#if USE_IMGUI
				ret->native->mouse_left_down_listeners.add([this](const ivec2& pos) {
					ImGuiIO& io = ImGui::GetIO();
					io.MouseDown[0] = true;
				});

				ret->native->mouse_left_up_listeners.add([this](const ivec2& pos) {
					ImGuiIO& io = ImGui::GetIO();
					io.MouseDown[0] = false;
				});

				ret->native->mouse_move_listeners.add([this](const ivec2& pos) {
					ImGuiIO& io = ImGui::GetIO();
					io.MousePos = ImVec2(pos.x, pos.y);
				});

				ret->native->mouse_scroll_listeners.add([this](int scroll) {
					ImGuiIO& io = ImGui::GetIO();
					io.MouseWheel = scroll;
				});

				ret->imgui_pl.reset(GraphicsPipeline::create(device, imgui_pl_str, { "rp=0x" + to_string((uint64)ret->renderpass_clear.get()) }));
				ret->imgui_buf_vtx.create(sizeof(ImDrawVert), 360000);
				ret->imgui_buf_idx.create(sizeof(ImDrawIdx), 240000);
				ret->imgui_ds.reset(DescriptorSet::create(DescriptorPool::current(device), ret->imgui_pl->info.layout->descriptor_set_layouts[0]));

				IMGUI_CHECKVERSION();

				ImGui::CreateContext();
				ImGuiIO& io = ImGui::GetIO();
				io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
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

				io.KeyMap[ImGuiKey_Space] = Keyboard_Space;

				{
					uchar* img_data;
					int img_w, img_h;
					io.Fonts->GetTexDataAsAlpha8(&img_data, &img_w, &img_h);

					StagingBuffer stag(nullptr, image_pitch(img_w) * img_h, img_data);
					InstanceCB cb(nullptr);

					ret->imgui_img_font.reset(Image::create(nullptr, Format_R8_UNORM, uvec2(img_w, img_h), 1, 1, SampleCount_1, ImageUsageSampled | ImageUsageTransferDst));
					cb->image_barrier(ret->imgui_img_font.get(), {}, ImageLayoutUndefined, ImageLayoutTransferDst);
					BufferImageCopy cpy;
					cpy.img_ext = uvec2(img_w, img_h);
					cb->copy_buffer_to_image(stag.get(), ret->imgui_img_font.get(), { &cpy, 1 });
					cb->image_barrier(ret->imgui_img_font.get(), {}, ImageLayoutTransferDst, ImageLayoutShaderReadOnly);
				}

				ret->imgui_ds->set_image(0, 0, ret->imgui_img_font->get_view({}, { SwizzleOne, SwizzleOne, SwizzleOne, SwizzleR }), Sampler::get(nullptr, FilterNearest, FilterNearest, false, AddressClampToEdge));
				ret->imgui_ds->update();

#if USE_IM_FILE_DIALOG
				ifd::FileDialog::Instance().CreateTexture = [](uint8_t* data, int w, int h, char fmt) -> void*
				{
					return graphics::Image::create(nullptr, fmt == 1 ? graphics::Format_R8G8B8A8_UNORM : graphics::Format_B8G8R8A8_UNORM, uvec2(w, h), data);
				};

				ifd::FileDialog::Instance().DeleteTexture = [](void* tex)
				{
					add_event([tex]() {
						graphics::Queue::get(nullptr)->wait_idle();
						delete ((graphics::Image*)tex);
						return false;
					});
				};
#endif

#endif

				auto resize = [ret]() {
					for (auto& img : ret->swapchain->images)
					{
						auto iv = img->get_view();
						ret->framebuffers.emplace_back(Framebuffer::create(ret->renderpass_clear.get(), { &iv, 1 }));
					}

#if USE_IMGUI
					ImGuiIO& io = ImGui::GetIO();
					auto sz = ret->framebuffers.empty() ? vec2(-1) : vec2(ret->native->size);
					io.DisplaySize = ImVec2(sz.x, sz.y);
#endif
				};

				resize();

				native->resize_listeners.add([&, ret](const vec2&) {
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

		const std::vector<WindowPtr> get_windows()
		{
			return windows;
		}
	}
}
