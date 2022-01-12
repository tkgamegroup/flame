#pragma once

#include "../component.h"

namespace flame
{
	/// Reflect
	struct cCamera : Component
	{
		/// Reflect
		float fovy = 45.f;
		/// Reflect
		float aspect = 1.f;
		/// Reflect
		float zNear = 1.f;
		/// Reflect
		float zFar = 1000.f;

		mat4 view_mat;
		mat4 proj_mat;

		virtual void update() = 0;

		struct Main
		{
			virtual cCameraPtr operator()() = 0;
		};
		FLAME_UNIVERSE_EXPORTS static Main& main;

		struct Create
		{
			virtual cCameraPtr operator()(EntityPtr e) = 0;
		};
		/// Reflect static
		FLAME_UNIVERSE_EXPORTS static Create& create;
	};
}
