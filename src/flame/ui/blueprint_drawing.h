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

#include <flame/blueprint.h>
#include <flame/UI/UI.h>

namespace flame
{
	namespace ui
	{
		struct Instance;
		struct wLayout;

		//struct BP_Scene_Draw
		//{
		//	enum SelType
		//	{
		//		SelTypeNone,
		//		SelTypeNode,
		//		SelTypeLink
		//	};

		//	wLayout *w_scene;

		//	FLAME_UI_EXPORTS void get_sel(SelType &type, void **v) const;
		//	FLAME_UI_EXPORTS void set_sel(SelType type, void *v);

		//	FLAME_UI_EXPORTS static BP_Scene_Draw *create(blueprint::Scene *s, Instance *ui);
		//	FLAME_UI_EXPORTS static void destroy(BP_Scene_Draw *d);
		//};
	}
}

