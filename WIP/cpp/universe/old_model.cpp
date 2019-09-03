namespace flame
{
	Vec2 from(const aiVector2D &v)
	{
		Vec2 ret;
		ret.x = v.x;
		ret.y = v.y;
		return ret;
	}

	Vec3 from(const aiVector3D &v)
	{
		Vec3 ret;
		ret.x = v.x;
		ret.y = v.y;
		ret.z = v.z;
		return ret;
	}

	struct ModelMap
	{
		std::string filename;
	};

	struct ModelMaterial
	{
		ModelMap maps[MapSemanticsCount];
	};

	struct VertexSemanticsAttribute
	{
		bool need;
		bool has;
		int buffer_index;
		int offset;

		VertexSemanticsAttribute()
		{
			need = false;
			has = false;
			buffer_index = -1;
			offset = -1;
		}
	};

	static void init(Model *m)
	{
		m->bone_count_exceeded = false;
		m->bone_count_per_vertex_exceeded = false;

		m->vertex_count = 0;
		m->indice_count = 0;

		m->mesh_count = 0;
		m->meshes = nullptr;

		m->bone_count = 0;
		m->bones = nullptr;

		m->animation_count = 0;
		m->animations = nullptr;
	}

	static void create_vertex_buffer(Model *m, ModelDescription *desc, VertexSemanticsAttribute *vtx_sem_attr)
	{
		if (desc->desired_vertex_buffer_count == 0)
		{
			// by default, we accept all vertex semantics and put them all in one buffer

			m->vertex_buffer_count = 1;
			m->vertex_buffers = new ModelVertexBuffer[1];

			m->vertex_buffers[0].semantic_count = 0;
			for (auto i = 0; i < VertexSemanticsCount; i++)
			{
				if (vtx_sem_attr[i].has)
					m->vertex_buffers[0].semantic_count++;
			}

			m->vertex_buffers[0].semantics = new VertexSemantic[m->vertex_buffers[0].semantic_count];

			for (auto i = 0, j = 0; i < VertexSemanticsCount; i++)
			{
				if (vtx_sem_attr[i].has)
				{
					m->vertex_buffers[0].semantics[j] = VertexSemantic(i);
					j++;
				}
			}
		}
		else
		{
			m->vertex_buffer_count = 0;

			for (auto i = 0; i < desc->desired_vertex_buffer_count; i++)
			{
				if (!desc->desired_vertex_buffers[i].active_if_has_bone || m->bone_count > 0)
					m->vertex_buffer_count++;
			}

			m->vertex_buffers = new ModelVertexBuffer[m->vertex_buffer_count];

			for (auto i = 0, j = 0; i < desc->desired_vertex_buffer_count; i++)
			{
				if (desc->desired_vertex_buffers[i].active_if_has_bone && m->bone_count == 0)
					continue;

				m->vertex_buffers[j].semantic_count = desc->desired_vertex_buffers[i].semantic_count;
				m->vertex_buffers[j].semantics = new VertexSemantic[m->vertex_buffers[j].semantic_count];

				for (auto k = 0; k < desc->desired_vertex_buffers[i].semantic_count; k++)
					m->vertex_buffers[j].semantics[k] = desc->desired_vertex_buffers[i].semantics[k];
			}
		}

		for (auto i = 0; i < m->vertex_buffer_count; i++)
		{
			auto vb = &m->vertex_buffers[i];

			vb->size = 0;
			for (auto j = 0; j < vb->semantic_count; j++)
			{
				auto s = vb->semantics[j];

				if (vtx_sem_attr[s].need)
					assert(0); // vertex semantics cannot repeat
				vtx_sem_attr[s].need = true;
				vtx_sem_attr[s].buffer_index = i;
				vtx_sem_attr[s].offset = vb->size;

				vb->size += vertex_semantic_size(s);
			}
		}
	}

	static void add_cube_vertexs(std::vector<Vec3> &positions, std::vector<Vec3> &normals, std::vector<int> &indices, Mat3 rotation, Vec3 center, float hf_ext)
	{
		int base_vertex = positions.size();

		auto a = center + rotation * (Vec3(1.f, -1.f, 1.f) * hf_ext);
		auto b = center + rotation * (Vec3(1.f, -1.f, -1.f) * hf_ext);
		auto c = center + rotation * (Vec3(1.f, 1.f, -1.f) * hf_ext);
		auto d = center + rotation * (Vec3(1.f, 1.f, 1.f) * hf_ext);
		auto e = center + rotation * (Vec3(-1.f, -1.f, 1.f) * hf_ext);
		auto f = center + rotation * (Vec3(-1.f, -1.f, -1.f) * hf_ext);
		auto g = center + rotation * (Vec3(-1.f, 1.f, -1.f) * hf_ext);
		auto h = center + rotation * (Vec3(-1.f, 1.f, 1.f) * hf_ext);

		auto n0 = rotation * Vec3(1.f, 0.f, 0.f);
		auto n1 = rotation * Vec3(-1.f, 0.f, 0.f);
		auto n2 = rotation * Vec3(0.f, 1.f, 0.f);
		auto n3 = rotation * Vec3(0.f, -1.f, 0.f);
		auto n4 = rotation * Vec3(0.f, 0.f, 1.f);
		auto n5 = rotation * Vec3(0.f, 0.f, -1.f);

		positions.push_back(a); normals.push_back(n0);
		positions.push_back(b); normals.push_back(n0);
		positions.push_back(c); normals.push_back(n0);
		positions.push_back(d); normals.push_back(n0);
		positions.push_back(e); normals.push_back(n1);
		positions.push_back(f); normals.push_back(n1);
		positions.push_back(g); normals.push_back(n1);
		positions.push_back(h); normals.push_back(n1);
		positions.push_back(c); normals.push_back(n2);
		positions.push_back(d); normals.push_back(n2);
		positions.push_back(g); normals.push_back(n2);
		positions.push_back(h); normals.push_back(n2);
		positions.push_back(a); normals.push_back(n3);
		positions.push_back(b); normals.push_back(n3);
		positions.push_back(e); normals.push_back(n3);
		positions.push_back(f); normals.push_back(n3);
		positions.push_back(a); normals.push_back(n4);
		positions.push_back(d); normals.push_back(n4);
		positions.push_back(e); normals.push_back(n4);
		positions.push_back(h); normals.push_back(n4);
		positions.push_back(b); normals.push_back(n5);
		positions.push_back(c); normals.push_back(n5);
		positions.push_back(f); normals.push_back(n5);
		positions.push_back(g); normals.push_back(n5);

		std::vector<int> list = {
			3, 0, 1, 3, 1, 2,
			6, 5, 4, 6, 4, 7,
			11, 9, 8, 11, 8, 10,
			12, 14, 15, 12, 15, 13,
			19, 18, 16, 19, 16, 17,
			21, 20, 22, 21, 22, 23
		};

		for (auto &i : list)
			i += base_vertex;

		indices.insert(indices.end(), list.begin(), list.end());
	}

	static void process_node(Model *m, ModelNode *p, aiNode *n)
	{
		auto c = new ModelNode;
		c->name = n->mName.data;
		memcpy(&c->local_matrix[0][0], &n->mTransformation.a1, sizeof(Mat4));
		c->global_matrix = Mat4(1.f);

		if (!p)
		{
			m->root_node = c;
			c->parent = nullptr;
		}
		else
		{
			c->parent = p;
			if (p->first_child)
				p->last_child->next_sibling = c;
			else
				p->first_child = p->last_child = c;
			p->last_child = c;
			p->children_count++;
		}

		c->next_sibling = nullptr;
		c->children_count = 0;
		c->first_child = c->last_child = nullptr;

		for (auto i = 0; i < m->mesh_count; i++)
		{
			if (strcmp(m->meshes[i]->name, c->name) == 0)
			{
				c->type = ModelNodeMesh;
				c->p = m->meshes[i];
				m->meshes[i]->pNode = c;

				goto do_recursion;
			}
		}

		for (auto i = 0; i < m->bone_count; i++)
		{
			if (strcmp(m->bones[i]->name, c->name) == 0)
			{
				c->type = ModelNodeBone;
				c->p = m->bones[i];
				m->bones[i]->pNode = c;

				if (!m->root_bone)
					m->root_bone = c;

				goto do_recursion;
			}
		}

		c->type = ModelNodeNode;
		c->p = nullptr;

		do_recursion:

		for (auto i = 0; i < n->mNumChildren; i++)
			process_node(m, c, n->mChildren[i]);
	};

	static void destroy_node(ModelNode *n)
	{
		auto c = n->first_child;
		while (c)
		{
			destroy_node(c);
			c = c->next_sibling;
		}
		delete n;
	}

	static void initialize_sem(VertexSemanticsAttribute *vtx_sem_attr, VertexSemantic sem)
	{
		auto s = &mvtx_sem_attr[sem];
		if (s->need)
		{
			auto d = &m->vertex_buffers[s->buffer_index];
			auto p = (float*)d->pVertex + s->offset;
			for (auto i = 0; i < m->vertex_count; i++)
			{
				p[0] = 0.f;
				p[1] = 0.f;
				p[2] = 0.f;
				p[3] = 0.f;
				p += d->size;
			}
		}
	}

	Model *load_model(ModelDescription *desc, const char *filename)
	{
		std::filesystem::path path(filename);
		if (!std::filesystem::exists(path))
			return nullptr;

		Assimp::Importer importer;
		auto scene = importer.ReadFile(filename, aiProcess_Triangulate);

		auto m = new Model;
		init(m);

		VertexSemanticsAttribute vtx_sem_attr[VertexSemanticsCount];

		m->mesh_count = scene->mNumMeshes;
		m->meshes = new ModelMesh*[m->mesh_count];

		for (auto i = 0; i < scene->mNumMeshes; i++)
		{
			auto mesh = scene->mMeshes[i];

			m->meshes[i] = new ModelMesh;
			m->meshes[i]->pNode = nullptr;
			strncpy(m->meshes[i]->name, mesh->mName.data, FLAME_MODEL_NAME_LENGTH);

			m->meshes[i]->material_index = 0;

			//auto mid = mesh->mMaterialIndex;
			//auto mtl = scene->mMaterials[mid];
			//if (mtl->GetTextureCount(aiTextureType_DIFFUSE) > 0)
			//{
			//	aiString ret_name;
			//	mtl->GetTexture(aiTextureType_DIFFUSE, 0, &ret_name);
			//}

			if (mesh->mVertices)
				vtx_sem_attr[VertexPosition].has = true;
			auto num_uv_channel = mesh->GetNumUVChannels();
			for (auto j = 0; j < num_uv_channel; j++)
				vtx_sem_attr[VertexUV0 + j].has = true;
			if (mesh->mNormals)
				vtx_sem_attr[VertexNormal].has = true;
			if (mesh->mTangents)
				vtx_sem_attr[VertexTangent].has = true;
			if (mesh->mBitangents)
				vtx_sem_attr[VertexBitangent].has = true;
			auto num_color_channel = mesh->GetNumColorChannels();
			assert(num_color_channel <= 1);
			if (num_color_channel == 1)
				vtx_sem_attr[VertexColor].has = true;

			m->bone_count += mesh->mNumBones;

			auto ic = mesh->mNumFaces * 3;

			m->meshes[i]->indice_base = m->indice_count;
			m->meshes[i]->indice_count = ic;

			m->vertex_count += mesh->mNumVertices;
			m->indice_count += ic;
		}

		if (m->bone_count > 0)
		{
			vtx_sem_attr[VertexBoneID].has = true;
			vtx_sem_attr[VertexBoneWeight].has = true;
		}

		create_vertex_buffer(m, desc, vtx_sem_attr);

		for (auto i = 0; i < m->vertex_buffer_count; i++)
			m->vertex_buffers[i].pVertex = (unsigned char*)malloc(m->vertex_count * m->vertex_buffers[i].size * sizeof(float));
		m->pIndices = (unsigned char*)malloc(m->indice_count * (desc->indice_type == IndiceUint ? sizeof(uint) : sizeof(ushort)));

		std::vector<int> vertex_bone_count;
		if (m->bone_count > 0)
		{
			// do initialize the bone ids and bone weights, since we don't know if there are some vertexs not covered by bones
			initialize_sem(vtx_sem_attr, VertexBoneID);
			initialize_sem(vtx_sem_attr, VertexBoneWeight);

			vertex_bone_count.resize(m->vertex_count);

			m->bones = new ModelBone*[m->bone_count];
			for (auto i = 0; i < m->bone_count; i++)
			{
				m->bones[i] = new ModelBone;
				m->bones[i]->pNode = nullptr;
			}
		}

		auto v_base = 0;
		auto i_base = 0;
		auto b_id = 0;

		for (auto i = 0; i < scene->mNumMeshes; i++)
		{
			auto mesh = scene->mMeshes[i];

			for (auto isem = 0; isem < VertexSemanticsCount; isem++)
			{
				auto s = &vtx_sem_attr[isem];
				if (s->need)
				{
					auto d = &m->vertex_buffers[s->buffer_index];
					auto p = (float*)d->pVertex + v_base * d->size + s->offset;
					switch (VertexSemantic(isem))
					{
						case VertexPosition:
							if (s->has)
							{
								for (auto j = 0; j < mesh->mNumVertices; j++)
								{
									auto pos = mesh->mVertices[j];
									p[0] = pos.x;
									p[1] = pos.y;
									p[2] = pos.z;
									p += d->size;
								}
							}
							else
							{
								for (auto j = 0; j < mesh->mNumVertices; j++)
								{
									p[0] = 0.f;
									p[1] = 0.f;
									p[2] = 0.f;
									p += d->size;
								}
							}
							break;
						case VertexUV0: case VertexUV1: case VertexUV2: case VertexUV3:
						case VertexUV4: case VertexUV5: case VertexUV6: case VertexUV7:
							if (s->has)
							{
								auto tcs = mesh->mTextureCoords[isem - VertexUV0];
								for (auto j = 0; j < mesh->mNumVertices; j++)
								{
									auto tc = tcs[j];
									p[0] = tc.x;
									p[1] = tc.y;
									p += d->size;
								}
							}
							else
							{
								for (auto j = 0; j < mesh->mNumVertices; j++)
								{
									p[0] = 0.f;
									p[1] = 0.f;
									p += d->size;
								}
							}
							break;
						case VertexNormal:
							if (s->has)
							{
								for (auto j = 0; j < mesh->mNumVertices; j++)
								{
									auto normal = mesh->mNormals[j];
									p[0] = normal.x;
									p[1] = normal.y;
									p[2] = normal.z;
									p += d->size;
								}
							}
							else
							{
								for (auto j = 0; j < mesh->mNumVertices; j++)
								{
									p[0] = 0.f;
									p[1] = 1.f;
									p[2] = 0.f;
									p += d->size;
								}
							}
							break;
						case VertexTangent:
							if (s->has)
							{
								for (auto j = 0; j < mesh->mNumVertices; j++)
								{
									auto tangent = mesh->mTangents[j];
									p[0] = tangent.x;
									p[1] = tangent.y;
									p[2] = tangent.z;
									p += d->size;
								}
							}
							else
							{
								for (auto j = 0; j < mesh->mNumVertices; j++)
								{
									p[0] = 1.f;
									p[1] = 0.f;
									p[2] = 0.f;
									p += d->size;
								}
							}
							break;
						case VertexBitangent:
							if (s->has)
							{
								for (auto j = 0; j < mesh->mNumVertices; j++)
								{
									auto bitangent = mesh->mBitangents[j];
									p[0] = bitangent.x;
									p[1] = bitangent.y;
									p[2] = bitangent.z;
									p += d->size;
								}
							}
							else
							{
								for (auto j = 0; j < mesh->mNumVertices; j++)
								{
									p[0] = 0.f;
									p[1] = 0.f;
									p[2] = 1.f;
									p += d->size;
								}
							}
							break;
						case VertexColor:
							if (s->has)
							{
								auto cols = mesh->mColors[0];
								for (auto j = 0; j < mesh->mNumVertices; j++)
								{
									auto col = cols[j];
									p[0] = col.r;
									p[1] = col.g;
									p[2] = col.b;
									p[3] = col.a;
									p += d->size;
								}
							}
							else
							{
								for (auto j = 0; j < mesh->mNumVertices; j++)
								{
									p[0] = 1.f;
									p[1] = 0.f;
									p[2] = 1.f;
									p[3] = 1.f;
									p += d->size;
								}
							}
							break;
					}
				}
			}

			if (m->bone_count > 0)
			{
				float *pBoneID = nullptr;
				int strid1;
				float *pBoneWeight = nullptr;
				int strid2;
				{
					auto s = &vtx_sem_attr[VertexBoneID];
					if (s->need)
					{
						auto d = &m->vertex_buffers[s->buffer_index];
						strid1 = d->size;
						pBoneID = (float*)d->pVertex + v_base * strid1 + s->offset;
					}
				}
				{
					auto s = &vtx_sem_attr[VertexBoneWeight];
					if (s->need)
					{
						auto d = &m->vertex_buffers[s->buffer_index];
						strid2 = d->size;
						pBoneWeight = (float*)d->pVertex + v_base * strid2 + s->offset;
					}
				}
				for (auto j = 0; j < mesh->mNumBones; j++)
				{
					auto b = mesh->mBones[j];
					for (auto k = 0; k < b->mNumWeights; k++)
					{
						auto vid = b->mWeights[k].mVertexId;
						auto &bc = vertex_bone_count[v_base + vid];
						if (bc < 4)
						{
							if (pBoneID)
								pBoneID[vid * strid1 + bc] = b_id;
							if (pBoneWeight)
								pBoneWeight[vid * strid2 + bc] = b->mWeights[k].mWeight;
							bc++;
						}
						else
							m->bone_count_per_vertex_exceeded = true;
					}

					strncpy(m->bones[b_id]->name, b->mName.data, FLAME_MODEL_NAME_LENGTH);
					memcpy(&m->bones[b_id]->offset_matrix[0][0], &b->mOffsetMatrix, sizeof(Mat4));
					m->bones[b_id]->offset_matrix.transpose();
					m->bones[b_id]->id = b_id;

					b_id++;
				}
			}

			v_base += mesh->mNumVertices;

			if (desc->indice_type == IndiceUint)
			{
				auto p = (uint*)m->pIndices + i_base;
				for (auto j = 0; j < mesh->mNumFaces; j++)
				{
					for (auto k = 0; k < 3; k++)
					{
						p[0] = i_base + mesh->mFaces[j].mIndices[k];
						p++;
					}
				}
			}
			else
			{
				auto p = (ushort*)m->pIndices + i_base;
				for (auto j = 0; j < mesh->mNumFaces; j++)
				{
					for (auto k = 0; k < 3; k++)
					{
						p[0] = i_base + mesh->mFaces[j].mIndices[k];
						p++;
					}
				}
			}

			i_base += mesh->mNumFaces * 3;
		}

		m->root_bone = nullptr;

		process_node(m, nullptr, scene->mRootNode);

		if (scene->mNumAnimations > 0)
		{
			m->animation_count = scene->mNumAnimations;
			m->animations = new ModelAnimation*[m->animation_count];

			for (auto i = 0; i < m->animation_count; i++)
			{
				auto src = scene->mAnimations[i];
				auto dst = new ModelAnimation;
				m->animations[i] = dst;

				strncpy(dst->name, src->mName.data, FLAME_MODEL_NAME_LENGTH);

				dst->total_ticks = src->mDuration;
				dst->ticks_per_second = src->mTicksPerSecond;

				dst->motion_count = 0;
				dst->motions = nullptr;
				if (src->mNumChannels > 0)
				{
					dst->motion_count = src->mNumChannels;
					dst->motions = new ModelMotion[dst->motion_count];

					for (auto j = 0; j < dst->motion_count; j++)
					{
						auto ch = src->mChannels[j];
						auto mo = &dst->motions[j];

						strncpy(mo->name, ch->mNodeName.data, FLAME_MODEL_NAME_LENGTH);

						mo->position_key_count = 0;
						mo->position_keys = nullptr;
						if (ch->mNumPositionKeys > 0)
						{
							mo->position_key_count = ch->mNumPositionKeys;
							mo->position_keys = new ModelPositionKey[mo->position_key_count];

							for (auto k = 0; k < mo->position_key_count; k++)
							{
								auto src_k = &ch->mPositionKeys[k];
								auto dst_k = &mo->position_keys[k];

								dst_k->time = src_k->mTime;
								dst_k->value.x = src_k->mValue.x;
								dst_k->value.y = src_k->mValue.y;
								dst_k->value.z = src_k->mValue.z;
							}
						}

						mo->rotation_key_count = 0;
						mo->rotation_keys = nullptr;
						if (ch->mNumRotationKeys > 0)
						{
							mo->rotation_key_count = ch->mNumRotationKeys;
							mo->rotation_keys = new ModelRotationKey[mo->rotation_key_count];

							for (auto k = 0; k < mo->rotation_key_count; k++)
							{
								auto src_k = &ch->mRotationKeys[k];
								auto dst_k = &mo->rotation_keys[k];

								dst_k->time = src_k->mTime;
								dst_k->value.x = src_k->mValue.x;
								dst_k->value.y = src_k->mValue.y;
								dst_k->value.z = src_k->mValue.z;
								dst_k->value.w = src_k->mValue.w;
							}
						}
					}
				}
			}
		}

		m->aabb.reset();
		if (desc->need_AABB)
		{
			for (auto i = 0; i < scene->mNumMeshes; i++)
			{
				auto mesh = scene->mMeshes[i];
				for (auto j = 0; j < mesh->mNumVertices; j++)
				{
					auto pos = from(mesh->mVertices[j]);
					m->aabb.merge(pos);
				}
			}
		}

		return m;
	}

	Model *create_cube_model(ModelDescription *desc, float hf_ext)
	{
		auto m = new Model;
		init(m);

		VertexSemanticsAttribute vtx_sem_attr[VertexSemanticsCount];

		vtx_sem_attr[VertexPosition].has = true;
		vtx_sem_attr[VertexNormal].has = true;

		create_vertex_buffer(m, desc, vtx_sem_attr);

		std::vector<Vec3> positions;
		std::vector<Vec3> normals;
		std::vector<int> indices;
		add_cube_vertexs(positions, normals, indices, Mat3(1.f), Vec3(0.f), 0.5f);

		m->vertex_count = positions.size();
		m->indice_count = indices.size();
		for (auto i = 0; i < m->vertex_buffer_count; i++)
			m->vertex_buffers[i].pVertex = (unsigned char*)malloc(m->vertex_count * m->vertex_buffers[i].size * sizeof(float));
		m->pIndices = (unsigned char*)malloc(m->indice_count * (desc->indice_type == IndiceUint ? sizeof(uint) : sizeof(ushort)));

		m->mesh_count = 1;
		m->meshes = new ModelMesh*[1];
		m->meshes[0] = new ModelMesh;
		m->meshes[0]->pNode = nullptr;
		m->meshes[0]->indice_base = 0;
		m->meshes[0]->indice_count = m->indice_count;
		strcpy(m->meshes[0]->name, "mesh");

		m->meshes[0]->material_index = 0;

		for (auto isem = 0; isem < VertexSemanticsCount; isem++)
		{
			auto s = &vtx_sem_attr[isem];
			if (s->need)
			{
				auto d = &m->vertex_buffers[s->buffer_index];
				auto p = (float*)d->pVertex + s->offset;
				switch (VertexSemantic(isem))
				{
					case VertexPosition:
						if (s->has)
						{
							for (auto j = 0; j < positions.size(); j++)
							{
								p[0] = positions[j].x;
								p[1] = positions[j].y;
								p[2] = positions[j].z;
								p += d->size;
							}
						}
						else
						{
							for (auto j = 0; j < positions.size(); j++)
							{
								p[0] = 0.f;
								p[1] = 0.f;
								p[2] = 0.f;
								p += d->size;
							}
						}
						break;
					case VertexUV0: case VertexUV1: case VertexUV2: case VertexUV3:
					case VertexUV4: case VertexUV5: case VertexUV6: case VertexUV7:
						for (auto j = 0; j < positions.size(); j++)
						{
							p[0] = 0.f;
							p[1] = 0.f;
							p += d->size;
						}
						break;
					case VertexNormal:
						if (s->has)
						{
							for (auto j = 0; j < positions.size(); j++)
							{
								p[0] = normals[j].x;
								p[1] = normals[j].y;
								p[2] = normals[j].z;
								p += d->size;
							}
						}
						else
						{
							for (auto j = 0; j < positions.size(); j++)
							{
								p[0] = 0.f;
								p[1] = 1.f;
								p[2] = 0.f;
								p += d->size;
							}
						}
						break;
					case VertexTangent:
						for (auto j = 0; j < positions.size(); j++)
						{
							p[0] = 1.f;
							p[1] = 0.f;
							p[2] = 0.f;
							p += d->size;
						}
						break;
					case VertexBitangent:
						for (auto j = 0; j < positions.size(); j++)
						{
							p[0] = 0.f;
							p[1] = 0.f;
							p[2] = 1.f;
							p += d->size;
						}
						break;
					case VertexColor:
						for (auto j = 0; j < positions.size(); j++)
						{
							p[0] = 1.f;
							p[1] = 0.f;
							p[2] = 1.f;
							p[3] = 1.f;
							p += d->size;
						}
						break;
				}
			}
		}

		if (desc->indice_type == IndiceUint)
		{
			auto p = (uint*)m->pIndices;
			for (auto j = 0; j < indices.size(); j++)
			{
				p[0] = indices[j];
				p++;
			}
		}
		else
		{
			auto p = (ushort*)m->pIndices;
			for (auto j = 0; j < indices.size(); j++)
			{
				p[0] = indices[j];
				p++;
			}
		}

		m->aabb.reset();
		if (desc->need_AABB)
		{
			for (auto &pos : positions)
				m->aabb.merge(pos);
		}

		m->root_node = new ModelNode;
		m->root_node->parent = nullptr;
		m->root_node->next_sibling = nullptr;
		m->root_node->children_count = 0;
		m->root_node->first_child = nullptr;
		m->root_node->last_child = nullptr;
		m->root_node->local_matrix = Mat4(1.f);
		m->root_node->global_matrix = Mat4(1.f);

		return m;
	}

	void save_model(Model *m, const char *filename)
	{

	}

	void destroy_model(Model *m)
	{
		for (auto i = 0; i < m->vertex_buffer_count; i++)
		{
			delete[]m->vertex_buffers[i].semantics;
			delete[]m->vertex_buffers[i].pVertex;
		}
		delete[]m->vertex_buffers;
		delete[]m->pIndices;

		for (auto i = 0; i < m->mesh_count; i++)
			delete m->meshes[i];
		delete[]m->meshes;

		for (auto i = 0; i < m->bone_count; i++)
			delete m->bones[i];
		delete[]m->bones;

		for (auto i = 0; i < m->animation_count; i++)
		{
			for (auto j = 0; j < m->animations[i]->motion_count; j++)
			{
				delete[]m->animations[i]->motions[j].position_keys;
				delete[]m->animations[i]->motions[j].rotation_keys;
			}
			delete m->animations[i]->motions;

			delete m->animations[i];
		}
		delete[]m->animations;

		destroy_node(m->root_node);

		delete m;
	}
}

