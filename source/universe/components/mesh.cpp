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
	void cMeshPrivate::AnimationLayer::stop()
	{
		frame = -1;
		max_frame = 0;
		poses.clear();
		if (event)
		{
			looper().remove_event(event);
			event = nullptr;
		}
	}

	cMeshPrivate::~cMeshPrivate()
	{
		destroy_deformer();
		for (auto i = 0; i < size(animation_layers); i++)
			stop_animation(i);
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
						fn = (path.empty() ? entity->path.parent_path() : path) / fn;
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

	void cMeshPrivate::set_animation(const std::string& name, bool loop, uint layer)
	{
		auto& al = animation_layers[layer];
		if (al.name == name && al.loop == loop)
			return;
		al.name = name;
		al.loop = loop;
		apply_animation(layer);
		if (node)
			node->mark_transform_dirty();
	}

	void cMeshPrivate::apply_animation(uint layer)
	{
		stop_animation(layer);
		auto& al = animation_layers[layer];
		if (model && deformer && !al.name.empty())
		{
			auto animation_id = model->find_animation(al.name.c_str());
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
						auto pkc = ch->get_position_keys_count();
						auto rkc = ch->get_rotation_keys_count();
						al.max_frame = max(pkc, rkc);

						auto& t = al.add_track();
						t.first = bid;
						t.second.resize(al.max_frame);

						auto pk = ch->get_position_keys();
						auto rk = ch->get_rotation_keys();
						for (auto j = 0; j < al.max_frame; j++)
						{
							auto& f = t.second[j];
							f.p = j < pkc ? pk[j].v : vec3(0.f);
							f.q = j < rkc ? rk[j].v : quat(1.f, 0.f, 0.f, 0.f);
						}
					}
				}
				al.frame = 0;
				al.event = looper().add_event([](Capture& c) {
					auto thiz = c.thiz<cMeshPrivate>();
					auto& al = thiz->animation_layers[c.data<uint>()];
					for (auto i = 0; i < al.poses.size(); i++)
					{
						auto& t = al.poses[i];
						auto& b = thiz->bones[t.first];
						auto& k = t.second[al.frame];
						b.node->set_pos(k.p);
						b.node->set_quat(k.q);
					}
					al.frame++;
					c._current = nullptr;
					if (al.frame == al.max_frame)
					{
						if (al.loop)
							al.frame = 0;
						else
						{
							al.name = "";
							al.event = nullptr;
							al.stop();
							c._current = INVALID_POINTER;
						}
					}
				}, Capture().set_thiz(this).set_data(&layer), 1.f / 24.f);
			}
			else
				al.name.clear();
		}
	}

	void cMeshPrivate::stop_animation(uint layer)
	{
		animation_layers[layer].stop();
	}

	void cMeshPrivate::draw(graphics::Canvas* canvas)
	{
		if (model_id != -1 && mesh_id != -1)
			canvas->draw_mesh(model_id, mesh_id, node->transform, cast_shadow, deformer);
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
		for (auto i = 0; i < size(animation_layers); i++)
			apply_animation(i);
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
