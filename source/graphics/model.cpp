#include <flame/serialize.h>
#include <flame/foundation/foundation.h>
#include "model_private.h"

#ifdef USE_ASSIMP
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#endif

#include <functional>

namespace flame
{
	namespace graphics
	{
		Material* Material::create(const wchar_t* filename, const char* defines)
		{
			auto ret = new MaterialPrivate;
			ret->pipeline_file = filename;
			ret->pipeline_defines = defines;
			return ret;
		}

		void MeshPrivate::add_vertices(uint n, Vec3f* _positions, Vec3f* _uvs, Vec3f* _normals)
		{
			auto b = positions.size();
			positions.resize(b + n);
			for (auto i = 0; i < n; i++)
			{
				auto& p = _positions[i];
				positions[b + i] = p;
				lower_bound = min(lower_bound, p);
				upper_bound = max(upper_bound, p);
			}
			if (_uvs)
			{
				uvs.resize(b + n);
				for (auto i = 0; i < n; i++)
					uvs[b + i] = _uvs[i];
			}
			if (_normals)
			{
				normals.resize(b + n);
				for (auto i = 0; i < n; i++)
					normals[b + i] = _normals[i];
			}
		}

		void MeshPrivate::add_indices(uint n, uint* _indices)
		{
			auto b = indices.size();
			indices.resize(b + n);
			for (auto i = 0; i < n; i++)
				indices[b + i] = _indices[i];
		}

		void MeshPrivate::add_cube(const Vec3f& extent, const Vec3f& center, const Mat3f& rotation)
		{
			positions.push_back(rotation * Vec3f(-0.5f, +0.5f, +0.5f) * extent + center);
			positions.push_back(rotation * Vec3f(+0.5f, +0.5f, +0.5f) * extent + center);
			positions.push_back(rotation * Vec3f(+0.5f, -0.5f, +0.5f) * extent + center);
			positions.push_back(rotation * Vec3f(-0.5f, -0.5f, +0.5f) * extent + center);
			normals.push_back(Vec3f(+0.f, +0.f, +1.f));
			normals.push_back(Vec3f(+0.f, +0.f, +1.f));
			normals.push_back(Vec3f(+0.f, +0.f, +1.f));
			normals.push_back(Vec3f(+0.f, +0.f, +1.f));
			indices.push_back(0); indices.push_back(2); indices.push_back(1);
			indices.push_back(0); indices.push_back(3); indices.push_back(2);

			positions.push_back(rotation * Vec3f(-0.5f, +0.5f, -0.5f) * extent + center);
			positions.push_back(rotation * Vec3f(+0.5f, +0.5f, -0.5f) * extent + center);
			positions.push_back(rotation * Vec3f(+0.5f, -0.5f, -0.5f) * extent + center);
			positions.push_back(rotation * Vec3f(-0.5f, -0.5f, -0.5f) * extent + center);
			normals.push_back(Vec3f(+0.f, +0.f, -1.f));
			normals.push_back(Vec3f(+0.f, +0.f, -1.f));
			normals.push_back(Vec3f(+0.f, +0.f, -1.f));
			normals.push_back(Vec3f(+0.f, +0.f, -1.f));
			indices.push_back(5); indices.push_back(7); indices.push_back(4);
			indices.push_back(5); indices.push_back(6); indices.push_back(7);

			positions.push_back(rotation * Vec3f(-0.5f, +0.5f, -0.5f) * extent + center);
			positions.push_back(rotation * Vec3f(+0.5f, +0.5f, -0.5f) * extent + center);
			positions.push_back(rotation * Vec3f(+0.5f, +0.5f, +0.5f) * extent + center);
			positions.push_back(rotation * Vec3f(-0.5f, +0.5f, +0.5f) * extent + center);
			normals.push_back(Vec3f(+0.f, +1.f, +0.f));
			normals.push_back(Vec3f(+0.f, +1.f, +0.f));
			normals.push_back(Vec3f(+0.f, +1.f, +0.f));
			normals.push_back(Vec3f(+0.f, +1.f, +0.f));
			indices.push_back(8); indices.push_back(10); indices.push_back(9);
			indices.push_back(8); indices.push_back(11); indices.push_back(10);

			positions.push_back(rotation * Vec3f(-0.5f, -0.5f, -0.5f) * extent + center);
			positions.push_back(rotation * Vec3f(+0.5f, -0.5f, -0.5f) * extent + center);
			positions.push_back(rotation * Vec3f(+0.5f, -0.5f, +0.5f) * extent + center);
			positions.push_back(rotation * Vec3f(-0.5f, -0.5f, +0.5f) * extent + center);
			normals.push_back(Vec3f(+0.f, -1.f, +0.f));
			normals.push_back(Vec3f(+0.f, -1.f, +0.f));
			normals.push_back(Vec3f(+0.f, -1.f, +0.f));
			normals.push_back(Vec3f(+0.f, -1.f, +0.f));
			indices.push_back(15); indices.push_back(13); indices.push_back(14);
			indices.push_back(15); indices.push_back(12); indices.push_back(13);

			positions.push_back(rotation * Vec3f(-0.5f, +0.5f, -0.5f) * extent + center);
			positions.push_back(rotation * Vec3f(-0.5f, +0.5f, +0.5f) * extent + center);
			positions.push_back(rotation * Vec3f(-0.5f, -0.5f, +0.5f) * extent + center);
			positions.push_back(rotation * Vec3f(-0.5f, -0.5f, -0.5f) * extent + center);
			normals.push_back(Vec3f(-1.f, +0.f, +0.f));
			normals.push_back(Vec3f(-1.f, +0.f, +0.f));
			normals.push_back(Vec3f(-1.f, +0.f, +0.f));
			normals.push_back(Vec3f(-1.f, +0.f, +0.f));
			indices.push_back(16); indices.push_back(18); indices.push_back(17);
			indices.push_back(16); indices.push_back(19); indices.push_back(18);

			positions.push_back(rotation * Vec3f(+0.5f, +0.5f, -0.5f) * extent + center);
			positions.push_back(rotation * Vec3f(+0.5f, +0.5f, +0.5f) * extent + center);
			positions.push_back(rotation * Vec3f(+0.5f, -0.5f, +0.5f) * extent + center);
			positions.push_back(rotation * Vec3f(+0.5f, -0.5f, -0.5f) * extent + center);
			normals.push_back(Vec3f(+1.f, +0.f, +0.f));
			normals.push_back(Vec3f(+1.f, +0.f, +0.f));
			normals.push_back(Vec3f(+1.f, +0.f, +0.f));
			normals.push_back(Vec3f(+1.f, +0.f, +0.f));
			indices.push_back(21); indices.push_back(23); indices.push_back(20);
			indices.push_back(21); indices.push_back(22); indices.push_back(23);
		}

