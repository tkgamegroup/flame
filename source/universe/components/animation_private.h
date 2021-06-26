#pragma once

#include "../../graphics/model.h"
#include "animation.h"

namespace flame
{
	struct cAnimationPrivate : cAnimation
	{
		struct Bone
		{
			std::string name;
			cNodePrivate* node;
			mat4 offmat;
		};

		struct Action
		{
			uint total_frame;
			std::vector<std::pair<uint, std::vector<graphics::BoneKey>>> tracks;

			void apply(Bone* bones, uint frame);
		};

		std::filesystem::path model_name;
		std::wstring src;

		cNodePrivate* node = nullptr;
		void* drawer = nullptr;
		sRendererPrivate* s_renderer = nullptr;
		int armature_id = -1;

		std::vector<Bone> bones;
		std::vector<mat4> bone_mats;
		std::vector<Action> actions;
		bool loop = false;
		int playing = -1;
		int frame = -1;
		void* event = nullptr;
		std::pair<int, int> peeding_pose = { -1, -1 };
		std::vector<std::unique_ptr<Closure<void(Capture&, int)>>> callbacks;

		~cAnimationPrivate();

		const wchar_t* get_model_name() const override { return model_name.c_str(); }
		void set_model_name(const std::filesystem::path& src);
		void set_model_name(const wchar_t* src) override { set_model_name(std::filesystem::path(src)); }

		const wchar_t* get_src() const override { return src.c_str(); }
		void set_src(const std::wstring& src);
		void set_src(const wchar_t* src) override { set_src(std::wstring(src)); }

		int get_playing() override { return playing; }
		void play(uint id) override;
		void stop() override;
		void stop_at(uint id, int frame) override;

		bool get_loop() const override { return loop; }
		void set_loop(bool l) override;

		void* add_callback(void (*callback)(Capture& c, int frame), const Capture& capture) override;
		void remove_callback(void* cb) override;

		void apply_src();
		void advance();

		void draw(sRenderer* s_renderer);

		void on_added() override;
		void on_removed() override;
		void on_entered_world() override;
		void on_left_world() override;
	};
}