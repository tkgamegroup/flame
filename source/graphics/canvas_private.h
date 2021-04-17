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
//
//		struct MeshWeight
//		{
//			ivec4 ids;
//			vec4 weights;
//		};
//
//		struct MeshInfo
//		{
//			MeshResourcePrivate* res;
//			mat4 transform;
//			bool cast_shadow;
//			ArmatureDeformerPrivate* deformer;
//			ShadeFlags flags;
//			void* userdata;
//		};
//
//		struct TerrainInfo
//		{
//			uvec2 blocks;
//			vec3 scale;
//			vec3 coord;
//			float tess_levels;
//			uint height_tex_id;
//			uint normal_tex_id;
//			uint material_id;
//			ShadeFlags flags;
//			void* userdata;
//		};
//
//		struct DirectionalLight
//		{
//			vec3 dir;
//			vec3 side;
//			vec3 up;
//			vec3 color;
//			bool cast_shadow;
//			float shadow_distance;
//		};
//
//		struct PointLight
//		{
//			vec3 coord;
//			vec3 color;
//			bool cast_shadow;
//			float shadow_distance;
//		};
// 
//		struct CmdDrawTerrain : Cmd
//		{
//			std::vector<uint> entries;
//		};
//
//		struct CmdDrawLines : Cmd
//		{
//			uint count = 0;
//		};
//
//		struct CmdDrawTriangles : Cmd
//		{
//			uint count = 0;
//		};
//
//		struct CmdSetViewport : Cmd
//		{
//			Rect viewport;
//
//			CmdSetViewport(const Rect& _viewport) : Cmd(SetViewport) { viewport = _viewport; }
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
//		struct CmdBloom : Cmd
//		{
//			CmdBloom() : Cmd(Bloom) {}
//		};
//
//		struct CanvasPrivate : Canvas
//		{
//			float shadow_distance = 100.f;
//			uint csm_levels = 3;
//			float csm_factor = 0.3f;
//
//			std::unique_ptr<ImagePrivate> default_sky_box_image;
//			std::unique_ptr<ImagePrivate> default_sky_irr_image;
//			std::unique_ptr<ImagePrivate> default_sky_rad_image;
//			std::unique_ptr<DescriptorSetPrivate> sky_descriptorset;
//
//			//ShaderBuffer material_info_buffer;
//			std::unique_ptr<DescriptorSetPrivate> material_descriptorset;
//
//			std::unique_ptr<ImagePrivate> shadow_depth_image;
//			std::unique_ptr<ImagePrivate> shadow_depth_back_image;
//			std::unique_ptr<FramebufferPrivate> shadow_depth_back_framebuffer;
//			std::unique_ptr<DescriptorSetPrivate> shadow_depth_back_descriptorset;
//
//			//ShaderBuffer light_sets_buffer;
//			//ShaderBuffer light_infos_buffer;
//			//ShaderBuffer shadow_matrices_buffer;
//			std::vector<std::unique_ptr<ImagePrivate>> directional_shadow_maps;
//			std::vector<std::unique_ptr<FramebufferPrivate>> directional_light_depth_framebuffers;
//			std::vector<std::unique_ptr<FramebufferPrivate>> directional_shadow_map_framebuffers;
//			std::vector<std::unique_ptr<DescriptorSetPrivate>> directional_shadow_map_descriptorsets;
//			std::vector<std::unique_ptr<ImagePrivate>> point_shadow_maps;
//			std::vector<std::unique_ptr<FramebufferPrivate>> point_light_depth_framebuffers;
//			std::vector<std::unique_ptr<FramebufferPrivate>> point_shadow_map_framebuffers;
//			std::vector<std::unique_ptr<DescriptorSetPrivate>> point_shadow_map_descriptorsets;
//
//			std::unique_ptr<DescriptorSetPrivate> light_descriptorset;
//
//			//ShaderBuffer terrain_info_buffer;
//			std::unique_ptr<DescriptorSetPrivate> terrain_descriptorset;
//
//			std::vector<ImageViewPrivate*> output_imageviews;
//			std::vector<std::unique_ptr<FramebufferPrivate>> output_framebuffers;
//			std::vector<std::unique_ptr<DescriptorSetPrivate>> output_descriptorsets;
//
//			std::unique_ptr<ImagePrivate> hdr_image;
//			std::unique_ptr<FramebufferPrivate> hdr_framebuffer;
//			std::unique_ptr<DescriptorSetPrivate> hdr_descriptorset;
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
//			std::vector<std::vector<vec2>> paths;
//
//			ShaderGeometryBuffer<Line> line_buffer;
//			ShaderGeometryBuffer<Triangle> triangle_buffer;
//
//			std::vector<MeshInfo> meshes;
//			std::vector<TerrainInfo> terrains;
//			std::vector<DirectionalLight> directional_lights;
//			std::vector<PointLight> point_lights;
//
//			Rect curr_viewport;
//		};
//	}
//}
