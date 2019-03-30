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
