#include "../../graphics/device.h"
#include "../../graphics/model.h"
#include "../entity_private.h"
#include "../world_private.h"
#include "node_private.h"
#include "mesh_private.h"
#include "../systems/renderer_private.h"

namespace flame
{
	cMeshPrivate::~cMeshPrivate()
	{
		stop_animation();
	}

	void cMeshPrivate::set_src(const std::string& _src)
	{
		if (src == _src)
			return;
		src = _src;
		apply_src();
		if (node)
			node->mark_transform_dirty();
	}

	void cMeshPrivate::set_cast_shadow(bool v)
	{
		if (cast_shadow != v)
		{
			cast_shadow = v;
			if (entity)
				entity->component_data_changed(this, S<"cast_shadow"_h>);
		}
	}

	void cMeshPrivate::apply_src()
	{
		mesh_id = -1;
		model = nullptr;
		mesh = nullptr;
		if (s_renderer && !src.empty())
		{
			auto sp = SUS::split(src, '#');
			if (sp.size() == 2)
			{
				auto fn = std::filesystem::path(sp[0]);
				if (fn.extension().empty())
					model = graphics::Model::get_standard(sp[0].c_str());
				else
				{
					if (!fn.is_absolute())
					{
						auto& srcs = entity->srcs;
						fn = srcs[srcs.size() - src_id - 1].parent_path() / fn;
					}
					fn.make_preferred();
					model = graphics::Model::get(fn.c_str());
					fassert(model);
				}

				if (!model)
					return;
				mesh = model->get_mesh(std::stoi(sp[1]));

				mesh_id = s_renderer->find_mesh_res(mesh);
				if (mesh_id == -1)
				{
					mesh_id = s_renderer->set_mesh_res(-1, mesh);
					if (mesh_id == -1)
					{
						mesh = nullptr;
						return;
					}
				}

				auto bones_count = mesh->get_bones_count();
				if (bones_count == 0)
					return;

				bones.resize(bones_count);
				bone_mats.resize(bones_count);
				auto armature = entity->parent;
				fassert(armature);
				for (auto i = 0; i < bones_count; i++)
				{
					auto src = mesh->get_bone(i);
					auto& dst = bones[i];
					auto name = std::string(src->get_name());
					auto e = armature->find_child(name);
					fassert(e);
					dst.name = name;
					dst.node = e->get_component_i<cNodePrivate>(0);
					fassert(dst.node);
					dst.offmat = src->get_offset_matrix();
				}
			}
		}
	}

	void cMeshPrivate::set_animation(const std::filesystem::path& name, bool loop)
	{
		if (ani_name == name)
		{
			if (loop_ani != loop)
				loop_ani = loop;
			return;
		}
		ani_name = name;
		loop_ani = loop;
		apply_animation();
		if (node)
			node->mark_transform_dirty();
	}

	void cMeshPrivate::apply_animation()
	{
		if (bones.empty() || ani_name.empty())
			return;

		auto fn = std::filesystem::path(ani_name);
		auto& srcs = entity->srcs;
		fn = srcs[srcs.size() - src_id - 1].parent_path() / fn;

		stop_animation();

		auto animation = graphics::Animation::get(fn.c_str());
		if (animation)
		{
			auto chs = animation->get_channels_count();
			for (auto i = 0; i < chs; i++)
			{
				auto ch = animation->get_channel(i);
				auto find_bone = [&](const std::string& name) {
					for (auto i = 0; i < bones.size(); i++)
					{
						if (bones[i].name == name)
							return i;
					}
					return -1;
				};
				auto bid = find_bone(ch->get_node_name());
				if (bid != -1)
				{
					auto pkc = ch->get_position_keys_count();
					auto rkc = ch->get_rotation_keys_count();
					ani_frame_max = max(pkc, rkc);

					auto& t = ani_tracks.emplace_back();
					t.first = bid;
					t.second.resize(ani_frame_max);

					auto pk = ch->get_position_keys();
					auto rk = ch->get_rotation_keys();
					for (auto j = 0; j < ani_frame_max; j++)
					{
						auto& f = t.second[j];
						f.p = j < pkc ? pk[j].v : vec3(0.f);
						f.q = j < rkc ? rk[j].v : quat(1.f, 0.f, 0.f, 0.f);
					}
				}
			}
			ani_frame = 0;
			ani_event = looper().add_event([](Capture& c) {
				auto thiz = c.thiz<cMeshPrivate>();
				thiz->advance_frame();
				if (thiz->ani_frame != -1)
					c._current = nullptr;
			}, Capture().set_thiz(this), 1.f / 24.f);
		}
	}

	void cMeshPrivate::stop_animation()
	{
		ani_name = L"";
		ani_frame = -1;
		ani_frame_max = 0;
		ani_tracks.clear();
		if (ani_event)
		{
			looper().remove_event(ani_event);
			ani_event = nullptr;
		}
	}

	void cMeshPrivate::advance_frame()
	{
		for (auto& t : ani_tracks)
		{
			auto& b = bones[t.first];
			auto& k = t.second[ani_frame];
			b.node->set_pos(k.p);
			b.node->set_quat(k.q);
		}
		ani_frame++;
		if (ani_frame == ani_frame_max)
		{
			if (loop_ani)
				ani_frame = 0;
			else
			{
				ani_event = nullptr;
				stop_animation();
			}
		}
	}

	void cMeshPrivate::draw(sRenderer* s_renderer)
	{
		if (mesh_id != -1)
		{
			for (auto i = 0; i < bones.size(); i++)
			{
				auto& b = bones[i];
				b.node->update_transform();
				bone_mats[i] = b.node->transform * b.offmat;
			}
			s_renderer->draw_mesh(node, mesh_id, cast_shadow, bones.size(), bone_mats.data());
		}
		//auto flags = s_renderer->wireframe ? graphics::ShadeWireframe : graphics::ShadeMaterial;
		//if (entity->state & StateSelected)
		//	flags = flags | graphics::ShadeOutline;
	}

	bool cMeshPrivate::measure(AABB* b)
	{
		if (!mesh)
			return false;
		*b = AABB(mesh->get_lower_bound(), mesh->get_upper_bound());
		return true;
	}

	void cMeshPrivate::on_added()
	{
		node = entity->get_component_i<cNodePrivate>(0);
		fassert(node);

		drawer = node->add_drawer([](Capture& c, sRendererPtr s_renderer) {
			auto thiz = c.thiz<cMeshPrivate>();
			thiz->draw(s_renderer);
		}, Capture().set_thiz(this));
		measurer = node->add_measure([](Capture& c, AABB* b) {
			auto thiz = c.thiz<cMeshPrivate>();
			return thiz->measure(b);
		}, Capture().set_thiz(this));
		node->mark_drawing_dirty();
	}

	void cMeshPrivate::on_removed()
	{
		node->remove_drawer(drawer);
		node = nullptr;
	}

	void cMeshPrivate::on_entered_world()
	{
		s_renderer = entity->world->get_system_t<sRendererPrivate>();
		fassert(s_renderer);

		apply_src();
		apply_animation();
	}

	void cMeshPrivate::on_left_world()
	{
		s_renderer = nullptr;
		mesh_id = -1;
		model = nullptr;
		mesh = nullptr;
	}

	cMesh* cMesh::create(void* parms)
	{
		return f_new<cMeshPrivate>();
	}
}
