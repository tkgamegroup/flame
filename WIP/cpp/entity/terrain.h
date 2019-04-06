namespace flame
{
	struct TerrainVertex
	{
		glm::vec4 normal_height;
		glm::vec3 tangent;
	};

	class TerrainComponent : public Component
	{
	private:
		int block_cx;
		int block_cy;
		float block_size;
		float height;
		float displacement_height;
		float tessellation_factor;
		float tiling_scale;

		std::shared_ptr<Texture> height_image;
		std::shared_ptr<Texture> blend_image;

		int material_count;
		std::shared_ptr<Material> materials[4];

		bool enable_physics;

		physx::PxRigidActor *actor;

		int terrain_index;
	};
}
