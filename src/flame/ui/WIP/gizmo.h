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

#include "instance.h"

namespace flame
{
	namespace ui
	{
		struct GizmoPrivate;

		struct Gizmo
		{
			enum TransType
			{
				TransMove,
				TransRotate,
				TransScale
			};

			enum Mode
			{
				ModeLocal,
				ModeWorld
			};

			Mode mode;

			bool enable;

			bool enable_snap;
			Vec3 move_snap;
			float rotate_snap;
			float scale_snap;

			GizmoPrivate *_priv;

			FLAME_UI_EXPORTS bool is_using() const;
			FLAME_UI_EXPORTS bool show(Instance *ui, TransType type, graphics::Camera *camera, ThreeDWorld::Object *o);
		};

		FLAME_UI_EXPORTS Gizmo *create_gizmo();
		FLAME_UI_EXPORTS void destroy_gizmo(Gizmo *g);
	}
}
