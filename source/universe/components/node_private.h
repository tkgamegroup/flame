#pragma once

#include "node.h"
#include "../entity_private.h"

namespace flame
{
	struct OctNode;

	struct cNodePrivate : cNode
	{
		bool eul_dirty = false;
		bool qut_dirty = false;
		bool rot_dirty = false;

		cNodePrivate* pnode = nullptr;
		bool transform_dirty = true;
		uint transform_updated_times = 0;
		mat4 transform;

		//std::pair<OctNode*, OctNode*> octnode = { nullptr, nullptr };

		//sScenePrivate* s_scene = nullptr;
		//bool pending_update_bounds = false;
		sNodeRendererPrivate* s_renderer = nullptr;

		cNodePrivate();

		void set_pos(const vec3& pos) override;
		vec3 get_eul() override;
		void set_eul(const vec3& eul) override;
		quat get_qut() override;
		void set_qut(const quat& qut) override;
		void set_scl(const vec3 & scl) override;

		void look_at(const vec3& t) override;

		void update_eul();
		void update_qut();
		void update_rot();
		void update_transform();

		void mark_transform_dirty();
		void mark_bounds_dirty(bool child_caused);
		void mark_drawing_dirty();
		void remove_from_bounds_list();

		void draw(uint frame, bool shadow_pass);

		void on_entered_world() override;
		void on_left_world() override;
	};
}
