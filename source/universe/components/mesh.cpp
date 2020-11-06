#include <flame/graphics/device.h>
#include <flame/graphics/model.h>
#include <flame/graphics/canvas.h>
#include "../entity_private.h"
#include "node_private.h"
#include "mesh_private.h"

namespace flame
{
	cMeshPrivate::~cMeshPrivate()
	{
		destroy_deformer();
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
		cast_shadow = v;
	}

	void cMeshPrivate::set_animation_name(const std::string& name)
	{
		if (animation_name == name)
			return;
		animation_name = name;
		apply_animation();
		if (node)
			node->mark_transform_dirty();
	}

	void cMeshPrivate::destroy_deformer()
	{
		if (deformer)
		{
			deformer->release();
			deformer = nullptr;
			for (auto& b : bones)
				b.node->entity->remove_local_data_changed_listener(b.changed_listener);
			bones.clear();
		}
	}

	void cMeshPrivate::stop_animation()
	{
		if (animation_event)
		{
			animation_frame = -1;
			looper().remove_event(animation_event);
			animation_event = nullptr;
		}
	}

	void cMeshPrivate::apply_src()
	{
		model_id = -1;
		mesh_id = -1;
		model = nullptr;
		mesh = nullptr;
		if (canvas && !src.empty())
		{
			auto sp = SUS::split(src, '.');
			if (sp.size() == 2)
			{
				model_id = canvas->find_model_resource(sp[0].c_str());
				model = (graphics::Model*)canvas->get_model_resource(model_id);
				mesh_id = model->find_mesh(sp[1].c_str());
				mesh = model->get_mesh(mesh_id);
				auto bones_count = mesh->get_bones_count();
				if (bones_count > 0)
				{
					deformer = graphics::ArmatureDeformer::create(canvas->get_preferences(), mesh);
					bones.resize(bones_count);
					auto armature = entity->parent;
					if (armature)
					{
						for (auto i = 0; i < bones_count; i++)
						{
							auto& b = bones[i];
							auto name = std::string(mesh->get_bone(i)->get_name());
							auto e = armature->find_child(name);
							if (e)
							{
								auto n = e->get_component_t<cNodePrivate>();
								if (n)
								{
									b.name = name;
									b.node = n;
									b.changed_listener = e->add_local_data_changed_listener([](Capture& c, Component* t, uint64 h) {
										auto thiz = c.thiz<cMeshPrivate>();
										auto id = c.data<int>();
										auto& b = thiz->bones[id];
										if (t == b.node && h == S<ch("transform")>::v)
										{
											b.node->update_transform();
											thiz->deformer->set_pose(id, b.node->transform);
										}
									}, Capture().set_thiz(this).set_data(&i));
								}
							}
						}
					}
				}
			}
		}
	}

	void cMeshPrivate::apply_animation()
	{
		stop_animation();
		if (canvas && model && deformer && !animation_name.empty())
		{
			auto animation_id = model->find_animation(animation_name.c_str());
			if (animation_id != -1)
			{
				auto animation = model->get_animation(animation_id);
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
						auto& b = bones[bid];
						auto pkc = ch->get_position_keys_count();
						auto rkc = ch->get_rotation_keys_count();
						animation_max_frame = max(pkc, rkc);
						b.frames.resize(animation_max_frame);
						auto pk = ch->get_position_keys();
						auto rk = ch->get_rotation_keys();
						for (auto j = 0; j < animation_max_frame; j++)
						{
							auto& f = b.frames[j];
							f.p = j < pkc ? pk[j].v : Vec3f(0.f);
							f.q = j < rkc ? rk[j].v : Vec4f(0.f, 0.f, 0.f, 1.f);
						}
					}
				}
				animation_frame = 0;
				animation_event = looper().add_event([](Capture& c) {
					auto thiz = c.thiz<cMeshPrivate>();
					for (auto i = 0; i < thiz->bones.size(); i++)
					{
						auto& b = thiz->bones[i];
						auto& k = b.frames[thiz->animation_frame];
						b.node->set_pos(k.p);
						b.node->set_quat(k.q);
					}
					thiz->animation_frame++;
					if (thiz->animation_frame == thiz->animation_max_frame)
						thiz->animation_frame = 0;
					c._current = nullptr;
				}, Capture().set_thiz(this), 1.f / 24.f);
			}
			else
				animation_name.clear();
		}
	}

	void cMeshPrivate::on_gain_canvas()
	{
		apply_src();
		apply_animation();
	}

	void cMeshPrivate::draw(graphics::Canvas* canvas)
	{
		if (model_id != -1 && mesh_id != -1)
			canvas->draw_mesh(model_id, mesh_id, node->transform, Mat4f(node->global_axes), cast_shadow, deformer);
	}

	cMesh* cMesh::create()
	{
		return f_new<cMeshPrivate>();
	}
}
