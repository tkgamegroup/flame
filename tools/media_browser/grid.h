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

#include "pic.h"

#include <flame/UI/layout.h>
#include <flame/UI/widgets/button.h>

#include <vector>

const auto grid_hori_pic_cnt = 16;
const auto grid_vert_pic_cnt = 7;
extern std::vector<Pic*> grid_pic_candidates;
extern Pic *grid_slots[grid_hori_pic_cnt * grid_vert_pic_cnt];
extern flame::UI::Layout *w_grids;
extern int grid_curr_page;
extern int grid_total_page;
extern flame::UI::Layout *w_page_ctrl;
extern flame::UI::Button *w_page_prev;
extern flame::UI::Button *w_page_next;
extern flame::UI::Button *w_page;

void create_grid_widgets();
void clear_grids();
void create_grids();
void set_grid_page_text();
