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

#include "share.h"
#include "tag.h"
#include "tags_list.h"
#include "grid.h"
#include "detail.h"

#include <flame/file.h>
#include <flame/system.h>
#include <flame/window.h>
#include <flame/UI/icon.h>
#include <flame/UI/dialog.h>
#include <flame/UI/widgets/sizedrag.h>
#include <flame/UI/styles/button_style_color.h>

using namespace flame;

Pic *detail_curr_pic;
flame::graphics::Image *detail_curr_image;
flame::UI::Layout *w_detail_bg;
flame::UI::Image *w_detail_img;
flame::UI::Text *w_name;
flame::UI::Layout *w_tagsbar;

DetailTagWidget::DetailTagWidget(UI::Instance *ui) :
	UI::Layout(ui)
{
	background_col.w = 100;
	align = UI::AlignLittleEnd;
	layout_type = UI::LayoutHorizontal;

	t = new UI::Text(ui);
	t->align = UI::AlignLittleEnd;
	add_widget(-1, t);

	b = new UI::Button(ui);
	b->align = UI::AlignLittleEnd;
	b->background_col.w = 0.f;
	b->add_style_T<UI::ButtonStyleColor>(0, Vec3(280.f, 0.5f, 1.f));
	b->text_col = Bvec4(255, 0, 0, 255);
	b->set_text_and_size(UI::Icon_TIMES);
	add_widget(-1, b);
}

void create_detail_widgets()
{
	w_detail_bg = new UI::Layout(ui);
	w_detail_bg->background_col = Bvec4(0, 0, 0, 200);
	w_detail_bg->event_attitude = UI::EventAccept;
	w_detail_bg->want_key_focus = true;
	w_detail_bg->visible = false;

	w_detail_bg->add_keydown_listener([](int key) {
		switch (key)
		{
		case Key_Esc:
			close_detail();
			break;
		case Key_Left:
			detail_prev();
			break;
		case Key_Right:
			detail_next();
			break;
		case Key_A:
			add_tag_for_pic(detail_curr_pic);
			break;
		case Key_Del:
			detail_delete();
			break;
		case Key_Space:
			exec(L"open", L"https://www.google.com/imghp", false);
			open_explorer_and_select(std::filesystem::path(
				detail_curr_pic->filename).make_preferred().wstring().c_str());
			break;
		}
	});

	w_detail_bg->add_mousescroll_listener([](int scroll) {
		auto old_scale = w_detail_img->scale;
		auto p = (Vec2(ui->window()->mouse_pos) - w_detail_img->pos) / old_scale;
		if (scroll > 0)
		{
			w_detail_img->scale += 0.1f;
			w_detail_img->scale = min(10.f, w_detail_img->scale);
		}
		else
		{
			w_detail_img->scale -= 0.1f;
			w_detail_img->scale = max(0.1f, w_detail_img->scale);
		}
		w_detail_img->pos -= p * (w_detail_img->scale - old_scale);
	});

	w_detail_bg->add_mousemove_listener([](const Vec2 &disp) {
		if (w_detail_bg == ui->dragging_widget())
			w_detail_img->pos += disp;
	});

	w_detail_img = new UI::Image(ui);
	w_detail_img->id = 127;
	w_detail_bg->add_widget(-1, w_detail_img);

	w_detail_img->add_mousemove_listener([](const Vec2 &disp) {
		if (w_detail_img == ui->dragging_widget())
			w_detail_img->pos += disp;
	});

	w_detail_img->add_mousescroll_listener([](int scroll) {
		w_detail_bg->on_mousescroll(scroll);
	});

	auto close_btn = new UI::Button(ui);
	UI::set_button_classic(close_btn, UI::Icon_TIMES);
	close_btn->align = UI::AlignFloatRightTopNoPadding;
	w_detail_bg->add_widget(-1, close_btn);

	close_btn->add_clicked_listener([]() {
		close_detail();
	});

	auto prev_btn = new UI::Button(ui);
	UI::set_button_classic(prev_btn, UI::Icon_CHEVRON_LEFT);
	prev_btn->align = UI::AlignFloatLeftNoPadding;
	w_detail_bg->add_widget(-1, prev_btn);

	prev_btn->add_clicked_listener([]() {
		detail_prev();
	});

	auto next_btn = new UI::Button(ui);
	UI::set_button_classic(next_btn, UI::Icon_CHEVRON_RIGHT);
	next_btn->align = UI::AlignFloatRightNoPadding;
	w_detail_bg->add_widget(-1, next_btn);

	next_btn->add_clicked_listener([]() {
		detail_next();
	});

	auto del_btn = new UI::Button(ui);
	UI::set_button_classic(del_btn, L"DELETE");
	del_btn->align = UI::AlignFloatBottomNoPadding;
	w_detail_bg->add_widget(-1, del_btn);

	del_btn->add_clicked_listener([]() {
		detail_delete();
	});

	auto w_topbar = new UI::Layout(ui);
	w_topbar->align = UI::AlignFloatTopNoPadding;
	w_topbar->layout_type = UI::LayoutVertical;
	w_topbar->item_padding = 4.f;
	w_detail_bg->add_widget(-1, w_topbar);

	w_name = new UI::Text(ui);
	w_name->align = UI::AlignMiddle;
	w_topbar->add_widget(-1, w_name);

	w_tagsbar = new UI::Layout(ui);
	w_tagsbar->align = UI::AlignMiddle;
	w_tagsbar->layout_type = UI::LayoutHorizontal;
	w_tagsbar->item_padding = 8.f;
	w_topbar->add_widget(-1, w_tagsbar);

	ui->root()->add_widget(-1, w_detail_bg);
}

