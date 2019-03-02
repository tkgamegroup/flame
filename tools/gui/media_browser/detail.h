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
