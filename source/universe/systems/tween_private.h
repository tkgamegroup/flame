#pragma once

#include "tween.h"

namespace flame
{
	struct TweenPrivate : Tween
	{
		enum ActionType
		{
			MoveTo,
			RotateTo,
			ScaleTo
		};

		struct Action
		{
			ActionType type;
			float duration;
			lVariant v0, v1;
		};

		struct Track
		{
			std::vector<Action> actions;
		};

		struct Animation
		{
			uint id;
			EntityPtr target;
			std::vector<Track> tracks;
		};

		TweenPrivate();

		uint begin(EntityPtr e) override;
		void end(uint id) override;
		void newline(uint id) override;
		void wait(uint id, float time) override;
		void move_to(uint id, const vec3& pos, float duration) override;
		void rotate_to(uint id, const quat& rot, float duration) override;
		void rotate_to(uint id, const vec3& eul, float duration) override;
		void scale_to(uint id, float scale, float duration) override;
		void scale_to(uint id, const vec3& scale, float duration) override;
		void play_animation(uint id, uint name) override;
		void kill(uint id) override;
	};
}
