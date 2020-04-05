#include <flame/serialize.h>
#include <flame/foundation/foundation.h>
#include <flame/graphics/device.h>
#include <flame/graphics/swapchain.h>
#include <flame/graphics/synchronize.h>
#include <flame/graphics/commandbuffer.h>
#include <flame/graphics/renderpass.h>
#include <flame/graphics/image.h>
#include <flame/graphics/shader.h>

using namespace flame;
using namespace graphics;

struct App
{
	SysWindow* w;
	Device* d;
	Swapchain* sc;
	Image* img;
}app;

int main(int argc, char** args)
{
	std::filesystem::path engine_path = getenv("FLAME_PATH");

	app.w = SysWindow::create("Graphics Test", Vec2u(800, 600), WindowFrame);
	app.d = Device::create(true);
	app.sc = Swapchain::create(app.d, app.w, true);
	app.img = Image::create_from_file(app.d, (engine_path / L"art/9.png").c_str(), ImageUsageTransferSrc);
	
	app.sc->acquire_image();

	auto dst = app.sc->image(0);
	auto cb = Commandbuffer::create(app.d->gcp);
	cb->begin();
	cb->change_image_layout(dst, ImageLayoutUndefined, ImageLayoutTransferDst);
	cb->change_image_layout(app.img, ImageLayoutShaderReadOnly, ImageLayoutTransferSrc);
	ImageCopy cpy;
	cpy.size = app.img->size;
	cpy.src_off = 0;
	cpy.dst_off = 0;
	cb->copy_image(app.img, dst, 1, &cpy);
	cb->change_image_layout(dst, ImageLayoutTransferDst, ImageLayoutPresent);
	cb->end();
	auto finished = Semaphore::create(app.d);
	app.d->gq->submit(1, &cb, app.sc->image_avalible(), finished, nullptr);
	
	app.d->gq->present(app.sc, finished);

	looper().loop([](void*) {
	}, Mail());
}