		void MeshPrivate::add_sphere(float radius, uint horiSubdiv, uint vertSubdiv, const Vec3f& center, const Mat3f& rotation)
		{
			std::vector<std::vector<int>> staging_indices;
			staging_indices.resize(horiSubdiv + 1);

			for (int level = 1; level < horiSubdiv; level++)
			{
				for (int i = 0; i < vertSubdiv; i++)
				{
					auto radian = (level * 180.f / horiSubdiv - 90.f) * ANG_RAD;
					auto ring_radius = cos(radian) * radius;
					auto height = sin(radian) * radius;
					auto ang = (i * 360.f / vertSubdiv) * ANG_RAD;
					staging_indices[level].push_back(positions.size());
					auto p = rotation * Vec3f(cos(ang) * ring_radius, height, sin(ang) * ring_radius);
					normals.push_back(p);
					positions.push_back(p + center);
				}
			}

			{
				staging_indices[0].push_back(positions.size());
				auto p = rotation * Vec3f(0.f, -radius, 0.f);
				normals.push_back(p);
				positions.push_back(p + center);
			}

			{
				staging_indices[horiSubdiv].push_back(positions.size());
				auto p = rotation * Vec3f(0.f, radius, 0.f);
				normals.push_back(p);
				positions.push_back(p + center);
			}

			for (int level = 0; level < horiSubdiv; level++)
			{
				if (level == 0)
				{
					for (int i = 0; i < vertSubdiv; i++)
					{
						auto ii = i + 1; if (ii == vertSubdiv) ii = 0;

						indices.push_back(staging_indices[0][0]);
						indices.push_back(staging_indices[1][i]);
						indices.push_back(staging_indices[1][ii]);
					}
				}
				else if (level == horiSubdiv - 1)
				{
					for (int i = 0; i < vertSubdiv; i++)
					{
						auto ii = i + 1; if (ii == vertSubdiv) ii = 0;

						indices.push_back(staging_indices[horiSubdiv - 1][i]);
						indices.push_back(staging_indices[horiSubdiv][0]);
						indices.push_back(staging_indices[horiSubdiv - 1][ii]);
					}
				}
				else
				{
					for (int i = 0; i < vertSubdiv; i++)
					{
						auto ii = i + 1; if (ii == vertSubdiv) ii = 0;

						indices.push_back(staging_indices[level][i]);
						indices.push_back(staging_indices[level + 1][i]);
						indices.push_back(staging_indices[level][ii]);

						indices.push_back(staging_indices[level][ii]);
						indices.push_back(staging_indices[level + 1][i]);
						indices.push_back(staging_indices[level + 1][ii]);
					}
				}
			}
		}

