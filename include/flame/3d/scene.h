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
	namespace graphics
	{
		struct Device;
		struct Image;
		struct Commandbuffer;
	}

	namespace _3d
	{
		FLAME_3D_EXPORTS void init(graphics::Device *d);
		FLAME_3D_EXPORTS void deinit();

		struct Model;
		struct Camera;

		enum ShowMode
		{
			ShowModeLightmap,
			ShowModeCameraLight
		};

		struct Scene
		{
			FLAME_3D_EXPORTS void set_show_mode(ShowMode mode);
			FLAME_3D_EXPORTS void set_show_frame(bool show_frame);

			FLAME_3D_EXPORTS void register_model(Model *m);

			FLAME_3D_EXPORTS void set_camera(Camera *c);

			FLAME_3D_EXPORTS void set_bake_props(float ratio, const Ivec2 &imgsize);
			FLAME_3D_EXPORTS Ivec2 get_bake_pen_pos() const;

			FLAME_3D_EXPORTS graphics::Image *get_col_image() const;
			FLAME_3D_EXPORTS graphics::Image *get_dep_image() const;
			FLAME_3D_EXPORTS graphics::Commandbuffer *get_cb() const;

			FLAME_3D_EXPORTS void begin(float elp_time);
			FLAME_3D_EXPORTS void end();

			FLAME_3D_EXPORTS void record_cb();

			FLAME_3D_EXPORTS void bake(int pass);

			FLAME_3D_EXPORTS static Scene *create(const Ivec2 &resolution);
			FLAME_3D_EXPORTS static void destroy(Scene *s);
		};
	}
}

