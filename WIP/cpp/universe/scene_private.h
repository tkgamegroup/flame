namespace flame
{
	namespace graphics
	{
		struct Buffer;
		struct Image;
		struct Imageview;
		struct Renderpass;
		struct Framebuffer;
		struct ClearValues;
		struct Shader;
		struct Pipeline;
		struct Sampler;
		struct Descriptorset;
	}

	namespace _3d
	{
		struct ModelPrivate;

		struct ShareData
		{
			graphics::Device *d;
			graphics::Renderpass *rp_scene;
			graphics::Renderpass *rp_one_att;
			graphics::Pipeline *pl_sky_blue;
			graphics::Pipeline *pl_sky_brightsun;
			graphics::Pipeline *pl_lightmap;
			graphics::Pipeline *pl_pbribl;
			graphics::Pipeline *pl_cameralight;
			graphics::Pipeline *pl_frame;

			float *bk_fix_center;
			float *bk_fix_left;
			float *bk_fix_right;
			float *bk_fix_top;
			float *bk_fix_bottom;

			void create(graphics::Device *_d);
			void destroy();
		};

		extern ShareData share_data;

		struct BakeUnit
		{
			Vec3 pos;
			Vec3 normal;
			Vec3 up;
			Ivec2 pixel_coord;
		};

		struct RegisteredModel
		{
			ModelPrivate *m;
			int vc;
			int vc_frame;

			graphics::Buffer *pos_buf;
			graphics::Buffer *uv_buf;
			graphics::Buffer *normal_buf;
			graphics::Buffer *frame_buf;

			std::vector<BakeUnit> bk_units;
		};

		struct ScenePrivate : Scene
		{
			Ivec2 res;
			ShowMode show_mode;
			bool show_frame;

			Camera *c;

			std::vector<RegisteredModel> ms;

			float bk_ratio;
			Ivec2 bk_imgsize;

			Ivec2 bk_pen_pos;
			int bk_pen_lineheight;

			graphics::Buffer *matrix_buf;
			graphics::Image *col_image;
			graphics::Image *dep_image;
			graphics::Image *bk_img;
			graphics::Framebuffer *framebuffer;
			graphics::ClearValues *clear_values;
			graphics::Descriptorset *ds_skybrightsun;
			graphics::Descriptorset *ds_lightmap;
			graphics::Descriptorset *ds_pbribl;
			graphics::Descriptorset *ds_cameralight;
			graphics::Descriptorset *ds_frame;
			graphics::Commandbuffer *cb;

			float elp_time_;

			ScenePrivate(const Ivec2 &resolution);
			~ScenePrivate();

			void register_model(ModelPrivate *m);

			void set_bake_props(float ratio, const Ivec2 &imgsize);

			void begin(float elp_time);
			void end();

			void draw_scene(graphics::Commandbuffer *cb, const Vec2 &camera_props);

			void record_cb();

			void bake(int pass);
		};
	}
}