		NodePrivate* NodePrivate::find(const std::string& _name)
		{
			if (name == _name)
				return this;
			for (auto& c : children)
			{
				auto ret = c->find(_name);
				if (ret)
					return ret;
			}
			return nullptr;
		}

		void NodePrivate::traverse(const std::function<void(NodePrivate*)>& callback)
		{
			callback(this);
			for (auto& c : children)
				c->traverse(callback);
		}

		ModelPrivate::ModelPrivate()
		{
			materials.emplace_back(new MaterialPrivate);
			root.reset(new NodePrivate);
		}

		int ModelPrivate::find_mesh(const std::string& name) const
		{
			for (auto i = 0; i < meshes.size(); i++)
			{
				if (meshes[i]->name == name)
					return i;
			}
			return -1;
		}

		int ModelPrivate::find_animation(const std::string& name) const
		{
			for (auto i = 0; i < animations.size(); i++)
			{
				if (animations[i]->name == name)
					return i;
			}
			return -1;
		}

		void ModelPrivate::save(const std::filesystem::path& filename) const
		{
			auto extension = filename.extension();

			if (extension == L".fb")
			{
				std::ofstream file(filename, std::ios::binary);

				write_u(file, materials.size());
				for (auto& m : materials)
				{
					write_s(file, m->name);
					write_t(file, m->color);
					write_t(file, m->metallic);
					write_t(file, m->roughness);
					write_t(file, m->alpha_test);
					for (auto i = 0; i < size(m->textures); i++)
						write_s(file, m->textures[i].string());
					write_s(file, m->pipeline_file.string());
					write_s(file, m->pipeline_defines);
				}

				write_u(file, meshes.size());
				for (auto& m : meshes)
				{
					write_s(file, m->name);
					write_i(file, m->material_index);
					auto n = m->positions.size();
					write_u(file, n);
					file.write((char*)m->positions.data(), sizeof(Vec3f) * n);
					write_b(file, !m->uvs.empty());
					if (!m->uvs.empty())
						file.write((char*)m->uvs.data(), sizeof(Vec2f) * n);
					write_b(file, !m->normals.empty());
					if (!m->normals.empty())
						file.write((char*)m->normals.data(), sizeof(Vec3f) * n);
					write_v(file, m->indices);
					write_u(file, m->bones.size());
					for (auto& b : m->bones)
					{
						write_s(file, b->name);
						write_t(file, b->offset_matrix);
						write_v(file, b->weights);
					}
					write_t(file, m->lower_bound);
					write_t(file, m->upper_bound);
				}

				root->traverse([&](NodePrivate* n) {
					write_s(file, n->name);
					write_t(file, n->pos);
					write_t(file, n->quat);
					write_t(file, n->scale);
					write_i(file, n->mesh_index);
					write_u(file, n->children.size());
				});

				write_u(file, animations.size());
				for (auto& a : animations)
				{
					write_s(file, a->name);
					write_u(file, a->channels.size());
					for (auto& c : a->channels)
					{
						write_s(file, c->node_name);
						write_v(file, c->position_keys);
						write_v(file, c->rotation_keys);
					}
				}

				file.close();
			}
			else if (extension == L".fm")
			{
				pugi::xml_document doc;
				auto doc_root = doc.append_child("model");

				auto n_materials = doc_root.append_child("materials");
				for (auto& m : materials)
				{
					auto n_material = n_materials.append_child("material");
					n_material.append_attribute("name").set_value(m->name.c_str());
					n_material.append_attribute("color").set_value(to_string(m->color).c_str());
					n_material.append_attribute("metallic").set_value(to_string(m->metallic).c_str());
					n_material.append_attribute("roughness").set_value(to_string(m->roughness).c_str());
					n_material.append_attribute("alpha_test").set_value(to_string(m->alpha_test).c_str());
					n_material.append_attribute("pipeline_file").set_value(m->pipeline_file.string().c_str());
					n_material.append_attribute("pipeline_defines").set_value(m->pipeline_defines.c_str());
					n_material.append_attribute("dir").set_value(m->dir.string().c_str());
					auto n_textures = n_material.append_child("textures");
					for (auto& t : m->textures)
						n_textures.append_child("texture").append_attribute("path").set_value(t.string().c_str());
				}

				auto n_meshes = doc_root.append_child("meshes");
				for (auto& m : meshes)
				{
					auto n_mesh = n_meshes.append_child("mesh");
					n_mesh.append_attribute("name").set_value(m->name.c_str());
					n_mesh.append_attribute("material_index").set_value(m->material_index);
					n_mesh.append_child("positions").append_attribute("data").set_value(base64_encode(std::string((char*)m->positions.data(), m->positions.size() * sizeof(Vec3f))).c_str());
					if (!m->uvs.empty())
						n_mesh.append_child("uvs").append_attribute("data").set_value(base64_encode(std::string((char*)m->uvs.data(), m->uvs.size() * sizeof(Vec2f))).c_str());
					if (!m->normals.empty())
						n_mesh.append_child("normals").append_attribute("data").set_value(base64_encode(std::string((char*)m->normals.data(), m->normals.size() * sizeof(Vec3f))).c_str());
				}

				std::function<void(NodePrivate* src, pugi::xml_node dst)> save_node;
				save_node = [&](NodePrivate* src, pugi::xml_node dst) {
					dst.append_attribute("name").set_value(src->name.c_str());
					dst.append_attribute("pos").set_value(to_string(src->pos).c_str());
					dst.append_attribute("quat").set_value(to_string(src->quat).c_str());
					dst.append_attribute("scale").set_value(to_string(src->scale).c_str());
					dst.append_attribute("mesh_index").set_value(src->mesh_index);
					for (auto& c : src->children)
					{
						auto n = dst.append_child("node");
						save_node(c.get(), n);
					}
				};

				save_node(root.get(), doc_root.append_child("node"));

				auto n_animations = doc_root.append_child("animations");
				for (auto& a : animations)
				{
					auto n_animation = n_animations.append_child("animation");
					n_animation.append_attribute("name").set_value(a->name.c_str());
					auto n_channels = n_animation.append_child("channels");
					for (auto& c : a->channels) 
					{
						auto n_channel = n_channels.append_child("channel");
						n_channel.append_attribute("node_name").set_value(c->node_name.c_str());
						n_channel.append_child("position_keys").append_attribute("data").set_value(base64_encode(std::string((char*)c->position_keys.data(), c->position_keys.size() * sizeof(PositionKey))).c_str());
						n_channel.append_child("rotation_keys").append_attribute("data").set_value(base64_encode(std::string((char*)c->rotation_keys.data(), c->rotation_keys.size() * sizeof(RotationKey))).c_str());
					}
				}

				doc.save_file(filename.c_str());
			}
		}

