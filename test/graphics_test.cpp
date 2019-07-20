#include <flame/basic_app.h>

using namespace flame;

struct App : BasicApp
{
	BP* bp;
	std::vector<void*>* cbs;

	virtual void do_run() override
	{
		auto idx = frame % FLAME_ARRAYSIZE(fences);
		auto sc = (graphics::Swapchain*)psc->v;

		if (sc)
		{
			if (!cbs->empty())
				sc->acquire_image(image_avalible);

			fences[idx]->wait();

			if (!cbs->empty())
			{
				d->gq->submit((graphics::Commandbuffer*)((*cbs)[sc->image_index()]), image_avalible, render_finished, fences[idx]);

				d->gq->present(sc, render_finished);
			}
		}

		bp->update();

		frame++;
	}

}app;
auto papp = &app;

int main(int argc, char** args)
{
	typeinfo_load(L"flame_foundation.typeinfo");
	typeinfo_load(L"flame_graphics.typeinfo");

	app.bp = BP::create_from_file(L"../renderpath/full_screen_shader/bp");
	if (!app.bp)
	{
		printf("bp not found, exit\n");
		return 0;
	}

	app.create("", Vec2u(1280, 720), WindowFrame);

	app.bp->find_input("sc.window")->set_data(&app.w);
	app.bp->update();

	app.psc = (AttributeP<void>*)app.bp->find_output("sc.out")->data();
	app.cbs = &((AttributeV<std::vector<void*>>*)app.bp->find_output("cbs.out")->data())->v;

	app.run();
}
