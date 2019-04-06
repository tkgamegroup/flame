#include <iostream>
#include <map>

#include <flame/global.h>
#include <flame/string.h>
#include <flame/math.h>
#include <flame/filesystem.h>
#include <flame/engine/core/core.h>
#include <flame/engine/graphics/buffer.h>
#include <flame/engine/entity/model.h>
#include <flame/engine/entity/animation.h>

namespace flame
{
	KeyFrame *AnimationTrack::new_keyframe()
	{
		auto k = new KeyFrame;
		keyframes.emplace_back(k);
		return k;
	}

	KeyFrame *AnimationTrack::new_keyframe(int frame)
	{
		auto it = keyframes.begin();
		for (; it != keyframes.end(); it++)
		{
			if ((*it)->frame > frame)
			{
				if (it != keyframes.begin())
				{
					if ((*(it - 1))->frame == frame)
						return nullptr;
				}
				break;
			}
		}
		auto k = std::make_unique<KeyFrame>();
		k->frame = frame;
		keyframes.insert(it, std::move(k));
		return k.get();
	}

	void AnimationTrack::remove_keyframe(KeyFrame *k)
	{
		for (auto it = keyframes.begin(); it != keyframes.end(); it++)
		{
			if (it->get() == k)
			{
				keyframes.erase(it);
				return;
			}
		}
	}

	AnimationTrack *Animation::new_track()
	{
		auto t = new AnimationTrack;
		tracks.emplace_back(t);
		return t;
	}

	void Animation::remove_track(AnimationTrack *t)
	{
		for (auto it = tracks.begin(); it != tracks.end(); it++)
		{
			if (it->get() == t)
			{
				tracks.erase(it);
				return;
			}
		}
	}

	namespace VMD
	{
#pragma pack(1)
		struct Header
		{
			char str[30];
			char modelName[20];
		};

		struct KeyFrameData
		{
			char name[15];
			int frame;
			glm::vec3 coord;
			glm::vec4 quaternion;
			char bezier[64];
		};
#pragma pack()

		void load(Animation *a, const std::string &filename)
		{
			std::ifstream file(filename, std::ios::binary);

			static_assert(sizeof(Header) == 50, "");
			static_assert(sizeof(KeyFrameData) == 111, "");

			Header header;
			file.read((char*)&header, sizeof(Header));

			auto count = read<int>(file);
			for (auto i = 0; i < count; i++)
			{
				KeyFrameData data;
				file.read((char*)&data, sizeof(KeyFrameData));
				auto name = japanese_to_chinese(data.name);
				AnimationTrack *t = nullptr;
				for (auto &_t : a->tracks)
				{
					if (_t->bone_name == name)
					{
						t = _t.get();
						break;
					}
				}
				if (!t)
				{
					t = a->new_track();
					t->bone_name = name;
				}
				auto k = t->new_keyframe(data.frame);
				k->frame = data.frame;
				k->coord = glm::vec3(data.coord);
				k->quaternion = glm::vec4(data.quaternion);
				k->coord.z *= -1.f;
				k->quaternion.z *= -1.f;
				k->quaternion.w *= -1.f;
			}
		}
	}

	namespace TKA
	{
		void load(Animation *a, const std::string &filename)
		{
			std::ifstream file(filename, std::ios::binary);

			auto track_count = read<int>(file);
			for (auto i = 0; i < track_count; i++)
			{
				auto t = a->new_track();
				t->bone_name = read_string(file);
				auto keyframe_count = read<int>(file);
				for (auto j = 0; j < keyframe_count; j++)
				{
					auto k = t->new_keyframe();
					k->frame = read<int>(file);
					k->coord = read<glm::vec3>(file);
					k->quaternion = read<glm::vec4>(file);
				}
			}
		}

		void save(Animation *a, const std::string &filename)
		{
			std::ofstream file(filename, std::ios::binary);

			write<int>(file, a->tracks.size());
			for (auto &t : a->tracks)
			{
				write_string(file, t->bone_name);
				write<int>(file, t->keyframes.size());
				for (auto &k : t->keyframes)
				{
					write<int>(file, k->frame);
					write<glm::vec3>(file, k->coord);
					write<glm::vec4>(file, k->quaternion);
				}
			}
		}
	}

