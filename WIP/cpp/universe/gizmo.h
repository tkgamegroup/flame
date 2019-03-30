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
