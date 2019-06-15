#include "share.h"
#include "grid.h"
#include "detail.h"

#include <flame/img.h>
#include <flame/system.h>
#include <flame/worker.h>
#include <flame/UI/icon.h>
#include <flame/UI/dialogs/input_dialog.h>
#include <flame/UI/styles/button_style_color.h>

#include <mutex>

using namespace flame;

std::vector<Pic*> grid_pic_candidates;
Pic *grid_slots[grid_hori_pic_cnt * grid_vert_pic_cnt];
UI::Layout *w_grids;
int grid_curr_page = 0;
int grid_total_page = 0;
UI::Layout *w_page_ctrl;
UI::Button *w_page_prev;
UI::Button *w_page_next;
UI::Button *w_page;

void page_change()
{
	w_page->set_text_and_size((std::to_wstring(grid_curr_page + 1) + L"/" + std::to_wstring(grid_total_page)).c_str());
	clear_grids();
	for (auto i = 0; i < grid_hori_pic_cnt * grid_vert_pic_cnt; i++)
	{
		auto id = grid_curr_page * grid_hori_pic_cnt * grid_vert_pic_cnt + i;
		if (id > (int)grid_pic_candidates.size() - 1)
			break;

		grid_slots[i] = grid_pic_candidates[id];
	}
	create_grids();
}

void create_grid_widgets()
{
	for (auto i = 0; i < grid_hori_pic_cnt * grid_vert_pic_cnt; i++)
		grid_slots[i] = nullptr;

	w_grids = new UI::Layout(ui);
	w_grids->pos = Vec2(175.f, 50.f);
	w_grids->layout_type = UI::LayoutGrid;
	w_grids->item_padding = 8.f;
	w_grids->grid_hori_count = grid_hori_pic_cnt;
	ui->root()->add_widget(-1, w_grids);

	w_page_ctrl = new UI::Layout(ui);
	w_page_ctrl->pos = Vec2(175.f, 820.f);
	w_page_ctrl->layout_type = UI::LayoutHorizontal;

	w_page_prev = new UI::Button(ui);
	UI::set_button_classic(w_page_prev, UI::Icon_CHEVRON_LEFT);
	w_page_prev->align = UI::AlignLittleEnd;
	w_page_ctrl->add_widget(-1, w_page_prev);

	w_page_prev->add_clicked_listener([]() {
		if (grid_curr_page > 0)
		{
			grid_curr_page--;
			page_change();
		}
	});

	w_page_next = new UI::Button(ui);
	UI::set_button_classic(w_page_next, UI::Icon_CHEVRON_RIGHT);
	w_page_next->align = UI::AlignLittleEnd;
	w_page_ctrl->add_widget(-1, w_page_next);

	w_page_next->add_clicked_listener([]() {
		if (grid_curr_page < grid_total_page - 1)
		{
			grid_curr_page++;
			page_change();
		}
	});

	w_page = new UI::Button(ui);
	UI::set_button_classic(w_page, L"0/0");
	w_page->align = UI::AlignLittleEnd;
	w_page_ctrl->add_widget(-1, w_page);

	w_page->add_clicked_listener([]() {
		new UI::InputDialog(ui, L"Page", -1.f, [](bool ok, const wchar_t *input) {
			if (ok)
			{
				auto page = std::stoi(input);
				if (page > 0 && page <= grid_total_page)
				{
					grid_curr_page = page - 1;
					page_change();
				}
			}
		});
	});

	ui->root()->add_widget(-1, w_page_ctrl);
}

void clear_grids()
{
	clear_works();
	app->clear_delay_events();

	w_grids->clear_widgets(0, -1, true);
	for (auto i = 0; i < grid_hori_pic_cnt * grid_vert_pic_cnt; i++)
	{
		auto p = grid_slots[i];
		if (p)
		{
			ui->set_imageview(1 + i, nullptr);
			if (p->img_thumbnail)
				graphics::Image::destroy(p->img_thumbnail);
			p->img_thumbnail = nullptr;
			p->w_img = nullptr;
			grid_slots[i] = nullptr;
		}
	}
}

void create_grids()
{
	for (auto i = 0; i < grid_hori_pic_cnt * grid_vert_pic_cnt; i++)
	{
		auto p = grid_slots[i];
		if (p)
		{
			add_work([p, i]() {
				auto img = get_thumbnai(100, p->filename.c_str());

				app->add_delay_event([p, img, i]() {
					p->img_thumbnail = graphics::Image::create_from_img(d, img);
					ui->set_imageview(1 + i, graphics::Imageview::get(p->img_thumbnail));
					p->w_img->inner_padding[0] = (100.f - img->size.x) * 0.5f;
					p->w_img->inner_padding[1] = p->w_img->inner_padding[0];
					p->w_img->inner_padding[2] = (100.f - img->size.y) * 0.5f;
					p->w_img->inner_padding[3] = p->w_img->inner_padding[2];
					destroy_img(img);
				});
			});

			p->w_img = new UI::Image(ui);
			p->w_img->size = Vec2(100.f);
			p->w_img->align = UI::AlignLittleEnd;
			p->w_img->background_offset = Vec4(5.f);
			p->w_img->background_col = Bvec4(255, 255, 255, 0);
			p->w_img->add_style_T<UI::ButtonStyleColor>(0, Vec3(280.f, 0.5f, 1.f));
			p->w_img->id = i + 1;
			w_grids->add_widget(-1, p->w_img, true);

			p->w_img->add_clicked_listener([i]() {
				auto p = grid_slots[i];
				if (!p)
					return;

				w_detail_bg->size = ui->root()->size;
				w_detail_bg->set_visibility(true);
				ui->set_focus_widget(w_detail_bg);

				show_detail_pic(p);
			});
		}
	}
}

void set_grid_page_text()
{
	w_page->set_text_and_size((std::to_wstring(grid_curr_page + 1) + L"/" + std::to_wstring(grid_total_page)).c_str());
}
