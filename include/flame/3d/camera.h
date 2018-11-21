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

#include <flame/3d/3d.h>

#include <flame/math.h>

namespace flame
{
	namespace _3d
	{
		struct Camera
		{
			Vec3 pos;
			float x_ang;
			float y_ang;

			float move_speed;
			float turn_speed;

			FLAME_3D_EXPORTS Mat4 proj() const;
			FLAME_3D_EXPORTS Mat4 view() const;

			FLAME_3D_EXPORTS void update(float elp_time);

			FLAME_3D_EXPORTS static void pf_keydown(CommonData *d);
			FLAME_3D_EXPORTS static void pf_keyup(CommonData *d);

			FLAME_3D_EXPORTS static Camera *create(float fovy, float aspect, float zNear, float zFar);
			FLAME_3D_EXPORTS static void destroy(Camera *c);
		};
	}
}
