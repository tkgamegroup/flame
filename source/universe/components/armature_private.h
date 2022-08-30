#pragma once

#include "../../graphics/model.h"
#include "../../graphics/animation.h"
#include "armature.h"
#include "node_private.h"

namespace flame
{
	struct cArmaturePrivate : cArmature
	{
		struct Pose
		{
			vec3 p = vec3(0);
			quat q = quat(1, 0, 0, 0);
		};

		struct Bone
		{
			std::string name;
			cNodePtr node = nullptr;
			mat4 offmat;
			Pose pose;

			inline mat4 calc_mat();
		};

		struct Track
		{
			uint bone_idx;
			std::vector<std::pair<float, vec3>> positions;
			std::vector<std::pair<float, quat>> rotations;
		};

		struct BoundAnimation
		{
			std::filesystem::path path;
			graphics::AnimationPtr animation = nullptr;
			float duration;
			std::vector<Track> tracks;
			std::unordered_map<uint, float> transitions;
		};

		bool dirty = true;
		std::vector<Bone> bones;
		std::unordered_map<uint, BoundAnimation> animations;
		float transition_time = -1.f;
		float transition_duration = 0.f;

		~cArmaturePrivate();
		void on_init() override;

		void set_armature_name(const std::filesystem::path& name) override;
		void set_animation_names(const std::vector<std::pair<std::filesystem::path, std::string>>& names) override;

		void play(uint name) override;
		void stop() override;

		void on_active() override;
		void on_inactive() override;
		void update() override;
	};
}
