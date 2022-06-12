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

		bool transform_dirty = true;

		~cNodePrivate();
		void set_pos(const vec3& pos) override;
		vec3 get_eul() override;
		void set_eul(const vec3& eul) override;
		quat get_qut() override;
		void set_qut(const quat& qut) override;
		void set_scl(const vec3 & scl) override;

		void look_at(const vec3& t) override;

		void update_eul() override;
		void update_qut() override;
		void update_rot() override;
		bool update_transform() override;

		void mark_transform_dirty() override;
		void mark_drawing_dirty() override;

		void draw(sRendererPtr renderer, uint pass);

		void on_active() override;
		void on_inactive() override;
	};
}
