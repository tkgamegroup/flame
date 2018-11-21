// MIT License
// 
// Copyright (c) 2018 wjs
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <flame/serialize.h>
#include <flame/graphics/image.h>
#include <flame/ui/style.h>
#include <flame/3d/model.h>
#include <flame/3d/camera.h>

#include <flame/basic_app.h>

using namespace flame;

BasicApp app;

void add_wall(const Vec3 &pos, const Vec3 &vx, float height)
{
	auto m = _3d::Model::create();
	m->add_plane(pos + Vec3(0.f, height, 0.f), vx, Vec3(0.f, -height, 0.f));
	app.scene_3d->register_model(m);
}

void add_ceil(const Vec3 &pos, float width, float depth)
{
	auto m = _3d::Model::create();
	m->add_plane(pos + Vec3(0.f, 0.f, depth), Vec3(width, 0.f, 0.f), Vec3(0.f, 0.f, -depth));
	app.scene_3d->register_model(m);
}

void add_floor(const Vec3 &pos, float width, float depth)
{
	auto m = _3d::Model::create();
	m->add_plane(pos, Vec3(width, 0.f, 0.f), Vec3(0.f, 0.f, depth));
	app.scene_3d->register_model(m);
}

extern "C" __declspec(dllexport) int main()
{
	auto res = Ivec2(1280, 720);
	const auto image_id = 99;

	app.create("Scene Test", res, WindowFrame, true);
	app.t_fps->text_col() = Bvec4(255);
	app.ui->set_imageview(image_id, graphics::Imageview::get(app.scene_3d->get_col_image()));

	auto w_image = ui::wImage::create(app.ui);
	w_image->size$ = res;
	w_image->align$ = ui::AlignLittleEnd;
	w_image->id() = image_id;
	app.ui->root()->add_child(w_image);

	auto w_ratio = ui::wEdit::create(app.ui);
	w_ratio->add_char_filter_float();
	w_ratio->set_size_by_width(100.f);
	app.ui->root()->add_child(w_ratio, 1);

	auto w_set = ui::wButton::create(app.ui);
	w_set->pos$ = Vec2(108.f, 0.f);
	w_set->set_classic(L"set bake props");
	app.ui->root()->add_child(w_set, 1);

	w_set->add_listener(cH("clicked"), [](CommonData *d) {
		auto w_e = (ui::wEdit*)d[0].p;

		app.scene_3d->set_bake_props(stof(w_e->text()), Ivec2(512));
		{
			auto bk_pen = app.scene_3d->get_bake_pen_pos();
			printf("bake pen: %d, %d\n", bk_pen.x, bk_pen.y);
		}
	}, "p:edit", w_ratio);

	auto w_bake = ui::wButton::create(app.ui);
	w_bake->pos$ = Vec2(200.f, 0.f);
	w_bake->set_classic(L"bake");
	app.ui->root()->add_child(w_bake, 1);

	w_bake->add_listener(cH("clicked"), [](CommonData *d) {
		app.scene_3d->bake(1);
	}, "");

	auto w_showmode = ui::wCombo::create(app.ui);
	w_showmode->pos$ = Vec2(0.f, 20.f);
	ui::add_style_color(w_showmode, 0, Vec3(0.f, 0.f, 0.7f));

	w_showmode->w_items()->add_child(ui::wMenuItem::create(app.ui, L"light map"));
	w_showmode->w_items()->add_child(ui::wMenuItem::create(app.ui, L"camera light"));

	w_showmode->add_listener(cH("changed"), [](CommonData *d) {
		auto thiz = (ui::wCombo*)d[0].p;

		app.scene_3d->set_show_mode(thiz->sel() == 0 ? _3d::ShowModeLightmap : _3d::ShowModeCameraLight);
	}, "p:this", w_showmode);

	w_showmode->set_sel(0);

	app.ui->root()->add_child(w_showmode);

	auto w_showframe = ui::wCheckbox::create(app.ui);
	w_showframe->pos$ = Vec2(0.f, 40.f);
	w_showframe->checked() = 1;
	app.ui->root()->add_child(w_showframe, 1);

	auto t_showframe = ui::wText::create(app.ui);
	t_showframe->pos$ = Vec2(20.f, 40.f);
	t_showframe->set_text(L"show frame");
	t_showframe->text_col() = Bvec4(255);
	app.ui->root()->add_child(t_showframe, 1);

	w_showframe->add_listener(cH("clicked"), [](CommonData *d) {
		auto thiz = (ui::wCheckbox*)d[0].p;

		app.scene_3d->set_show_frame(thiz->checked());
	}, "p:this", w_showframe);

	auto c = _3d::Camera::create(60.f, (float)res.x / (float)res.y, 0.1f, 1000.f);
	app.scene_3d->set_camera(c);

	add_wall(Vec3(-1.f, 0.f, -3.f), Vec3(2.f, 0.f, 0.f), 2.f);
	add_wall(Vec3(1.f, 0.f, -3.f), Vec3(0.f, 0.f, 2.f), 2.f);
	add_wall(Vec3(-1.f, 0.f, -1.f), Vec3(0.f, 0.f, -2.f), 2.f);
	add_wall(Vec3(1.f, 0.f, -1.f), Vec3(-1.5f, 0.f, 0.f), 2.f);
	add_ceil(Vec3(-1.f, 2.f, -3.f), 2.f, 2.f);
	add_floor(Vec3(-1.f, 0.f, -3.f), 2.f, 2.f);

	app.scene_3d->set_bake_props(16.f, Ivec2(512));
	{
		auto bk_pen = app.scene_3d->get_bake_pen_pos();
		printf("bake pen: %d, %d\n", bk_pen.x, bk_pen.y);
	}

	w_image->want_key_focus$ = true;
	app.ui->set_key_focus_widget(w_image);
	w_image->add_listener(cH("key down"), _3d::Camera::pf_keydown, "this:camera", c);
	w_image->add_listener(cH("key up"), _3d::Camera::pf_keyup, "this:camera", c);

	app.run();

	return 0;
}
