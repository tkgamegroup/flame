//namespace flame
//{
//	namespace graphics
//	{
//		struct ArmatureDeformerPrivate : ArmatureDeformer
//		{
//			MeshPrivate* mesh;
//			//ShaderBuffer poses_buffer;
//			std::unique_ptr<DescriptorSetPrivate> descriptorset;
//
//			ArmatureDeformerPrivate(RenderPreferencesPrivate* preferences, MeshPrivate* mesh);
//			void release() override { delete this; }
//			void set_pose(uint id, const mat4& pose) override;
//		};
//
//		struct MeshWeight
//		{
//			ivec4 ids;
//			vec4 weights;
//		};
//
//		struct CmdBlur : Cmd
//		{
//			Rect range;
//			uint radius;
//
//			CmdBlur(const Rect& _range, uint _radius) : Cmd(Blur) { range = _range; radius = _radius; }
//		};
//
//		struct CanvasPrivate : Canvas
//		{
//			std::unique_ptr<ImagePrivate> default_sky_box_image;
//			std::unique_ptr<ImagePrivate> default_sky_irr_image;
//			std::unique_ptr<ImagePrivate> default_sky_rad_image;
//			std::unique_ptr<DescriptorSetPrivate> sky_descriptorset;
//
//			//ShaderBuffer shadow_matrices_buffer;
//			std::vector<std::unique_ptr<ImagePrivate>> directional_shadow_maps;
//			std::vector<std::unique_ptr<FramebufferPrivate>> directional_light_depth_framebuffers;
//			std::vector<std::unique_ptr<ImagePrivate>> point_shadow_maps;
//			std::vector<std::unique_ptr<FramebufferPrivate>> point_light_depth_framebuffers;
//
//			std::vector<ImageViewPrivate*> output_imageviews;
//			std::vector<std::unique_ptr<FramebufferPrivate>> output_framebuffers;
//			std::vector<std::unique_ptr<DescriptorSetPrivate>> output_descriptorsets;
//
//			std::unique_ptr<ImagePrivate> depth_image;
//
//			std::vector<std::unique_ptr<FramebufferPrivate>> mesh_framebuffers;
//
//			std::unique_ptr<ImagePrivate> back_image;
//			std::vector<std::unique_ptr<FramebufferPrivate>> back_framebuffers;
//			std::vector<std::unique_ptr<DescriptorSetPrivate>> back_nearest_descriptorsets;
//			std::vector<std::unique_ptr<DescriptorSetPrivate>> back_linear_descriptorsets;
//
//			std::unique_ptr<ImagePrivate> pickup_image;
//			std::unique_ptr<FramebufferPrivate> pickup_framebuffer;
//
//			ShaderGeometryBuffer<Line> line_buffer;
//			ShaderGeometryBuffer<Triangle> triangle_buffer;
//		};
//	}
//}