void show_detail_pic(Pic *p)
{
	if (detail_curr_pic)
		graphics::Image::destroy(detail_curr_image);

	detail_curr_pic = p;
	detail_curr_image = graphics::Image::create_from_file(d, p->filename.c_str());
	ui->set_imageview(127, graphics::Imageview::get(detail_curr_image));
	w_detail_img->scale = 1.f;
	w_detail_img->size = Vec2(detail_curr_image->size);
	w_detail_img->pos = (w_detail_bg->size - w_detail_img->size) * 0.5f;

	refresh_detail_topbar(p);
}

void close_detail()
{
	ui->set_imageview(127, nullptr);
	graphics::Image::destroy(detail_curr_image);
	detail_curr_pic = nullptr;
	detail_curr_image = nullptr;

	w_detail_bg->set_visibility(false);
};

bool detail_prev()
{
	for (auto it = grid_pic_candidates.begin(); it != grid_pic_candidates.end(); it++)
	{
		if (*it == detail_curr_pic)
		{
			if (it != grid_pic_candidates.begin())
			{
				it--;
				show_detail_pic(*it);
				return true;
			}
			return false;
		}
	}
	return false;
}

bool detail_next()
{
	for (auto it = grid_pic_candidates.begin(); it != grid_pic_candidates.end(); it++)
	{
		if (*it == detail_curr_pic)
		{
			it++;
			if (it != grid_pic_candidates.end())
			{
				show_detail_pic(*it);
				return true;
			}
			return false;
		}
	}
	return false;
}

void detail_delete()
{
	auto p = detail_curr_pic;
	if (!detail_next())
		close_detail();
	for (auto it = grid_pic_candidates.begin(); it != grid_pic_candidates.end(); it++)
	{
		if (*it == p)
		{
			grid_pic_candidates.erase(it);
			break;
		}
	}
	for (auto i = 0; i < grid_hori_pic_cnt * grid_vert_pic_cnt; i++)
	{
		if (grid_slots[i] == p)
			grid_slots[i] = nullptr;
	}
	move_to_trashbin(p->filename.c_str());
	for (auto it = pics.begin(); it != pics.end(); it++)
	{
		if (it->get() == p)
		{
			delete_pic(p);
			pics.erase(it);
			break;
		}
	}
	update_all_tags();
}

void refresh_detail_topbar(Pic *p)
{
	w_name->set_text_and_size(p->filename.c_str());

	w_tagsbar->clear_widgets(0, -1, true);
	if (!p->tags.empty() || std::filesystem::path(p->filename).stem().wstring()[0] != L'~')
	{
		for (auto &t : p->tags)
		{
			auto w = new DetailTagWidget(ui);
			w->t->set_text_and_size(t->name.c_str());
			w_tagsbar->add_widget(-1, w, true);

			w->b->add_clicked_listener([p, t]() {
				p->remove_tag(t);
				t->remove_pic(p);

				auto prev_filename = p->filename;
				p->make_filename_from_tags();
				std::filesystem::rename(prev_filename, p->filename);

				update_all_tags();
				refresh_detail_topbar(p);
			});
		}
	}

	auto w_add = new UI::Button(ui);
	w_add->inner_padding = Vec4(4.f, 4.f, 2.f, 2.f);
	w_add->align = UI::AlignLittleEnd;
	w_add->set_text_and_size(UI::Icon_PLUS);
	w_add->add_style_T<UI::ButtonStyleColor>(0, Vec3(0.f, 1.f, 1.f));
	w_tagsbar->add_widget(-1, w_add, true);

	w_add->add_clicked_listener([p]() {
		add_tag_for_pic(p);
	});
}

