#include <flame/foundation/serialize.h>
#include <flame/foundation/blueprint.h>
#include <flame/network/network.h>
#include <flame/graphics/device.h>
#include <flame/graphics/synchronize.h>
#include <flame/graphics/swapchain.h>
#include <flame/graphics/commandbuffer.h>
#include <flame/graphics/image.h>
#include <flame/graphics/font.h>
#include <flame/graphics/canvas.h>
#include <flame/universe/default_style.h>
#include <flame/universe/components/element.h>
#include <flame/universe/components/text.h>
#include <flame/universe/components/edit.h>
#include <flame/universe/components/image.h>
#include <flame/universe/components/event_dispatcher.h>
#include <flame/universe/components/event_receiver.h>
#include <flame/universe/components/aligner.h>
#include <flame/universe/components/layout.h>
#include <flame/universe/components/style.h>
#include <flame/universe/components/checkbox.h>
#include <flame/universe/components/combobox.h>
#include <flame/universe/components/tree.h>
#include <flame/universe/components/scrollbar.h>
#include <flame/universe/components/window.h>

#include "app.h"
#include "window/resource_explorer.h"

void App::create()
{
	w = Window::create("Editor", Vec2u(1, 1), WindowFrame | WindowResizable);
	w->set_maximized(true);
	d = Device::create(true);
	render_finished = Semaphore::create(d);
	scr = SwapchainResizable::create(d, w);
	fence = Fence::create(d);
	auto sc = scr->sc();
	canvas = Canvas::create(d, TargetImages, &sc->images());
	sc_cbs.resize(sc->images().size());
	for (auto i = 0; i < sc_cbs.size(); i++)
		sc_cbs[i] = Commandbuffer::create(d->gcp);

	auto font14 = Font::create(L"c:/windows/fonts/msyh.ttc", 14);
	auto font_awesome14 = Font::create(L"../asset/font_awesome.ttf", 14);
	auto font32 = Font::create(L"c:/windows/fonts/msyh.ttc", 32);
	auto font_awesome32 = Font::create(L"../asset/font_awesome.ttf", 32);
	font_atlas_pixel = FontAtlas::create(d, FontDrawPixel, { font14, font_awesome14 });
	font_atlas_sdf = FontAtlas::create(d, FontDrawSdf, { font32, font_awesome32 });
	font_atlas_pixel->index = 1;
	font_atlas_sdf->index = 2;
	canvas->set_image(font_atlas_pixel->index, Imageview::create(font_atlas_pixel->image(), Imageview2D, 0, 1, 0, 1, SwizzleOne, SwizzleOne, SwizzleOne, SwizzleR));
	canvas->set_image(font_atlas_sdf->index, Imageview::create(font_atlas_sdf->image()));
	canvas->set_clear_color(Vec4c(100, 100, 100, 255));
	default_style.set_to_light();

	universe_serialization_set_data("font_atlas1", app.font_atlas_pixel);

	root = Entity::create();
	{
		c_element_root = cElement::create(canvas);
		root->add_component(c_element_root);

		root->add_component(cEventDispatcher::create(w));

		root->add_component(cLayout::create());
	}

	auto e_fps = Entity::create();
	root->add_child(e_fps);
	{
		e_fps->add_component(cElement::create());

		auto c_text = cText::create(font_atlas_pixel);
		c_text_fps = c_text;
		e_fps->add_component(c_text);

		auto c_aligner = cAligner::create();
		c_aligner->x_align = AlignxLeft;
		c_aligner->y_align = AlignyBottom;
		e_fps->add_component(c_aligner);
	}

	open_resource_explorer(Vec2f(20.f));
}

void App::run()
{
	auto sc = scr->sc();
	auto sc_frame = scr->sc_frame();

	if (sc_frame > rt_frame)
	{
		canvas->set_render_target(TargetImages, sc ? &sc->images() : nullptr);
		rt_frame = sc_frame;
	}

	if (sc)
	{
		sc->acquire_image();
		fence->wait();

		c_element_root->width = w->size.x();
		c_element_root->height = w->size.y();
		c_text_fps->set_text(std::to_wstring(looper().fps));
		root->update();

		std::vector<Commandbuffer*> cbs;
		{
			auto img_idx = sc->image_index();
			auto cb = sc_cbs[img_idx];
			canvas->record(cb, img_idx);
			cbs.push_back(cb);
		}
		cbs.insert(cbs.begin(), extra_cbs.begin(), extra_cbs.end());
		extra_cbs.clear();

		d->gq->submit(cbs, sc->image_avalible(), render_finished, fence);
		d->gq->present(sc, render_finished);
	}
}

App app;

int main(int argc, char **args)
{
	app.create();

	looper().loop([](void* c) {
		auto app = (*(App**)c);
		app->run();
	}, new_mail_p(&app));

	return 0;
}
