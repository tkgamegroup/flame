#include <flame/foundation/foundation.h>

using namespace flame;

void* sc_in = nullptr;
void* sc_out = nullptr;
void* sc_window = nullptr;
void* sc_image1 = nullptr;
void* sc_image2 = nullptr;
void* sc_renderpass_clear = nullptr;
void* sc_renderpass_dont_clear = nullptr;
void* sc_framebuffer1 = nullptr;
void* sc_framebuffer2 = nullptr;
void* cv_in = nullptr;
void* cv_renderpass = nullptr;
Bvec4 cv_colors_0 = ;
void* cv_out = nullptr;
void* d_in = nullptr;
void* d_out = nullptr;
void* cb1_in = nullptr;
void* cb1_device = nullptr;
void* cb1_out = nullptr;
void* cb2_in = nullptr;
void* cb2_device = nullptr;
void* cb2_out = nullptr;
void* cmd_begin_cmd1 = nullptr;
void* cmd_begin_cmd2 = nullptr;
void* cmd_begin_out = nullptr;
void* cmd_begin_renderpass_in = nullptr;
void* cmd_begin_renderpass_cmd1 = nullptr;
void* cmd_begin_renderpass_cmd2 = nullptr;
void* cmd_begin_renderpass_renderpass = nullptr;
void* cmd_begin_renderpass_framebuffer1 = nullptr;
void* cmd_begin_renderpass_framebuffer2 = nullptr;
void* cmd_begin_renderpass_clearvalues = nullptr;
void* cmd_begin_renderpass_out = nullptr;
void* cmd_end_renderpass_in = nullptr;
void* cmd_end_renderpass_cmd1 = nullptr;
void* cmd_end_renderpass_cmd2 = nullptr;
void* cmd_end_renderpass_out = nullptr;
void* cmd_end_in = nullptr;
void* cmd_end_cmd1 = nullptr;
void* cmd_end_cmd2 = nullptr;

void update()
{
	if (in$i) { out$o = in$i; auto sc = (graphics::Swapchain*)out$o; window$o = sc->window(); image1$o = sc->get_image(0); image2$o = sc->get_image(1); renderpass_clear$o = sc->get_renderpass_clear(); renderpass_dont_clear$o = sc->get_renderpass_dont_clear(); framebuffer1$o = sc->get_framebuffer(0); framebuffer2$o = sc->get_framebuffer(1); }
	if (in$i) out$o = in$i; else { if (renderpass$i) out$o = graphics::ClearValues::create((graphics::Renderpass*)renderpass$i); } if (out$o) { for (auto i = 0; i < colors$i.size; i++) { auto cv = (graphics::ClearValues*)out$o; cv->set(i, colors$i[i]); } }
	if (in$i) out$o = in$i;
	if (in$i) out$o = in$i; else { if (device$i) out$o = graphics::Commandbuffer::create(((graphics::Device*)device$i)->gcp); }
	if (in$i) out$o = in$i; else { if (device$i) out$o = graphics::Commandbuffer::create(((graphics::Device*)device$i)->gcp); }
	if (cmd1$i) ((graphics::Commandbuffer*)cmd1$i)->begin(); if (cmd2$i) ((graphics::Commandbuffer*)cmd2$i)->begin();
	if (cmd1$i) ((graphics::Commandbuffer*)cmd1$i)->begin_renderpass((graphics::Renderpass*)renderpass$i, (graphics::Framebuffer*)framebuffer1$i, (graphics::ClearValues*)clearvalues$i); if (cmd2$i) ((graphics::Commandbuffer*)cmd2$i)->begin_renderpass((graphics::Renderpass*)renderpass$i, (graphics::Framebuffer*)framebuffer2$i, (graphics::ClearValues*)clearvalues$i);
	if (cmd1$i) ((graphics::Commandbuffer*)cmd1$i)->end_renderpass(); if (cmd2$i) ((graphics::Commandbuffer*)cmd2$i)->end_renderpass();
	if (cmd1$i) ((graphics::Commandbuffer*)cmd1$i)->end(); if (cmd2$i) ((graphics::Commandbuffer*)cmd2$i)->end();
}

