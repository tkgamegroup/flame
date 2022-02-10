#include "../../graphics/model.h"
#include "../entity_private.h"
#include "../world_private.h"
#include "node_private.h"
#include "armature_private.h"
#include "../systems/node_renderer_private.h"

namespace flame
{
	mat4 cArmaturePrivate::Bone::calc_mat()
	{
		return node->transform * offmat;
	}

	void cArmaturePrivate::Animation::apply(Bone* bones, uint playing_frame)
	{
		for (auto& t : tracks)
		{
			auto& b = bones[t.first];
			auto& k = t.second[playing_frame];
			b.node->set_pos(k.p);
			b.node->set_qut(k.q);
		}
	}

	cArmaturePrivate::~cArmaturePrivate()
	{
		node->drawers.remove(drawer_lis);
		node->measurers.remove(measurer_lis);
	}

	void cArmaturePrivate::on_init()
	{
		drawer_lis = node->drawers.add([this](sNodeRendererPtr renderer, bool shadow_pass) {
			draw(renderer);
			});

		measurer_lis = node->measurers.add([this](AABB* ret) {
			*ret = AABB(vec3(0.f), 10000.f);
			return true;
			});

		node->mark_transform_dirty();
	}

	void cArmaturePrivate::set_model_name(const std::filesystem::path& path)
	{
		if (model_name == path)
			return;
		bones.clear();
		model_name = path;
		apply_src();
		if (node)
			node->mark_transform_dirty();
		data_changed("model_name"_h);
	}

	void cArmaturePrivate::set_animation_names(const std::wstring& paths)
	{
		if (animation_names == paths)
			return;
		animation_names = paths;
		apply_src();
		if (node)
			node->mark_transform_dirty();
		data_changed("animation_names"_h);
	}

	void cArmaturePrivate::play(uint id)
	{
		if (playing_id == id)
			return;
		stop();
		playing_id = id;
	}

	void cArmaturePrivate::stop()
	{
		playing_id = -1;
		playing_frame = 0;
		time = 0.f;
		playing_id = -1;
	}

	void cArmaturePrivate::apply_src()
	{
		auto model = graphics::Model::get(model_name);
		if (!model)
			return;

		bones.resize(model->bones.size());
		for (auto i = 0; i < bones.size(); i++)
		{
			auto& src = model->bones[i];
			auto& dst = bones[i];
			auto name = src.name;
			auto e = entity->find_child(name);
			if (e)
			{
				dst.name = name;
				dst.node = e->get_component_i<cNodeT>(0);
				if (dst.node)
					dst.offmat = src.offset_matrix;
			}
		}

		if (bones.empty() || animation_names.empty())
			return;

		auto sp = SUW::split(animation_names, ';');
		for (auto& s : sp)
		{
			auto animation = graphics::Animation::get(s);
			if (animation)
			{
				auto& a = animations.emplace_back();
				a.total_frame = 0;

				for (auto& sh : animation->channels)
				{
					auto find_bone = [&](std::string_view name) {
						for (auto i = 0; i < bones.size(); i++)
						{
							if (bones[i].name == name)
								return i;
						}
						return -1;
					};
					auto id = find_bone(sh.node_name);
					if (id != -1)
					{
						uint count = sh.keys.size();
						if (a.total_frame == 0)
							a.total_frame = max(a.total_frame, count);

						auto& t = a.tracks.emplace_back();
						t.first = id;
						t.second.resize(count);
						memcpy(t.second.data(), sh.keys.data(), sizeof(graphics::Channel::Key) * count);
					}
				}

				for (auto& t : a.tracks)
					t.second.resize(a.total_frame);
			}
		}
	}

	void cArmaturePrivate::draw(sNodeRendererPtr renderer)
	{
		if (object_id == -1)
			return;

		if (frame < (int)frames)
		{
			if (playing_id != -1)
			{
				auto& a = animations[playing_id];
				a.apply(bones.data(), playing_frame);

				time += playing_speed;
				while (time > 1.f)
				{
					time -= 1.f;

					playing_frame++;
					if (playing_frame == a.total_frame)
						playing_frame = loop ? 0 : -1;

					if (playing_frame == -1)
					{
						stop();
						break;
					}
				}
			}

			auto dst = renderer->set_armature_object_matrices(object_id);
			for (auto i = 0; i < bones.size(); i++)
				dst[i] = bones[i].calc_mat();

			frame = frames;
		}
	}

	void cArmaturePrivate::on_active()
	{
		apply_src();

		object_id = sNodeRenderer::instance()->register_armature_object();

		node->mark_transform_dirty();
	}

	void cArmaturePrivate::on_inactive()
	{
		stop();
		bones.clear();
		animations.clear();

		sNodeRenderer::instance()->unregister_armature_object(object_id);
		object_id = -1;
	}

	struct cArmatureCreatePrivate : cArmature::Create
	{
		cArmaturePtr operator()(EntityPtr e) override
		{
			return new cArmaturePrivate();
		}
	}cArmature_create_private;
	cArmature::Create& cArmature::create = cArmature_create_private;
}
