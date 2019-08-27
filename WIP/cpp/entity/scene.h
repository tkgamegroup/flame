namespace flame
{
	struct CollisionGroup;

	class Scene : public Node
	{
	private:
		std::string filename;

		std::unique_ptr<Sky> sky;
		bool enable_sun_light;

		float hdr_exposure;
		float hdr_white;
	
		glm::vec3 bg_color;
		glm::vec3 ambient_color ;
		glm::vec3 fog_color;

		float ssao_radius;
		float ssao_bias;
		float ssao_intensity;

		float fog_thickness;

		//std::vector<CollisionGroup*> pCollisionGroups;

		//physx::PxControllerManager *pxControllerManager = nullptr;
	};
}
