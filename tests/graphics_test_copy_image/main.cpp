#include <flame/serialize.h>
#include <flame/foundation/foundation.h>
#include <flame/graphics/device.h>
#include <flame/graphics/swapchain.h>
#include <flame/graphics/synchronize.h>
#include <flame/graphics/command.h>
#include <flame/graphics/renderpass.h>
#include <flame/graphics/image.h>
#include <flame/graphics/shader.h>

using namespace flame;
using namespace graphics;

struct App
{
	Window* w;
	Device* d;
	Swapchain* sc;
	Image* img;

	void on_resize()
	{
		if (sc->image_count() > 0)
		{
			sc->acquire_image();

			auto dst = sc->image(sc->image_index());
			auto cb = CommandBuffer::create(Commandpool::get_default(QueueGraphics));
			cb->begin();
			cb->change_image_layout(dst, ImageLayoutUndefined, ImageLayoutTransferDst);
			ImageCopy cpy;
			cpy.size = min(dst->size, img->size);
			cb->copy_image(img, dst, 1, &cpy);
			cb->change_image_layout(dst, ImageLayoutTransferDst, ImageLayoutPresent);
			cb->end();
			auto finished = Semaphore::create(d);
			Queue::get_default(QueueGraphics)->submit(1, &cb, sc->image_avalible(), finished, nullptr);

			Queue::get_default(QueueGraphics)->present(sc, finished);
		}
	}
}app;

int main(int argc, char** args)
{
	std::filesystem::path engine_path = getenv("FLAME_PATH");

	app.w = Window::create("Graphics Test", Vec2u(800, 600), WindowFrame | WindowResizable);
	app.d = Device::create(true);
	app.sc = Swapchain::create(app.d, app.w);
	app.img = Image::create_from_file(app.d, (engine_path / L"art/9.png").c_str(), ImageUsageTransferSrc, false);
	app.img->change_layout(ImageLayoutShaderReadOnly, ImageLayoutTransferSrc);
	app.on_resize();
	app.w->add_resize_listener([](Capture&, const Vec2u&) {
		app.on_resize();
	}, Capture());

	looper().loop([](Capture&) {
	}, Capture());
}
