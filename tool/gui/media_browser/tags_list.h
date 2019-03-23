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

#include "tag.h"

#include <flame/UI/widget.h>

struct TagsListLayout : flame::UI::Layout
{
	flame::UI::Layout *top;
	flame::UI::Button *clear;
	flame::UI::Edit *filter;
	flame::UI::List *list;

	TagsListLayout(flame::UI::Instance *ui);
};

struct TagsListItem : flame::UI::ListItem
{
	flame::UI::Checkbox *w_c;
	flame::UI::Layout *w_right;
	flame::UI::Text *w_t;
	flame::UI::Button *w_edt;
	flame::UI::Button *w_del;
	Tag *t;

	TagsListItem(flame::UI::Instance *ui, Tag *_t);
};

void create_tags_list();
void refresh_tags_list();
void update_tag(TagsListItem *item);
void update_all_tags();