		void ModelPrivate::generate_prefab(const std::filesystem::path& filename) const
		{
			pugi::xml_document prefab;

			auto model_name = filename.filename().string();
			std::function<void(pugi::xml_node, NodePrivate*)> print_node;
			print_node = [&](pugi::xml_node dst, NodePrivate* src) {
				auto n = dst.append_child("entity");
				n.append_attribute("name").set_value(src->name.c_str());
				auto nn = n.append_child("cNode");
				nn.append_attribute("pos").set_value(to_string(src->pos).c_str());
				nn.append_attribute("quat").set_value(to_string(src->quat).c_str());
				nn.append_attribute("scale").set_value(to_string(src->scale).c_str());
				if (src->mesh_index != -1)
				{
					auto nm = n.append_child("cMesh");
					nm.append_attribute("src").set_value((model_name + "." + meshes[src->mesh_index]->name).c_str());
					if (src->name.starts_with("sm_"))
					{
						auto nr = n.append_child("cRigid");
						nr.append_attribute("dynamic").set_value(false);
						auto ns = n.append_child("cShape");
						ns.append_attribute("type").set_value("Mesh");
					}
				}
				else
				{
					if (src->name.starts_with("trigger_"))
					{
						auto nr = n.append_child("cRigid");
						nr.append_attribute("dynamic").set_value(false);
						auto ns = n.append_child("cShape");
						ns.append_attribute("type").set_value("Cube");
						ns.append_attribute("size").set_value("2,2,0.01");
						ns.append_attribute("trigger").set_value(true);
					}
				}
				for (auto& c : src->children)
					print_node(n, c.get());
			};
			print_node(prefab.append_child("prefab"), root.get());

			auto prefab_path = filename;
			prefab_path.replace_extension(L".prefab");
			prefab.save_file(prefab_path.c_str());
		}

