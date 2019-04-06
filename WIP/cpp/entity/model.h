#pragma once

#include <memory>
#include <vector>

#include <flame/math.h>
#include <flame/engine/config.h>
#include <flame/engine/graphics/graphics.h>

namespace flame
{
	enum ModelStateAnimationKind
	{
		ModelStateAnimationStand,
		ModelStateAnimationForward,
		ModelStateAnimationBackward,
		ModelStateAnimationLeftward,
		ModelStateAnimationRightward,
		ModelStateAnimationJump,

		ModelStateAnimationCount
	};

	struct BoneIK
	{
		int targetID = -1;
		int effectorID = -1;
		unsigned short iterations = 0;
		float weight = 0.f;
		std::vector<int> chain;
	};

	struct GeometryAux
	{
		struct Triangle
		{
			int indices[3];
			std::pair<int, int> adjacency[3]; // tri idx and vtx idx
		};

		std::vector<glm::vec3> unique_vertex;
		std::unique_ptr<Triangle[]> triangles;
	};

	struct Model
	{
		struct UV
		{
			std::string name;
			std::vector<glm::vec2> unique;
			std::vector<int> indices;
			std::vector<std::pair<int, int>> series;

			void add(const glm::vec2 &v);
		};

		std::string filename;
		std::string filepath;

		int vertex_base = 0;
		int indice_base = 0;

		std::vector<ModelVertex> vertexes;
		std::vector<ModelVertexSkeleton> vertexes_skeleton;
		std::vector<int> indices;

		std::vector<std::unique_ptr<Geometry>> geometries;
		std::vector<std::unique_ptr<UV>> uvs;
		UV *geometry_uv = nullptr;
		UV *bake_uv = nullptr;
		std::unique_ptr<GeometryAux> geometry_aux;

		int bake_grid_pixel_size = 4;
		int bake_image_cx = 256;
		int bake_image_cy = 256;

		std::vector<std::unique_ptr<Bone>> bones;
		std::vector<std::unique_ptr<BoneIK>> iks;

		std::shared_ptr<AnimationBinding> stateAnimations[ModelStateAnimationCount];

		std::string stand_animation_filename;
		std::string forward_animation_filename;
		std::string leftward_animation_filename;
		std::string rightward_animation_filename;
		std::string backward_animation_filename;
		std::string jump_animation_filename;

#if FLAME_ENABLE_PHYSICS != 0
		std::vector<std::unique_ptr<Rigidbody>> rigidbodies;
		std::vector<std::unique_ptr<Joint>> joints;
#endif

		glm::vec3 max_coord = glm::vec3(0.f);
		glm::vec3 min_coord = glm::vec3(0.f);

		glm::vec3 bounding_position = glm::vec3(0.f);
		float bounding_size = 1.f;

		glm::vec3 controller_position = glm::vec3(0.f);
		float controller_height = 1.f;
		float controller_radius = 0.5f;

		glm::vec3 eye_position = glm::vec3(0.f);

		void add_vertex_position_normal(const glm::vec3 &position, const glm::vec3 &normal);

		const char *get_uv_use_name(UV *uv) const;
		void create_geometry_aux();
		void create_uv();
		void remove_uv(UV *uv);
		void assign_uv_to_geometry(UV *uv);
		void assign_uv_to_bake(UV *uv);

		void set_state_animation(ModelStateAnimationKind kind, std::shared_ptr<AnimationBinding> b);

		Bone *new_bone();
		void remove_bone(Bone *b);

		BoneIK *new_bone_ik();
		void remove_bone_ik(BoneIK *b);

#if FLAME_ENABLE_PHYSICS != 0
		Rigidbody *new_rigidbody();
		void remove_rigidbody(Rigidbody *r);

		Joint *new_joint();
		void remove_joint(Joint *j);
#endif
	};
}