	std::map<unsigned int, std::weak_ptr<Animation>> _animations;
	std::shared_ptr<Animation> getAnimation(const std::string &filename)
	{
		auto hash = HASH(filename.c_str());
		auto it = _animations.find(hash);
		if (it != _animations.end())
		{
			auto s = it->second.lock();
			if (s) return s;
		}

		std::experimental::filesystem::path path(filename);
		if (!std::experimental::filesystem::exists(filename))
			return nullptr;

		auto ext = path.extension().string();
		void(*load_func)(Animation *, const std::string &) = nullptr;
		if (ext == ".vmd")
			load_func = &VMD::load;
		else if (ext == ".t3a")
			load_func = &TKA::load;
		else
			return nullptr;

		auto a = std::make_shared<Animation>();
		a->filename = filename;
		load_func(a.get(), filename);

		_animations[hash] = a;
		return a;
	}

	std::vector<std::pair<Model *, std::weak_ptr<AnimationBinding>>> _animation_bindings;
	std::shared_ptr<AnimationBinding> get_animation_binding(Model *m, std::shared_ptr<Animation> anim)
	{
		for (auto &b : _animation_bindings)
		{
			if (b.first == m)
			{
				auto s = b.second.lock();
				if (s && s->animation == anim) 
					return s;
			}
		}

		auto b = std::make_shared<AnimationBinding>();
		b->animation = anim;
		for (auto &t : anim->tracks)
		{
			auto bone_index = -1;
			for (int i = 0; i < m->bones.size(); i++)
			{
				if (m->bones[i]->name == t->bone_name)
				{
					bone_index = i;
					break;
				}
			}
			if (bone_index == -1)
				continue;

			for (auto &k : t->keyframes)
				b->total_frame = glm::max(b->total_frame, k->frame);

			b->tracks.emplace_back(bone_index, t.get());
		}

		_animation_bindings.emplace_back(m, b);
	}

	AnimationRunner::AnimationRunner(Model *_model)
		:model(_model)
	{
		bone_data = std::make_unique<BoneData[]>(model->bones.size());
		bone_matrix = std::make_unique<glm::mat4[]>(model->bones.size());
		for (int i = 0; i < model->bones.size(); i++)
			bone_matrix[i] = glm::mat4(1.f);
		bone_buffer = std::make_unique<Buffer>(BufferTypeUniform, sizeof(glm::mat4) * model->bones.size());
		bone_buffer->update(bone_matrix.get(), sizeof(glm::mat4) * model->bones.size());
	}

	void AnimationRunner::reset_bones()
	{
		for (int i = 0; i < model->bones.size(); i++)
		{
			bone_data[i].coord = glm::vec3();
			bone_data[i].rotation = glm::mat3();
			bone_matrix[i] = glm::mat4();
		}
	}

	void AnimationRunner::refresh_bone()
	{
		assert(model);

		for (int i = 0; i < model->bones.size(); i++)
		{
			bone_matrix[i] = glm::translate(model->bones[i]->relateCoord + bone_data[i].coord) * glm::mat4(bone_data[i].rotation);
			if (model->bones[i]->parent != -1) 
				bone_matrix[i] = bone_matrix[model->bones[i]->parent] * bone_matrix[i];
		}
	}

	void AnimationRunner::refresh_bone(int i)
	{
		assert(model && i < model->bones.size());

		bone_matrix[i] = glm::translate(model->bones[i]->relateCoord + bone_data[i].coord) * glm::mat4(bone_data[i].rotation);
		if (model->bones[i]->parent != -1)
			bone_matrix[i] = bone_matrix[model->bones[i]->parent] * bone_matrix[i];

		for (auto child : model->bones[i]->children)
			refresh_bone(child);
	}

	void AnimationRunner::set_animation(AnimationBinding *animation)
	{
		if (curr_anim == animation)
			return;

		reset_bones();
		bone_buffer->update(bone_matrix.get(), sizeof(glm::mat4) * model->bones.size());
		curr_anim = animation;
		curr_frame = 0;
		curr_frame_index.resize(animation->tracks.size());
		for (auto &i : curr_frame_index)
			i = 0;
	}

