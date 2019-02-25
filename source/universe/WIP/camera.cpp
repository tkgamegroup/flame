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

#include <flame/3d/camera.h>

#include <flame/window.h>

namespace flame
{
	namespace _3d
	{
		struct CameraPrivate : Camera
		{
			Mat4 proj;
			Mat4 view;

			bool W, S, A, D, Q, E, Z, X, R, C;

			inline CameraPrivate(float fovy, float aspect, float zNear, float zFar)
			{
				proj = get_proj_mat(ANG_RAD * fovy, aspect, zNear, zFar);

				pos = Vec3(0.f);
				x_ang = 0.f;
				y_ang = 0.f;

				move_speed = 1.f;
				turn_speed = 30.f;
				W = S = A = D = Q = E = Z = X = R = C = false;
			}

			inline void update(float elp_time)
			{
				auto turn = Ivec2(0);
				if (A)
					turn.x++;
				if (D)
					turn.x--;
				if (R)
					turn.y--;
				if (C)
					turn.y++;
				if (turn != 0)
				{
					x_ang += float(turn.x) * (elp_time * turn_speed);
					y_ang += float(turn.y) * (elp_time * turn_speed);
				}

				auto view_dir = Vec3(0.f, 0.f, -1.f);
				auto up_dir = Vec3(0.f, 1.f, 0.f);

				auto mat_yaw = Mat3(Vec3(0.f, 1.f, 0.f), ANG_RAD * x_ang);
				auto x_dir = mat_yaw * Vec3(1.f, 0.f, 0.f);
				auto mat_pitch = Mat3(x_dir, ANG_RAD * y_ang);
				view_dir = mat_pitch * mat_yaw * view_dir;
				up_dir = mat_pitch * mat_yaw * up_dir;
				x_dir = mat_pitch * x_dir;

				auto move = Ivec3(0);
				if (W)
					move.x--;
				if (S)
					move.x++;
				if (Q)
					move.z--;
				if (E)
					move.z++;
				if (Z)
					move.y++;
				if (X)
					move.y--;
				if (move != 0)
				{
					auto m = Mat3(-view_dir, up_dir, x_dir);
					pos += m * Vec3(move) * (elp_time * move_speed / move.length());
				}

				view = get_view_mat(pos, pos + view_dir, up_dir);
			}
		};

		Mat4 Camera::proj() const
		{
			return ((CameraPrivate*)this)->proj;
		}

		Mat4 Camera::view() const
		{
			return ((CameraPrivate*)this)->view;
		}

		void Camera::update(float elp_time)
		{
			((CameraPrivate*)this)->update(elp_time);
		}

		void Camera::pf_keydown(CommonData *d)
		{
			auto thiz = (CameraPrivate*)d[1].p();

			switch (d[0].i1())
			{
			case Key_W:
				thiz->W = true;
				break;
			case Key_S:
				thiz->S = true;
				break;
			case Key_A:
				thiz->A = true;
				break;
			case Key_D:
				thiz->D = true;
				break;
			case Key_Q:
				thiz->Q = true;
				break;
			case Key_E:
				thiz->E = true;
				break;
			case Key_Z:
				thiz->Z = true;
				break;
			case Key_X:
				thiz->X = true;
				break;
			case Key_R:
				thiz->R = true;
				break;
			case Key_C:
				thiz->C = true;
				break;
			}
		}

		void Camera::pf_keyup(CommonData *d)
		{
			auto thiz = (CameraPrivate*)d[1].p();

			switch (d[0].i1())
			{
			case Key_W:
				thiz->W = false;
				break;
			case Key_S:
				thiz->S = false;
				break;
			case Key_A:
				thiz->A = false;
				break;
			case Key_D:
				thiz->D = false;
				break;
			case Key_Q:
				thiz->Q = false;
				break;
			case Key_E:
				thiz->E = false;
				break;
			case Key_Z:
				thiz->Z = false;
				break;
			case Key_X:
				thiz->X = false;
				break;
			case Key_R:
				thiz->R = false;
				break;
			case Key_C:
				thiz->C = false;
				break;
			}
		}

		Camera *Camera::create(float fovy, float aspect, float zNear, float zFar)
		{
			return new CameraPrivate(fovy, aspect, zNear, zFar);
		}

		void Camera::destroy(Camera *c)
		{

		}
	}
}
