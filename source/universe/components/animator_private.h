#pragma once

#include "../../graphics/model.h"
#include "../../graphics/animation.h"
#include "animator.h"
#include "node_private.h"

namespace flame
{
	struct cAnimatorPrivate : cAnimator
	{
		struct Pose
		{
			vec3 p = vec3(0);
			quat q = quat(1, 0, 0, 0);
			vec3 s = vec3(1);
			mat4 m = mat4(1.f);
		};

		struct Cluster
		{
			std::string name;
			cNodePtr node = nullptr;
			mat4 offmat;
			Pose pose;
		};

		struct Track
		{
			uint cluster_idx;
			graphics::ChannelPtr channel = nullptr;
		};

		struct BoundAnimation
		{
			std::filesystem::path path;
			graphics::AnimationPtr animation = nullptr;
			float duration;
			std::vector<Track> tracks;
			std::vector<graphics::EventKey>::iterator events_beg;
			std::vector<graphics::EventKey>::iterator events_end;
			std::vector<graphics::EventKey>::iterator events_it;
			std::unordered_map<uint, float> transitions;
		};

		bool dirty = true;
		std::vector<Cluster> clusters;
		std::unordered_map<cNodePtr, Cluster*> node_to_cluster;
		std::unordered_map<uint, BoundAnimation> animations;
		float transition_time = -1.f;
		float transition_duration = 0.f;

		~cAnimatorPrivate();

		void new_cluster(uint cluster_idx, const std::string& name, EntityPtr e, const mat4& offmat = mat4(1.f));
		void attach();
		void detach();
		void update_instance();

		void on_active() override;
		void on_inactive() override;
		void update() override;

		void set_armature_name(const std::filesystem::path& name) override;
		void set_animation_names(const std::vector<std::pair<std::filesystem::path, std::string>>& names) override;

		void reset() override;
		void play(uint name) override;
		void stop() override;
	};
}
