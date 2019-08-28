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
