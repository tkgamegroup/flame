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
		GraphicsApplication::on_render();
		canvas->add_rect_filled(vec2(0.f), vec2(100.f), cvec4(255, 127, 60, 255));
		canvas->add_text(vec2(0.f), L"Hello World", cvec4(0, 0, 0, 255));
		canvas->add_rect(vec2(100.f), vec2(200.f), 1.f, cvec4(0, 0, 0, 255));
	}
};
MyApp app;

int main(int argc, char** args)
{
	app.create("Graphics Test", uvec2(800, 600), WindowFrame | WindowResizable, true, { { "mesh_shader"_h, 0 } });
	canvas = Canvas::create(app.main_window);
	sdf_font = FontAtlas::get({}, FontAtlasSDF);
	app.run();

	return 0;
}
