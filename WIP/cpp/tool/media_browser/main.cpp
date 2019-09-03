int main(int argc, char **args)
{
	Ivec2 res(1280, 720);

	app = Application::create();
	auto w = Window::create(app, "Media Browser", res, WindowFrame | WindowResizable);
	w->set_maximized(true);

	d = graphics::Device::create(true);

	auto sc = graphics::Swapchain::create(d, w);

	auto image_avalible = graphics::Semaphore::create(d);
	auto ui_finished = graphics::Semaphore::create(d);

	ui = UI::Instance::create(d, sc, graphics::SampleCount_8);
	auto canvas = UI::Canvas::create(d);

	load_tags();
	load_pics();

	create_tags_list();
	create_grid_widgets();
	create_detail_widgets();

	srand(time(0));

	auto t_fps = new UI::Text(ui);
	t_fps->align = UI::AlignFloatRightBottomNoPadding;
	ui->root()->add_widget(-1, t_fps);

	app->run([&](){
		if (!w->minimized)
		{
			auto index = sc->acquire_image(image_avalible);

			ui->begin(app->elapsed_time);
			ui->end(canvas);
			canvas->record_cb(index);

			d->gq->submit(canvas->get_cb(), image_avalible, ui_finished);
			d->gq->wait_idle();
			d->gq->present(index, sc, ui_finished);

			static wchar_t buf[16];
			swprintf(buf, L"FPS:%lld", app->fps);
			t_fps->set_text_and_size(buf);
		}
	});

	return 0;
}
