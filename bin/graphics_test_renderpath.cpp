#include <flame/foundation/foundation.h>
#include <flame/graphics/all.h>

using namespace flame;

void* sc_in;
void* sc_out;
void* sc_window;
void* sc_image1;
void* sc_image2;
void* sc_renderpass_clear;
void* sc_renderpass_dont_clear;
void* sc_framebuffer1;
void* sc_framebuffer2;
void* cv_in;
void* cv_renderpass;
Array<Bvec4> cv_colors;
void* cv_out;
void* d_in;
void* d_out;
void* cb1_in;
void* cb1_device;
void* cb1_out;
void* cb2_in;
void* cb2_device;
void* cb2_out;
void* cmd_begin_cmd1;
void* cmd_begin_cmd2;
void* cmd_begin_out;
void* cmd_begin_renderpass_in;
void* cmd_begin_renderpass_cmd1;
void* cmd_begin_renderpass_cmd2;
void* cmd_begin_renderpass_renderpass;
void* cmd_begin_renderpass_framebuffer1;
void* cmd_begin_renderpass_framebuffer2;
void* cmd_begin_renderpass_clearvalues;
void* cmd_begin_renderpass_out;
void* cmd_end_renderpass_in;
void* cmd_end_renderpass_cmd1;
void* cmd_end_renderpass_cmd2;
void* cmd_end_renderpass_out;
void* cmd_end_in;
void* cmd_end_cmd1;
void* cmd_end_cmd2;

 __declspec(dllexport) void initialize()
{
	sc_in = nullptr;
	sc_out = nullptr;
	sc_window = nullptr;
	sc_image1 = nullptr;
	sc_image2 = nullptr;
	sc_renderpass_clear = nullptr;
	sc_renderpass_dont_clear = nullptr;
	sc_framebuffer1 = nullptr;
	sc_framebuffer2 = nullptr;
	cv_in = nullptr;
	cv_renderpass = nullptr;
	cv_colors.resize(1);
	cv_colors[0] = Bvec4(255, 127, 0, 1);
	cv_out = nullptr;
	d_in = nullptr;
	d_out = nullptr;
	cb1_in = nullptr;
	cb1_device = nullptr;
	cb1_out = nullptr;
	cb2_in = nullptr;
	cb2_device = nullptr;
	cb2_out = nullptr;
	cmd_begin_cmd1 = nullptr;
	cmd_begin_cmd2 = nullptr;
	cmd_begin_out = nullptr;
	cmd_begin_renderpass_in = nullptr;
	cmd_begin_renderpass_cmd1 = nullptr;
	cmd_begin_renderpass_cmd2 = nullptr;
	cmd_begin_renderpass_renderpass = nullptr;
	cmd_begin_renderpass_framebuffer1 = nullptr;
	cmd_begin_renderpass_framebuffer2 = nullptr;
	cmd_begin_renderpass_clearvalues = nullptr;
	cmd_begin_renderpass_out = nullptr;
	cmd_end_renderpass_in = nullptr;
	cmd_end_renderpass_cmd1 = nullptr;
	cmd_end_renderpass_cmd2 = nullptr;
	cmd_end_renderpass_out = nullptr;
	cmd_end_in = nullptr;
	cmd_end_cmd1 = nullptr;
	cmd_end_cmd2 = nullptr;
}

 __declspec(dllexport) void update()
{
	 if (sc_in) 
 	 { 
 	 	 sc_out = sc_in; 
 	 	 auto sc = (graphics::Swapchain*)sc_out; 
 	 	 sc_window = sc->window(); 
 	 	 sc_image1 = sc->get_image(0); 
 	 	 sc_image2 = sc->get_image(1); 
 	 	 sc_renderpass_clear = sc->get_renderpass_clear(); 
 	 	 sc_renderpass_dont_clear = sc->get_renderpass_dont_clear(); 
 	 	 sc_framebuffer1 = sc->get_framebuffer(0); 
 	 	 sc_framebuffer2 = sc->get_framebuffer(1); 
 	 }

	cv_renderpass = sc_renderpass_clear;
	 if (cv_in) 
 	 	 cv_out = cv_in; 
 	 else 
 	 { 
 	 	 if (cv_renderpass) 
 	 	 	 cv_out = graphics::ClearValues::create((graphics::Renderpass*)cv_renderpass); 
 	 } 
 	 if (cv_out) 
 	 { 
 	 	 for (auto i = 0; i < cv_colors.size; i++) 
 	 	 { 
 	 	 	 auto cv = (graphics::ClearValues*)cv_out; 
 	 	 	 cv->set(i, cv_colors[i]); 
 	 	 } 
 	 }

	 if (d_in) 
 	 	 d_out = d_in;

	cb1_device = d_out;
	 if (cb1_in) 
 	 	 cb1_out = cb1_in; 
 	 else 
 	 { 
 	 	 if (cb1_device) 
 	 	 	 cb1_out = graphics::Commandbuffer::create(((graphics::Device*)cb1_device)->gcp); 
 	 }

	cb2_device = d_out;
	 if (cb2_in) 
 	 	 cb2_out = cb2_in; 
 	 else 
 	 { 
 	 	 if (cb2_device) 
 	 	 	 cb2_out = graphics::Commandbuffer::create(((graphics::Device*)cb2_device)->gcp); 
 	 }

	cmd_begin_cmd1 = cb1_out;
	cmd_begin_cmd2 = cb2_out;
	 if (cmd_begin_cmd1) 
 	 	 ((graphics::Commandbuffer*)cmd_begin_cmd1)->begin(); 
 	 if (cmd_begin_cmd2) 
 	 	 ((graphics::Commandbuffer*)cmd_begin_cmd2)->begin();

	cmd_begin_renderpass_in = cmd_begin_out;
	cmd_begin_renderpass_cmd1 = cb1_out;
	cmd_begin_renderpass_cmd2 = cb2_out;
	cmd_begin_renderpass_renderpass = sc_renderpass_clear;
	cmd_begin_renderpass_framebuffer1 = sc_framebuffer1;
	cmd_begin_renderpass_framebuffer2 = sc_framebuffer2;
	cmd_begin_renderpass_clearvalues = cv_out;
	 if (cmd_begin_renderpass_cmd1) 
 	 { 	 	 ((graphics::Commandbuffer*)cmd_begin_renderpass_cmd1)->begin_renderpass( 
 	 	 	 (graphics::Renderpass*)cmd_begin_renderpass_renderpass, 
 	 	 	 (graphics::Framebuffer*)cmd_begin_renderpass_framebuffer1, 
 	 	 	 (graphics::ClearValues*)cmd_begin_renderpass_clearvalues); 
 	 } 	 if (cmd_begin_renderpass_cmd2) 
 	 { 	 	 ((graphics::Commandbuffer*)cmd_begin_renderpass_cmd2)->begin_renderpass( 
 	 	 	 (graphics::Renderpass*)cmd_begin_renderpass_renderpass, 
 	 	 	 (graphics::Framebuffer*)cmd_begin_renderpass_framebuffer2, 
 	 	 	 (graphics::ClearValues*)cmd_begin_renderpass_clearvalues); 
 	 }

	cmd_end_renderpass_in = cmd_begin_renderpass_out;
	cmd_end_renderpass_cmd1 = cb1_out;
	cmd_end_renderpass_cmd2 = cb2_out;
	 if (cmd_end_renderpass_cmd1) 
 	 	 ((graphics::Commandbuffer*)cmd_end_renderpass_cmd1)->end_renderpass(); 
 	 if (cmd_end_renderpass_cmd2) 
 	 	 ((graphics::Commandbuffer*)cmd_end_renderpass_cmd2)->end_renderpass();

	cmd_end_in = cmd_end_renderpass_out;
	cmd_end_cmd1 = cb1_out;
	cmd_end_cmd2 = cb2_out;
	 if (cmd_end_cmd1) 
 	 	 ((graphics::Commandbuffer*)cmd_end_cmd1)->end(); 
 	 if (cmd_end_cmd2) 
 	 	 ((graphics::Commandbuffer*)cmd_end_cmd2)->end();

}

