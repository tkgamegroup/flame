#pragma once

#include <flame/graphics/image.h>
#include <flame/UI/layout.h>
#include <flame/UI/widgets/text.h>
#include <flame/UI/widgets/button.h>
#include <flame/UI/widgets/image.h>

#include "pic.h"

extern Pic *detail_curr_pic;
extern flame::graphics::Image *detail_curr_image;
extern flame::UI::Layout *w_detail_bg;
extern flame::UI::Image *w_detail_img;

struct DetailTagWidget : flame::UI::Layout
{
	flame::UI::Text *t;
	flame::UI::Button *b;

	DetailTagWidget(flame::UI::Instance *ui);
};

void create_detail_widgets();
void show_detail_pic(Pic *p);
void close_detail();
bool detail_prev();
bool detail_next();
void detail_delete();
void refresh_detail_topbar(Pic *p);
void add_tag_for_pic(Pic *p);