	void AnimationRunner::update()
	{
		const float dist = 1.f / 60.f;

		if (curr_anim)
		{
			reset_bones();

			for (int i = 0; i < curr_frame_index.size(); i++)
			{
				auto &t = curr_anim->tracks[i];
				auto index = curr_frame_index[i];
				auto left_keyframe = t.second->keyframes[index].get();
				index++;
				if (index == t.second->keyframes.size())
					index = 0;
				auto right_keyframe = t.second->keyframes[index].get();

				auto beta = 0.f;
				if (left_keyframe != right_keyframe)
				{
					beta = (curr_frame - left_keyframe->frame) /
						(right_keyframe->frame - left_keyframe->frame);
				}

				auto data = &bone_data[t.first];
				data->rotation = flame::quaternion_to_mat3(glm::normalize((1.f - beta) *
					left_keyframe->quaternion + beta * right_keyframe->quaternion));
				data->coord = left_keyframe->coord + (right_keyframe->coord - 
					right_keyframe->coord) * beta;
			}

			bool wrap = false;
			curr_frame += elapsed_time / dist;
			if (curr_anim->total_frame > 0)
			{
				if (curr_frame >= curr_anim->total_frame)
				{
					curr_frame = glm::mod(curr_frame, (float)curr_anim->total_frame);
					wrap = true;
				}
			}

			auto dst = curr_frame;
			if (wrap)
				dst += curr_anim->total_frame;
			for (int i = 0; i < curr_frame_index.size(); i++)
			{
				auto &t = curr_anim->tracks[i];
				if (t.second->keyframes.size() == 0)
					continue;
				auto index = curr_frame_index[i];
				while (t.second->keyframes[index]->frame <= dst)
				{
					index++;
					if (index == t.second->keyframes.size())
					{
						index = 0;
						dst -= curr_anim->total_frame;
					}
				}
				if (index == 0)
					index = t.second->keyframes.size() - 1;
				else
					index -= 1;
				curr_frame_index[i] = index;
			}
		}

		refresh_bone();

		if (enable_IK)
		{
			//	for (int i = 0; i < pModel->iks.size(); i++)
			//	{
			//		auto &ik = pModel->iks[i];
			//		auto t = glm::vec3(boneMatrix[ik.targetID][3]);
			//		//t.z *= -1.f;
			//		for (int times = 0; times < ik.iterations; times++)
			//		{
			//			for (int index = 0; index < ik.chain.size(); index++)
			//			{
			//				//index = iChain;
			//				auto e = glm::vec3(boneMatrix[ik.effectorID][3]);
			//				//e.z *= -1.f;
			//				if (glm::length(t - e) < 0.0001f)
			//				{
			//					goto nextIk;
			//				}

			//				auto boneID = ik.chain[index];

			//				auto p = glm::vec3(boneMatrix[boneID][3]);
			//				//p.z *= -1.f;

			//				auto pe = glm::normalize(e - p);
			//				auto pt = glm::normalize(t - p);
			//				auto theDot = glm::dot(pe, pt);
			//				theDot = glm::clamp(theDot, 0.f, 1.f);
			//				auto theAcos = glm::acos(theDot);
			//				auto ang = glm::degrees(theAcos);
			//				if (glm::abs(ang) > 0.5f)
			//				{
			//					auto n = glm::normalize(glm::cross(pe, pt));
			//					if (glm::abs(n.z) < 1.f)
			//					{
			//						n.z = 0;
			//						n = glm::normalize(n);
			//					}
			//					boneData[boneID].rotation = glm::mat3(glm::rotate(ang, n)) * boneData[boneID].rotation;
			//					//refreshBone(ik.effectorID, boneData, outMat);
			//					pModel->refreshBone(boneID, boneData, boneMatrix);
			//					p = glm::vec3(boneMatrix[boneID][3]);
			//					e = glm::vec3(boneMatrix[ik.effectorID][3]);
			//					pe = glm::normalize(e - p);
			//					auto dot = glm::dot(pe, pt);
			//					int cut = 1;
			//				}
			//				//break;
			//			}
			//		}
			//	nextIk:
			//		//break;
			//		continue;
			//	}
			//}
		}

		for (int i = 0; i < model->bones.size(); i++)
			bone_matrix[i] *= glm::translate(-model->bones[i]->rootCoord);

		bone_buffer->update(bone_matrix.get(), sizeof(glm::mat4) * model->bones.size());
	}
}
