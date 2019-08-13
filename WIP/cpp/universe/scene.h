namespace flame
{
	namespace _3d
	{
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