		static ModelPrivate* standard_cube = nullptr;
		static ModelPrivate* standard_sphere = nullptr;

		Model* Model::get_standard(const char* _name)
		{
			auto name = std::string(_name);
			if (name == "cube")
			{
				if (!standard_cube)
				{
					auto m = new ModelPrivate;
					auto mesh = new MeshPrivate;
					mesh->name = "0";
					mesh->add_cube(Vec3f(1.f), Vec3f(0.f), Mat3f(1.f));
					m->meshes.emplace_back(mesh);
					m->root->mesh_index = 0;

					standard_cube = m;
				}
				return standard_cube;
			}
			else if (name == "sphere")
			{
				if (!standard_sphere)
				{
					auto m = new ModelPrivate;
					auto mesh = new MeshPrivate;
					mesh->name = "0";
					mesh->add_sphere(0.5f, 12, 12, Vec3f(0.f), Mat3f(1.f));
					m->meshes.emplace_back(mesh);
					m->root->mesh_index = 0;

					standard_sphere = m;
				}
				return standard_sphere;
			}
			return nullptr;
		}

		ModelPrivate* ModelPrivate::create(const std::filesystem::path& filename)
		{
			if (!std::filesystem::exists(filename))
			{
				wprintf(L"cannot find model: %s\n", filename.c_str());
				return nullptr;
			}

			ModelPrivate* ret = nullptr;

			auto extension = filename.extension();

			if (extension == L".fb")
			{
				ret = new ModelPrivate();
				ret->filename = filename;

				std::ifstream file(filename, std::ios::binary);

				ret->materials.resize(read_u(file));
				for (auto i = 0; i < ret->materials.size(); i++)
				{
					auto m = new MaterialPrivate;
					m->dir = filename.parent_path();
					ret->materials[i].reset(m);

					read_s(file, m->name);

					read_t(file, m->color);
					read_t(file, m->metallic);
					read_t(file, m->roughness);
					read_t(file, m->alpha_test);

					for (auto i = 0; i < size(m->textures); i++)
					{
						std::string str;
						read_s(file, str);
						m->textures[i] = str;
					}

					{
						std::string str;
						read_s(file, str);
						m->pipeline_file = str;
					}
					read_s(file, m->pipeline_defines);
				}

				ret->meshes.resize(read_u(file));
				for (auto i = 0; i < ret->meshes.size(); i++)
				{
					auto m = new MeshPrivate;
					ret->meshes[i].reset(m);

					read_s(file, m->name);

					m->material_index = read_i(file);

					auto n = read_u(file);
					m->positions.resize(n);
					file.read((char*)m->positions.data(), sizeof(Vec3f) * n);
					if (read_b(file))
					{
						m->uvs.resize(n);
						file.read((char*)m->uvs.data(), sizeof(Vec2f) * n);
					}
					if (read_b(file))
					{
						m->normals.resize(n);
						file.read((char*)m->normals.data(), sizeof(Vec3f) * n);
					}
					read_v(file, m->indices);

					m->bones.resize(read_u(file));
					for (auto j = 0; j < m->bones.size(); j++)
					{
						auto b = new BonePrivate;
						m->bones[j].reset(b);

						read_s(file, b->name);
						read_t(file, b->offset_matrix);
						read_v(file, b->weights);
					}

					read_t(file, m->lower_bound);
					read_t(file, m->upper_bound);
				}

				std::function<void(NodePrivate*)> load_node;
				load_node = [&](NodePrivate* n) {
					read_s(file, n->name);

					read_t(file, n->pos);
					read_t(file, n->quat);
					read_t(file, n->scale);

					n->mesh_index = read_i(file);

					n->children.resize(read_u(file));
					for (auto i = 0; i < n->children.size(); i++)
					{
						auto c = new NodePrivate;
						n->children[i].reset(c);
						load_node(c);
					}
				};
				load_node(ret->root.get());

				ret->animations.resize(read_u(file));
				for (auto i = 0; i < ret->animations.size(); i++)
				{
					auto a = new AnimationPrivate;
					ret->animations[i].reset(a);

					read_s(file, a->name);

					a->channels.resize(read_u(file));
					for (auto j = 0; j < a->channels.size(); j++)
					{
						auto c = new ChannelPrivate;
						a->channels[j].reset(c);
						read_s(file, c->node_name);
						read_v(file, c->position_keys);
						read_v(file, c->rotation_keys);
					}
				}

				file.close();
			}
			else if (extension == L".fm")
			{
				pugi::xml_document doc;
				pugi::xml_node doc_root;
				if (!doc.load_file(filename.c_str()) || (doc_root = doc.first_child()).name() != std::string("model"))
				{
					printf("model does not exist: %s\n", filename.string().c_str());
					return nullptr;
				}

				ret = new ModelPrivate();
				ret->filename = filename;

				for (auto n_material : doc_root.child("materials"))
				{
					auto m = new MaterialPrivate;
					m->name = n_material.attribute("name").value();
					m->color = sto<Vec3f>(n_material.attribute("color").value());
					m->metallic = sto<float>(n_material.attribute("metallic").value());
					m->roughness = sto<float>(n_material.attribute("roughness").value());
					m->alpha_test = sto<float>(n_material.attribute("alpha_test").value());
					m->pipeline_file = n_material.attribute("pipeline_file").value();
					m->pipeline_defines = n_material.attribute("pipeline_defines").value();
					m->dir = n_material.attribute("dir").value();
					auto itex = 0;
					for (auto n_texture : n_material.child("textures"))
					{
						m->textures[itex] = n_texture.attribute("path").value();
						itex++;
					}
					ret->materials.emplace_back(m);
				}

				for (auto& n_mesh : doc_root.child("meshes"))
				{
					auto m = new MeshPrivate;
					m->name = n_mesh.attribute("name").value();
					m->material_index = n_mesh.attribute("material_index").as_uint();
					{
						auto str = base64_decode(n_mesh.child("positions").attribute("data").value());
						m->positions.resize(str.length() / sizeof(Vec3f));
						memcpy(m->positions.data(), str.data(), str.size());
					}
					{
						auto n = n_mesh.child("uvs");
						if (n)
						{
							auto str = base64_decode(n.attribute("data").value());
							m->uvs.resize(str.length() / sizeof(Vec2f));
							memcpy(m->uvs.data(), str.data(), str.size());
						}
					}
					{
						auto n = n_mesh.child("normals");
						if (n)
						{
							auto str = base64_decode(n.attribute("data").value());
							m->normals.resize(str.length() / sizeof(Vec3f));
							memcpy(m->normals.data(), str.data(), str.size());
						}
					}
					ret->meshes.emplace_back(m);
				}

				std::function<void(pugi::xml_node src, NodePrivate* dst)> load_node;
				load_node = [&](pugi::xml_node src, NodePrivate* dst) {
					dst->name = src.attribute("name").value();
					dst->pos = sto<Vec3f>(src.attribute("pos").value());
					dst->quat = sto<Vec4f>(src.attribute("quat").value());
					dst->scale = sto<Vec3f>(src.attribute("scale").value());
					dst->mesh_index = src.attribute("scale").as_int();
					for (auto c : src.children())
					{
						auto n = new NodePrivate;
						load_node(c, n);
						dst->children.emplace_back(n);
					}
				};

				load_node(doc_root.child("node"), ret->root.get());

				for (auto n_animation : doc_root.child("animations"))
				{
					auto a = new AnimationPrivate;
					a->name = n_animation.attribute("name").value();
					for (auto n_channel : n_animation.child("channels"))
					{
						auto c = new ChannelPrivate;
						c->node_name = n_channel.attribute("node_name").value();
						{
							auto str = base64_decode(n_channel.child("position_keys").attribute("data").value());
							c->position_keys.resize(str.size() / sizeof(PositionKey));
							memcpy(c->position_keys.data(), str.data(), str.size());
						}
						{
							auto str = base64_decode(n_channel.child("rotation_keys").attribute("data").value());
							c->rotation_keys.resize(str.size() / sizeof(RotationKey));
							memcpy(c->rotation_keys.data(), str.data(), str.size());
						}
						a->channels.emplace_back(c);
					}
					ret->animations.emplace_back(a);
				}

				return ret;
			}
			else
			{
#ifdef USE_ASSIMP
				Assimp::Importer importer;
				auto load_flags = 
					aiProcess_RemoveRedundantMaterials |
					aiProcess_Triangulate |
					aiProcess_JoinIdenticalVertices |
					aiProcess_SortByPType |
					aiProcess_FlipUVs;
				auto scene = importer.ReadFile(filename.string(), load_flags);
				if (!scene)
				{
					printf("load model %s failed: %s\n", filename.string().c_str(), importer.GetErrorString());
					return nullptr;
				}

				ret = new ModelPrivate();

				ret->materials.clear();
				for (auto i = 0; i < scene->mNumMaterials; i++)
				{
					auto src = scene->mMaterials[i];
					auto dst = new MaterialPrivate;
					ret->materials.emplace_back(dst);

					aiString name;
					aiColor3D color;
					ai_real shininess;

					dst->name = src->GetName().C_Str();

					std::string filename;

					name.Clear();
					src->GetTexture(aiTextureType_DIFFUSE, 0, &name);
					filename = name.C_Str();
					if (!filename.empty())
					{
						if (filename[0] == '/')
							filename.erase(filename.begin());
						dst->textures[0] = filename;
						dst->pipeline_defines += "COLOR_MAP ";
					}

					name.Clear();
					src->GetTexture(aiTextureType_OPACITY, 0, &name);
					filename = name.C_Str();
					if (!filename.empty())
					{
						if (filename[0] == '/')
							filename.erase(filename.begin());
						dst->textures[1] = filename;
						dst->pipeline_defines += "ALPHA_MAP ";
					}
				}

				for (auto i = 0; i < scene->mNumMeshes; i++)
				{
					auto src = scene->mMeshes[i];
					auto dst = new MeshPrivate;
					ret->meshes.emplace_back(dst);

					dst->name = src->mName.C_Str();
					for (auto& c : dst->name)
					{
						if (c == '.')
							c = '_';
					}
					if (dst->name.empty())
						dst->name = std::to_string(i);

					dst->material_index = src->mMaterialIndex;

					dst->add_vertices(src->mNumVertices, (Vec3f*)src->mVertices, (Vec3f*)src->mTextureCoords[0], (Vec3f*)src->mNormals);

					std::vector<uint> indices(src->mNumFaces * 3);
					for (auto j = 0; j < src->mNumFaces; j++)
					{
						indices[j * 3 + 0] = src->mFaces[j].mIndices[0];
						indices[j * 3 + 1] = src->mFaces[j].mIndices[1];
						indices[j * 3 + 2] = src->mFaces[j].mIndices[2];
					}
					dst->add_indices(indices.size(), indices.data());

					for (auto j = 0; j < src->mNumBones; j++)
					{
						auto src_b = src->mBones[j];
						auto dst_b = new BonePrivate;
						dst->bones.emplace_back(dst_b);

						dst_b->name = src_b->mName.C_Str();
						{
							auto& m = src_b->mOffsetMatrix;
							dst_b->offset_matrix = Mat4f(
								Vec4f(m.a1, m.b1, m.c1, m.d1),
								Vec4f(m.a2, m.b2, m.c2, m.d2),
								Vec4f(m.a3, m.b3, m.c3, m.d3),
								Vec4f(m.a4, m.b4, m.c4, m.d4)
							);
						}
						dst_b->weights.resize(src_b->mNumWeights);
						for (auto k = 0; k < src_b->mNumWeights; k++)
						{
							auto& src_w = src_b->mWeights[k];
							auto& dst_w = dst_b->weights[k];
							dst_w.first = src_w.mVertexId;
							dst_w.second = src_w.mWeight;
						}
					}
				}

				for (auto i = 0; i < ret->meshes.size(); i++)
				{
					auto name = ret->meshes[i]->name;
					auto n = 0;
					auto unique = [&]() {
						for (auto j = 0; j < ret->meshes.size(); j++)
						{
							if (ret->meshes[j]->name == name)
								return false;
						}
						return true;
					};
					while (!unique())
					{
						name = ret->meshes[i]->name + std::to_string(i);
						n++;
					}
					if (n > 0)
						ret->meshes[i]->name = name;
				}

				std::vector<uint> mesh_refs;
				mesh_refs.resize(ret->meshes.size());

				std::function<void(NodePrivate*, aiNode*)> get_node;
				get_node = [&](NodePrivate* dst, aiNode* src) {
					dst->name = std::string(src->mName.C_Str());

					{
						aiVector3D s;
						aiVector3D r;
						ai_real a;
						aiVector3D p;
						src->mTransformation.Decompose(s, r, a, p);
						dst->pos = Vec3f(p.x, p.y, p.z);
						dst->quat = make_quat(a * RAD_ANG, Vec3f(r.x, r.y, r.z));
						dst->scale = Vec3f(s.x, s.y, s.z);
					}

					if (src->mNumMeshes > 0)
					{
						dst->mesh_index = src->mMeshes[0];
						if (dst->name.starts_with("trigger_"))
						{
							auto& m = ret->meshes[dst->mesh_index];
							if (m->positions.size() == 4 && m->indices.size() == 6)
							{
								// plane
								// TODO
							}
							dst->mesh_index = -1;
						}
					}
					if (dst->mesh_index != -1)
						mesh_refs[dst->mesh_index]++;

					for (auto i = 0; i < src->mNumChildren; i++)
					{
						auto n = new NodePrivate;
						dst->children.emplace_back(n);
						get_node(n, src->mChildren[i]);
					}
				};
				{
					auto n = new NodePrivate;
					ret->root->children.emplace_back(n);
					get_node(n, scene->mRootNode);
				}

				for (auto i = 0; i < scene->mNumAnimations; i++)
				{
					auto src = scene->mAnimations[i];
					auto dst = new AnimationPrivate;
					ret->animations.emplace_back(dst);

					dst->name = src->mName.C_Str();
					dst->channels.resize(src->mNumChannels);
					for (auto j = 0; j < src->mNumChannels; j++)
					{
						auto src_c = src->mChannels[j];
						auto dst_c = new ChannelPrivate;
						dst->channels[j].reset(dst_c);

						dst_c->node_name = src_c->mNodeName.C_Str();

						dst_c->position_keys.resize(src_c->mNumPositionKeys);
						for (auto k = 0; k < src_c->mNumPositionKeys; k++)
						{
							auto& src_k = src_c->mPositionKeys[k];
							auto& dst_k = dst_c->position_keys[k];
							dst_k.t = src_k.mTime;
							auto& p = src_k.mValue;
							dst_k.v = Vec3f(p.x, p.y, p.z);
						}
						dst_c->rotation_keys.resize(src_c->mNumRotationKeys);
						for (auto k = 0; k < src_c->mNumRotationKeys; k++)
						{
							auto& src_k = src_c->mRotationKeys[k];
							auto& dst_k = dst_c->rotation_keys[k];
							dst_k.t = src_k.mTime;
							auto& q = src_k.mValue;
							dst_k.v = Vec4f(q.x, q.y, q.z, q.w);
						}
					}
				}

				for (auto i = 0; i < mesh_refs.size(); i++)
				{
					if (mesh_refs[i] == 0)
					{
						auto n = new NodePrivate;
						n->mesh_index = i;
						ret->root->children.emplace_back(n);
					}
				}
#endif
			}

			if (ret && ret->root->children.empty())
			{
				auto n = new NodePrivate;
				auto r = ret->root.release();
				n->children.emplace_back(r);
				ret->root.reset(n);
			}

			return ret;
		}

		Model* Model::create(const wchar_t* filename)
		{
			return ModelPrivate::create(filename);
		}
	}
}
