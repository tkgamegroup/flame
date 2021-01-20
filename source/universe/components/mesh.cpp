#include <flame/graphics/device.h>
#include <flame/graphics/model.h>
#include <flame/graphics/canvas.h>
#include "../entity_private.h"
#include "../world_private.h"
#include "node_private.h"
#include "mesh_private.h"
#include "../systems/renderer_private.h"

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
			//for (auto& b : bones)
			//	b.node->entity->remove_local_data_changed_listener(b.changed_listener);
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
			auto sp = SUS::split(src, '#');
			if (sp.size() == 2)
			{
				auto isfile = false;
				auto fn = std::filesystem::path(sp[0]);
				if (!fn.extension().empty())
				{
					isfile = true;
					if (!fn.is_absolute())
						fn = entity->path.parent_path() / fn;
				}
				model_id = canvas->find_model_resource(fn.string().c_str());
				if (model_id == -1 && isfile)
				{
					auto model = graphics::Model::create(fn.c_str());
					fassert(model);
					model_id = canvas->set_model_resource(-1, model, fn.filename().string().c_str());
				}

				if (model_id != -1)
				{
					model = (graphics::Model*)canvas->get_model_resource(model_id);
					mesh_id = model->find_mesh(sp[1].c_str());
					if (mesh_id != -1)
					{
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
											b.changed_listener = e->add_data_listener(b.node, [](Capture& c, uint64 h) {
												auto thiz = c.thiz<cMeshPrivate>();
												auto id = c.data<int>();
												auto& b = thiz->bones[id];
												if (h == S<"transform"_h>)
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
		}
	}

	void cMeshPrivate::draw(graphics::Canvas* canvas)
	{
		if (model_id != -1 && mesh_id != -1)
			canvas->draw_mesh(model_id, mesh_id, node->transform, cast_shadow, deformer);
	}

	void cMeshPrivate::apply_animation()
	{
		stop_animation();
		if (model && deformer && !animation_name.empty())
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
							f.p = j < pkc ? pk[j].v : vec3(0.f);
							f.q = j < rkc ? rk[j].v : quat(1.f, 0.f, 0.f, 0.f);
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

	void cMeshPrivate::on_added()
	{
		node = entity->get_component_t<cNodePrivate>();
		fassert(node);

		drawer = node->add_drawer([](Capture& c, graphics::Canvas* canvas) {
			auto thiz = c.thiz<cMeshPrivate>();
			thiz->draw(canvas);
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
		canvas = entity->world->get_system_t<sRendererPrivate>()->canvas;
		fassert(canvas);

		apply_src();
		apply_animation();
	}

	void cMeshPrivate::on_left_world()
	{
		canvas = nullptr;
		model_id = -1;
		mesh_id = -1;
		model = nullptr;
		mesh = nullptr;
	}

	cMesh* cMesh::create()
	{
		return f_new<cMeshPrivate>();
	}
}