struct AddTagsDialogItem : UI::ListItem
{
	Tag *t;
	UI::Checkbox *w_c;

	AddTagsDialogItem(UI::Instance *ui) :
		UI::ListItem(ui)
	{
		w_c = new UI::Checkbox(ui);
		w_c->align = UI::AlignLittleEnd;
		w_c->checked = false;
		w_c->add_style_T<UI::ButtonStyleColor>(0, Vec3(0.f, 0.f, 0.7f));
		add_widget(0, w_c);
	}
};

struct AddTagsDialog : UI::Dialog
{
	TagsListLayout *w_list;
	UI::Layout *w_buttons;
	UI::Button *w_ok;
	UI::Button *w_cancel;

	std::vector<AddTagsDialogItem*> items;

	AddTagsDialog(UI::Instance *ui, Pic *p) :
		UI::Dialog(ui, L"Add Tags", -1.f, true)
	{
		event_attitude = UI::EventBlackHole;
		want_key_focus = true;

		w_title->background_col = Bvec4(200, 40, 20, 255);
		sizedrag->min_size = Vec2(300.f, 400.f);
		set_size(sizedrag->min_size);

		w_content->size_policy_hori = UI::SizeFitLayout;
		w_content->size_policy_vert = UI::SizeFitLayout;
		w_content->layout_type = UI::LayoutVertical;
		w_content->item_padding = 4.f;

		w_list = new TagsListLayout(ui);
		for (auto &t : tags)
		{
			auto item = new AddTagsDialogItem(ui);
			item->t = t.get();
			item->w_btn->set_text_and_size(t->name.c_str());
			w_list->list->add_item(item, true);
			items.push_back(item);
		}

		w_list->filter->add_changed_listener([this]() {
			auto l = w_list->list->w_items;
			for (auto i = 0; i < l->widget_count(); i++)
			{
				auto item = (AddTagsDialogItem*)l->widget(i);
				item->visible = (item->t->name.find(w_list->filter->text.data)
					!= std::wstring::npos);
			}
			l->arrange();
		});

		w_content->add_widget(-1, w_list);

		w_buttons = new UI::Layout(ui);
		w_buttons->align = UI::AlignMiddle;
		w_buttons->layout_type = UI::LayoutHorizontal;
		w_buttons->item_padding = 4.f;

		w_ok = new UI::Button(ui);
		w_ok->inner_padding = Vec4(4.f, 4.f, 2.f, 2.f);
		w_ok->align = UI::AlignLittleEnd;
		w_ok->set_text_and_size(L"OK");
		w_ok->add_style_T<UI::ButtonStyleColor>(0, Vec3(0.f, 1.f, 1.f));
		w_buttons->add_widget(-1, w_ok);

		w_cancel = new UI::Button(ui);
		w_cancel->inner_padding = Vec4(4.f, 4.f, 2.f, 2.f);
		w_cancel->align = UI::AlignLittleEnd;
		w_cancel->set_text_and_size(L"Cancel");
		w_cancel->add_style_T<UI::ButtonStyleColor>(0, Vec3(0.f, 1.f, 1.f));
		w_buttons->add_widget(-1, w_cancel);

		w_ok->add_clicked_listener([this, p]() {
			for (auto &i : items)
			{
				if (i->w_c->checked)
				{
					if (!p->has_tag(i->t))
					{
						p->tags.push_back(i->t);
						i->t->pics.push_back(p);
					}
				}
			}

			auto prev_filename = p->filename;
			p->make_filename_from_tags();
			std::filesystem::rename(prev_filename, p->filename);

			update_all_tags();
			refresh_detail_topbar(p);

			layout()->remove_widget(this, true);
			::ui->set_focus_widget(w_detail_bg);
		});

		w_cancel->add_clicked_listener([this]() {
			layout()->remove_widget(this, true);
			::ui->set_focus_widget(w_detail_bg);
		});

		w_content->add_widget(-1, w_buttons);

		pos = (Vec2(ui->window()->size) - size) * 0.5f;

		ui->root()->add_widget(-1, this, true, [this]() {
			auto ui = instance();
			ui->set_focus_widget(w_list->filter);
			ui->set_dragging_widget(nullptr);
		});
	}
};

void add_tag_for_pic(Pic *p)
{
	new AddTagsDialog(ui, p);
}
