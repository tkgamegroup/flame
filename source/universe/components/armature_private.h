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

		struct Animation
		{
			float duration;
			std::vector<Track> tracks;
		};

		graphics::ModelPtr model = nullptr;

		std::vector<Bone> bones;
		std::vector<Animation> animations;
		bool bones_dirty = false;
		bool animations_dirty = false;
		float transition_time = -1.f;

		int frame = -1;

		~cArmaturePrivate();
		void on_init() override;

		void set_model_name(const std::filesystem::path& src) override;
		void set_animation_names(const std::wstring& paths) override;

		void play(uint id) override;
		void stop() override;

		void draw(sRendererPtr renderer);

		void on_active() override;
		void on_inactive() override;
	};
}
