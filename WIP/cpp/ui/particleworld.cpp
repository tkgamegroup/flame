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

#include "../widget.h"
#include "../instance.h"
#include "../../blueprint.h"
#include "../../graphics/device.h"
#include "../../graphics/image.h"

using namespace flame;

#define t_name "Particle World"

static UI::WidgetTypeInfo type_info;

struct ParticleWorld : UI::Widget
{
	graphics::Device *d;
	UI::Instance *ui;

	MediumString shader_filename;
	MediumString blueprint_filename;

	graphics::Image *t;
	graphics::Imageview *v;
	int img_idx;

	virtual void show(const Vec2 &off) override
	{
		ui->set_cursor_pos(off + pos);
		ui->image(img_idx, size);
	}

	virtual void show_in_editor() override
	{

	}
};

UI::Widget *create(graphics::Device *d, UI::Instance *ui)
{
	auto w = new ParticleWorld;
	w->type_info = &type_info;
	w->d = d;
	w->ui = ui;
	
	w->img_size = Vec2(0.f);
	return w;
}

void destroy(UI::Widget *w)
{
	delete w;
}

str get_name()
{
	return t_name;
}

int get_BP_inputslot_count()
{
	return 1;
}

void get_BP_inputslot_data(int index, str &name, blueprint::InputslotType &type)
{
	switch (index)
	{
	case 0:
		name = "Style";
		type = blueprint::InputslotLink;
		break;
	}
}

int get_BP_outputslot_count()
{
	return 0;
}

void get_BP_outputslot_data(int index, str &name)
{
	switch (index)
	{
	}
}

void bp_update(float elapsed_time, void *ptr, blueprint::Node *n)
{

}

extern "C" _declspec(dllexport) UI::WidgetTypeInfo *init()
{
	type_info.name = t_name;
	type_info.hash = CHASH(t_name);
	type_info.icon = ICON_PTCWRD;

	type_info.create = create;
	type_info.destroy = destroy;

	type_info.bp.get_name = get_name;
	type_info.bp.get_inputslot_count = get_BP_inputslot_count;
	type_info.bp.get_inputslot_data = get_BP_inputslot_data;
	type_info.bp.get_outputslot_count = get_BP_outputslot_count;
	type_info.bp.get_outputslot_data = get_BP_outputslot_data;

	type_info.bp_update = bp_update;

	return &type_info;
}
