#include <flame/graphics/application.h>
#include <flame/graphics/font.h>
#include <flame/graphics/canvas.h>

using namespace flame;
using namespace graphics;

CanvasPtr canvas;
FontAtlasPtr sdf_font;

struct MyApp : GraphicsApplication
{
	void on_render() override
	{
		canvas->add_rect_filled(vec2(0.f), vec2(100.f), cvec4(255, 127, 60, 255));
		canvas->add_text(sdf_font, 32, vec2(0.f), L"Hello World", cvec4(0, 0, 0, 255), 0.5f, 0.f);
		canvas->add_rect(vec2(100.f), vec2(400.f), 1.f, cvec4(0, 0, 0, 255));
		canvas->begin_stencil_write();
		canvas->add_circle_filled(vec2(400.f), 100.f, cvec4(255, 255, 0, 255));
		canvas->end_stencil_write();
		canvas->begin_stencil_compare();
		canvas->add_text(sdf_font, 32, vec2(300.f, 400.f), L"Some Text Clip By Cicle", cvec4(0, 0, 0, 255), 0.5f, 0.f);
		canvas->end_stencil_compare();
		canvas->render(main_window->swapchain->image_index, command_buffer.get());

		GraphicsApplication::on_render();
	}
};
MyApp app;

int main(int argc, char** args)
{
	app.create("Graphics Test", uvec2(800, 600), WindowStyleFrame | WindowStyleResizable, false, true, { { "mesh_shader"_h, 0 }, { "replace_renderpass_attachment_dont_care_to_load"_h, 1 } });
	canvas = Canvas::create();
	canvas->bind_window(app.main_window);
	sdf_font = FontAtlas::get({ L"flame\\fonts\\OpenSans-Regular.ttf" }, FontAtlasSDF);
	app.run();

	return 0;
}
