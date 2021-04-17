//namespace flame
//{
//	namespace graphics
//	{
//		enum ShadeFlags
//		{
//			ShadeMaterial = 1 << 0,
//			ShadeWireframe = 1 << 1,
//			ShadeOutline = 1 << 2
//		};
//
//		inline ShadeFlags operator| (ShadeFlags a, ShadeFlags b) { return (ShadeFlags)((int)a | (int)b); }
//
//		struct ArmatureDeformer
//		{
//			virtual void release() = 0;
//
//			virtual void set_pose(uint id, const mat4& pose) = 0;
//
//			FLAME_GRAPHICS_EXPORTS static ArmatureDeformer* create(RenderPreferences* preferences, Mesh* mesh);
//		};
//	}
//}
