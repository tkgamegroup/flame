namespace flame
{
	namespace ui
	{
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
		};
	}
}
