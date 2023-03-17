#pragma once

#include "node.h"
#include "../entity_private.h"

namespace flame
{
	struct OctNode;

	struct cNodePrivate : cNode
	{
		bool transform_dirty = true;

		uint instance_frame = 0;

		void set_pos(const vec3& pos) override;
		vec3 get_eul() override;
		void set_eul(const vec3& eul) override;
		void set_qut(const quat& qut) override;
		void set_scl(const vec3& scl) override;

		vec3 global_scl() override;

		void mark_transform_dirty() override;
		void mark_drawing_dirty() override;

		void look_at(const vec3& t) override;

		bool update_transform() override;

		void draw(DrawData& draw_data);

		void on_active() override;
		void on_inactive() override;
	};
}
