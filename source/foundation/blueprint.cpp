#include <flame/serialize.h>
#include "blueprint_private.h"
#include <flame/foundation/typeinfo.h>

namespace flame
{
	bpSlotPrivate::bpSlotPrivate(bpNode* _node, bpSlotIO _io, uint _index, TypeInfo* _type, const std::string& _name, uint _offset, uint _size, const void* _default_value) :
		setter(nullptr),
		listener(nullptr)
	{
		node = _node;
		io = _io;
		index = _index;
		type = _type;
		name = _name;
		offset = _offset;
		size = _size;
		user_data = nullptr;

		if (_default_value)
		{
			default_value = new char[size];
			memcpy(default_value, _default_value, size);
		}
		else
			default_value = nullptr;

		data = (char*)((bpNodePrivate*)node)->object + offset;

		if (io == bpSlotIn)
			links.push_back(nullptr);
	}

	bpSlotPrivate::bpSlotPrivate(bpNode* node, bpSlotIO io, uint index, VariableInfo* vi) :
		bpSlotPrivate(node, io, index, vi->type, vi->name.str(),
			vi->offset, vi->size, vi->default_value)
	{
	}

	bpSlotPrivate::~bpSlotPrivate()
	{
		delete[] default_value;
		delete setter;
	}

	void bpSlotPrivate::set_data(const void* d)
	{
		if (!setter)
			type->copy_from(d, data, size);
		else
			setter->set(d);
	}

	bool bpSlotPrivate::link_to(bpSlotPrivate* target)
	{
		assert(io == bpSlotIn);
		if (io == bpSlotIn)
			return false;

		if (links[0] == target)
			return true;

		if (target)
		{
			if (target->io == bpSlotIn)
				return false;
			if (node && node == target->node)
				return false;
		}

		if (target)
		{
			if (!can_link(type, target->type))
				return false;
		}

		if (links[0])
		{
			auto o = links[0];
			for (auto i = 0; i < o->links.s; i++)
			{
				if (o->links[i] == this)
				{
					o->links.remove(i);
					break;
				}
			}
			if (type->base_name == "ListenerHub")
				(*(ListenerHub<void(Capture&)>**)data)->remove(listener);
		}

		links[0] = target;
		if (target)
		{
			target->links.push_back(this);
			if (type->base_name == "ListenerHub")
			{
				auto p = target->data;
				memcpy(data, &p, sizeof(void*));
				listener = (*(ListenerHub<void(Capture&)>**)data)->add([](Capture& c) {
					c.thiz<bpNode>()->update();
				}, Capture().set_thiz(node));
			}
		}

		if (!target && type->tag == TypePointer)
			memset(data, 0, sizeof(void*));

		return true;
	}

	bpNodePrivate::bpNodePrivate(bpNodeType _node_type, bpNodePrivate* _parent, const std::string& _id, const std::string& _type) :
		object(nullptr),
		library(nullptr),
		dtor_addr(nullptr),
		update_addr(nullptr),
		order(0xffffffff),
		need_rebuild_update_list(true)
	{
		id = _id;
		pos = Vec2f(0.f);
		user_data = nullptr;
		parent = _parent;
		type = _type;
		udt = nullptr;

		std::string parameters;
		auto t = break_bp_node_type(_type, &parameters);
		if (t != 0)
		{
			assert(_node_type == bpNodeReal);
			node_type = bpNodeReal;

			switch (t)
			{
			case 'S':
			{
#pragma pack(1)
				struct Dummy
				{
					int in;
					int chk;

					int out;
					float res;

					void update()
					{
						out = in;
						res = in == chk ? 1.f : 0.f;
					}
				};
#pragma pack()
				node_type = bpNodeReal;
				auto size = udt->size;
				object = malloc(size);
				memset(object, 0, size);
				inputs.push_back(new bpSlotPrivate(this, bpSlotIn, 0, (TypeInfo*)TypeInfo::get(TypeEnumSingle, parameters.c_str()), "in", offsetof(Dummy, in), sizeof(Dummy::in), nullptr));
				inputs.push_back(new bpSlotPrivate(this, bpSlotIn, 1, (TypeInfo*)TypeInfo::get(TypeEnumSingle, parameters.c_str()), "chk", offsetof(Dummy, chk), sizeof(Dummy::chk), nullptr));
				outputs.push_back(new bpSlotPrivate(this, bpSlotOut, 0, (TypeInfo*)TypeInfo::get(TypeEnumSingle, parameters.c_str()), "out", offsetof(Dummy, out), sizeof(Dummy::out), nullptr));
				outputs.push_back(new bpSlotPrivate(this, bpSlotOut, 1, (TypeInfo*)TypeInfo::get(TypeData, "float"), "res", offsetof(Dummy, res), sizeof(Dummy::res), nullptr));
				update_addr = f2v(&Dummy::update);
			}
				break;
			case 'M':
			{
#pragma pack(1)
				struct Dummy
				{
					int in;
					int chk;

					int out;
					float res;

					void update()
					{
						out = in;
						res = (in & chk) ? 1.f : 0.f;
					}
				};
#pragma pack()
				node_type = bpNodeReal;
				auto size = sizeof(Dummy);
				object = malloc(size);
				memset(object, 0, size);
				inputs.push_back(new bpSlotPrivate(this, bpSlotIn, 0, (TypeInfo*)TypeInfo::get(TypeEnumMulti, parameters.c_str()), "in", offsetof(Dummy, in), sizeof(Dummy::in), nullptr));
				inputs.push_back(new bpSlotPrivate(this, bpSlotIn, 1, (TypeInfo*)TypeInfo::get(TypeEnumSingle, parameters.c_str()), "chk", offsetof(Dummy, chk), sizeof(Dummy::chk), nullptr));
				outputs.push_back(new bpSlotPrivate(this, bpSlotOut, 0, (TypeInfo*)TypeInfo::get(TypeEnumMulti, parameters.c_str()), "out", offsetof(Dummy, out), sizeof(Dummy::out), nullptr));
				outputs.push_back(new bpSlotPrivate(this, bpSlotOut, 1, (TypeInfo*)TypeInfo::get(TypeData, "float"), "res", offsetof(Dummy, res), sizeof(Dummy::res), nullptr));
				update_addr = f2v(&Dummy::update);
			}
				break;
			case 'V':
			{
#pragma pack(1)
				struct Dummy
				{
					uint type_hash;
					uint type_size;

					void dtor()
					{
						auto in = (char*)&type_size + sizeof(uint);
						auto out = (char*)&type_size + sizeof(uint) + type_size;
						basic_type_dtor(type_hash, in);
						basic_type_dtor(type_hash, out);
					}

					void update()
					{
						auto in = (char*)&type_size + sizeof(uint);
						auto out = (char*)&type_size + sizeof(uint) + type_size;
						basic_type_copy(type_hash, in, out, type_size);
					}
				};
#pragma pack()
				auto type_hash = FLAME_HASH(parameters.c_str());
				auto type_size = basic_type_size(type_hash);

				node_type = bpNodeReal;
				auto size = sizeof(Dummy) + type_size * 2;
				object = malloc(size);
				memset(object, 0, size);
				inputs.push_back(new bpSlotPrivate(this, bpSlotIn, 0, (TypeInfo*)TypeInfo::get(TypeData, parameters.c_str()), "in", sizeof(Dummy), type_size, nullptr));
				outputs.push_back(new bpSlotPrivate(this, bpSlotOut, 0, (TypeInfo*)TypeInfo::get(TypeData, parameters.c_str()), "out", sizeof(Dummy) + type_size, type_size, nullptr));
				dtor_addr = f2v(&Dummy::dtor);
				update_addr = f2v(&Dummy::update);
				auto& obj = *(Dummy*)object;
				obj.type_hash = type_hash;
				obj.type_size = type_size;
			}
				break;
			case 'A':
			{
				auto sp = SUS::split(parameters, '+');
#pragma pack(1)
				struct Dummy
				{
					uint type_hash;
					uint type_size;
					uint length;

					void dtor()
					{
						for (auto i = 0; i < length; i++)
							basic_type_dtor(type_hash, (char*)&length + sizeof(uint) + type_size * i);
						auto& out = *(Array<int>*)((char*)&length + sizeof(uint) + type_size * length);
						for (auto i = 0; i < out.s; i++)
							basic_type_dtor(type_hash, (char*)out.v + type_size * i);
						f_free(out.v);
					}

					void update()
					{
						auto& out = *(Array<int>*)((char*)&length + sizeof(uint) + type_size * length);
						if (out.s != length)
						{
							out.s = length;
							auto m_size = type_size * length;
							f_free(out.v);
							out.v = (int*)f_malloc(m_size);
							memset(out.v, 0, m_size);
						}
						for (auto i = 0; i < length; i++)
						{
							auto v = (char*)&length + sizeof(uint) + type_size * i;
							basic_type_copy(type_hash, v, (char*)out.v + type_size * i, type_size);
						}
					}
				};
#pragma pack()
				auto tag = TypeData;
				auto type_name = sp[1];
				auto base_name = type_name;
				if (type_name.back() == '*')
				{
					base_name.erase(base_name.end() - 1);
					tag = TypePointer;
				}
				auto type_hash = FLAME_HASH(base_name.c_str());
				uint type_size = tag == TypeData ? basic_type_size(type_hash) : sizeof(void*);
				auto length = stoi(sp[0]);
				auto size = sizeof(Dummy) + type_size * length + sizeof(Array<int>);
				object = malloc(size);
				memset(object, 0, size);
				for (auto i = 0; i < length; i++)
					inputs.push_back(new bpSlotPrivate(this, bpSlotIn, i, (TypeInfo*)TypeInfo::get(tag, base_name.c_str()), std::to_string(i), sizeof(Dummy) + type_size * i, type_size, nullptr));
				outputs.push_back(new bpSlotPrivate(this, bpSlotOut, 0, (TypeInfo*)TypeInfo::get(TypeData, type_name.c_str(), true), "out", sizeof(Dummy) + type_size * length, sizeof(Array<int>), nullptr));
				dtor_addr = f2v(&Dummy::dtor);
				update_addr = f2v(&Dummy::update);
				auto& obj = *(Dummy*)object;
				obj.type_hash = type_hash;
				obj.type_size = type_size;
				obj.length = length;
			}
				break;
			case 'G':
			{
#pragma pack(1)
				struct Dummy
				{
					ListenerHub<void()>* signal;
				};
#pragma pack()
				node_type = bpNodeReal;
				auto size = sizeof(Dummy);
				object = malloc(size);
				memset(object, 0, size);
				inputs.push_back(new bpSlotPrivate(this, bpSlotIn, 0, (TypeInfo*)TypeInfo::get(TypePointer, "ListenerHub"), "signal", offsetof(Dummy, signal), sizeof(Dummy::signal), nullptr));
			}
				break;
			default:
				assert(0);
				printf("wrong type in add node: %s\n", _type.c_str());
			}
		}
		else
		{
			node_type = _node_type;

			udt = find_udt(FLAME_HASH(_type.c_str()));

			if (!udt)
			{
				assert(0);
				printf("udt not found in add node: %s\n", _type.c_str());
			}

			library = udt->db->library;

			if (node_type == bpNodeReal)
			{
				auto size = udt->size;
				object = malloc(size);
				memset(object, 0, size);

				{
					auto f = udt->find_function("ctor");
					if (f && f->parameters.s == 0)
						cmf(p2f<MF_v_v>((char*)library + (uint)f->rva), object);
				}

				{
					auto f = udt->find_function("dtor");
					if (f)
						dtor_addr = (char*)library + (uint)f->rva;
				}

				{
					auto f = udt->find_function("bp_update");
					assert(f && check_function(f, "D#void", {}));
					update_addr = (char*)library + (uint)f->rva;
				}

				for (auto v : udt->variables)
				{
					if (v->flags & VariableFlagOutput)
						outputs.push_back(new bpSlotPrivate(this, bpSlotOut, outputs.s, v));
					else
						inputs.push_back(new bpSlotPrivate(this, bpSlotIn, inputs.s, v));
				}
			}
			else
			{
				// TODO
				//{
				//	auto f = udt->find_function("get_linked_object");
				//	assert(f && check_function(f, ("P#" + udt->name.str()).c_str(), {}));
				//	object = cf(p2f<F_vp_v>((char*)library + (uint)f->rva));
				//	assert(object);
				//}

				if (node_type == bpNodeRefRead)
				{
					for (auto v : udt->variables)
						outputs.push_back(new bpSlotPrivate(this, bpSlotOut, outputs.s, v));
				}
				else if (node_type == bpNodeRefWrite)
				{
					for (auto v : udt->variables)
					{
						auto type = v->type;
						if (type->tag != TypeData)
							continue;
						auto base_hash = type->base_hash;
						auto input = new bpSlotPrivate(this, bpSlotIn, inputs.s, v);
						auto f_set = udt->find_function(("set_" + v->name.str()).c_str());
						if (f_set)
						{
							auto f_set_addr = (char*)library + (uint)f_set->rva;
							Setter* setter = nullptr;
							switch (base_hash)
							{
							case FLAME_CHASH("bool"):
								setter = new Setter_t<bool>;
								break;
							case FLAME_CHASH("int"):
								setter = new Setter_t<int>;
								break;
							case FLAME_CHASH("flame::Vec(2+int)"):
								setter = new Setter_t<Vec2i>;
								break;
							case FLAME_CHASH("flame::Vec(3+int)"):
								setter = new Setter_t<Vec3i>;
								break;
							case FLAME_CHASH("flame::Vec(4+int)"):
								setter = new Setter_t<Vec4i>;
								break;
							case FLAME_CHASH("uint"):
								setter = new Setter_t<uint>;
								break;
							case FLAME_CHASH("flame::Vec(2+uint)"):
								setter = new Setter_t<Vec2u>;
								break;
							case FLAME_CHASH("flame::Vec(3+uint)"):
								setter = new Setter_t<Vec3u>;
								break;
							case FLAME_CHASH("flame::Vec(4+unt)"):
								setter = new Setter_t<Vec4u>;
								break;
							case FLAME_CHASH("float"):
								setter = new Setter_t<float>;
								break;
							case FLAME_CHASH("flame::Vec(2+float)"):
								setter = new Setter_t<Vec2f>;
								break;
							case FLAME_CHASH("flame::Vec(3+float)"):
								setter = new Setter_t<Vec3f>;
								break;
							case FLAME_CHASH("flame::Vec(4+float)"):
								setter = new Setter_t<Vec4f>;
								break;
							case FLAME_CHASH("uchar"):
								setter = new Setter_t<uchar>;
								break;
							case FLAME_CHASH("flame::Vec(2+uchar)"):
								setter = new Setter_t<Vec2c>;
								break;
							case FLAME_CHASH("flame::Vec(3+uchar)"):
								setter = new Setter_t<Vec3c>;
								break;
							case FLAME_CHASH("flame::Vec(4+uchar)"):
								setter = new Setter_t<Vec4c>;
								break;
							}
							setter->o = object;
							setter->f = f_set_addr;
							setter->s = parent->scene;
							input->setter = setter;
						}
						memcpy(input->default_value, input->data, input->size);
						inputs.push_back(input);
					}
				}
			}
		}
	}

	bpNodePrivate::~bpNodePrivate()
	{
		if (node_type == bpNodeReal)
		{
			if (dtor_addr)
				cmf(p2f<MF_v_v>(dtor_addr), object);
			free(object);
		}
		for (auto in : inputs)
			delete (bpSlotPrivate*)in;
		for (auto out : outputs)
			delete (bpSlotPrivate*)out;

		for (auto n : children)
			delete (bpNodePrivate*)n;
	}

	static void get_order(bpNodePrivate* n, uint& order)
	{
		if (n->order != 0xffffffff)
			return;
		for (auto i : n->inputs)
		{
			auto o = i->links[0];
			if (o)
			{
				auto n = o->node;
				if (n)
					get_order((bpNodePrivate*)n, order);
			}
		}
		n->order = order++;
	}

	static void build_update_list(bpNodePrivate* n)
	{
		for (auto n : n->children)
			((bpNodePrivate*)n)->order = 0xffffffff;
		auto order = 0U;
		for (auto n : n->children)
			get_order((bpNodePrivate*)n, order);
		n->update_list.clear();
		for (auto _n : n->children)
		{
			auto n = (bpNodePrivate*)_n;
			std::vector<bpNodePrivate*>::iterator it;
			for (it = n->update_list.begin(); it != n->update_list.end(); it++)
			{
				if (((bpNodePrivate*)n)->order < (*it)->order)
					break;
			}
			n->update_list.emplace(it, (bpNodePrivate*)n);
		}
		n->need_rebuild_update_list = false;
	}

	void bpNodePrivate::update()
	{
		for (auto _in : inputs)
		{
			auto in = (bpSlotPrivate*)_in;
			auto out = in->links[0];
			if (out)
			{
				if (out->type->tag == TypeData && in->type->tag == TypePointer)
					memcpy(in->data, &out->data, sizeof(void*));
				else
				{
					if (in->setter)
						in->setter->set(out->data);
					else
						memcpy(in->data, out->data, in->size);
				}
			}
		}

		if (update_addr)
			cmf(p2f<MF_v_v>(update_addr), object);

		if (need_rebuild_update_list)
			build_update_list(this);

		for (auto n : update_list)
			n->update();
	}

	static bool check_or_create_id(bpNodePrivate* parent, std::string& id)
	{
		if (!id.empty())
		{
			if (parent->find_node(id))
				return false;
		}
		else
		{
			id = std::to_string(::rand());
			while (parent->find_node(id))
				id = std::to_string(::rand());
		}
		return true;
	}

	bpNodePrivate* bpNodePrivate::add_node(const std::string& _id, const std::string& type, bpNodeType node_type)
	{
		std::string id = _id;
		if (!check_or_create_id(this, id))
		{
			printf("cannot add node, id repeated\n");
			return nullptr;
		}

		auto n = new bpNodePrivate(node_type, this, id, type);

		n->guid = generate_guid();
		children.push_back(n);

		need_rebuild_update_list = true;

		return n;
	}

	void bpNodePrivate::remove_node(bpNodePrivate* _n)
	{
		for (auto i = 0; i < children.s;i++)
		{
			auto n = children[i];
			if (n == _n)
			{
				for (auto in : n->inputs)
				{
					auto o = in->links[0];
					if (o)
					{
						for (auto j = 0; j < o->links.s; j++)
						{
							if (o->links[j] == in)
							{
								o->links.remove(j);
								break;
							}
						}
					}
				}
				for (auto o : n->outputs)
				{
					for (auto l : o->links)
						l->links[0] = nullptr;
				}
				delete (bpNodePrivate*)n;
				children.remove(i);
				break;
			}
		}

		need_rebuild_update_list = true;
	}

	bpScenePrivate::bpScenePrivate()
	{
		time = 0.f;

		root = new bpNodePrivate(bpNodeReal, nullptr, "root", "Group");
	}

	bpScenePrivate::~bpScenePrivate()
	{
		delete root;
	}

	static float bp_time = 0.f;

	void bpScenePrivate::update()
	{
		bp_time = time;

		root->update();

		time += looper().delta_time;
	}

	void bpSlot::set_data(const void* d)
	{
		((bpSlotPrivate*)this)->set_data(d);
	}

	bool bpSlot::link_to(bpSlot* target)
	{
		return ((bpSlotPrivate*)this)->link_to((bpSlotPrivate*)target);
	}

	bpNode* bpNode::add_node(const char* id, const char* type, bpNodeType node_type)
	{
		return ((bpNodePrivate*)this)->add_node(id, type, node_type);
	}

	void bpNode::remove_node(bpNode* n)
	{
		((bpNodePrivate*)this)->remove_node((bpNodePrivate*)n);
	}

	void bpNode::update()
	{
		((bpNodePrivate*)this)->update();
	}

	void bpScene::update()
	{
		((bpScenePrivate*)this)->update();
	}

	bpScene* bpScene::create_from_file(const wchar_t* filename)
	{
		auto s_filename = w2s(filename);
		auto path = std::filesystem::path(filename);
		auto ppath = path.parent_path();
		auto ppath_str = ppath.wstring();

		printf("begin to load bp: %s\n", s_filename.c_str());

		pugi::xml_document file;
		pugi::xml_node file_root;
		if (!file.load_file(filename) || (file_root = file.first_child()).name() != std::string("BP"))
		{
			printf("bp file does not exist, abort\n", s_filename.c_str());
			printf("end loading bp: %s\n", s_filename.c_str());
			return nullptr;
		}

		auto bp = new bpScenePrivate();
		bp->filename = filename;
		
		std::function<void(pugi::xml_node, bpNodePrivate*)> load_group;
		load_group = [&](pugi::xml_node n_group, bpNodePrivate* parent) {
			for (auto n_node : n_group.child("nodes"))
			{
				auto n = parent->add_node(n_node.attribute("id").value(), n_node.attribute("type").value(), (bpNodeType)n_node.attribute("node_type").as_int());
				if (n)
				{
					n->pos = stof2(n_node.attribute("pos").value());
					for (auto n_data : n_node.child("datas"))
					{
						auto input = n->find_input(n_data.attribute("name").value());
						auto type = input->type;
						auto tag = type->tag;
						if (!type->is_array && (tag == TypeEnumSingle || tag == TypeEnumMulti || tag == TypeData))
							type->unserialize(n_data.attribute("value").value(), input->data);
					}
				}

				auto g = n_node.child("group");
				if (g)
					load_group(g, n);
			}

			for (auto n_link : n_group.child("links"))
			{
				auto o_addr = std::string(n_link.attribute("out").value());
				auto i_addr = std::string(n_link.attribute("in").value());

				auto o = parent->find_output(o_addr);
				auto i = parent->find_input(i_addr);
				if (o && i)
				{
					if (!i->link_to(o))
						printf("link type mismatch: %s - > %s\n", o_addr.c_str(), i_addr.c_str());
				}
				else
					printf("cannot link: %s - > %s\n", o_addr.c_str(), i_addr.c_str());
			}
		};

		load_group(file_root.child("group"), (bpNodePrivate*)bp->root);

		printf("end loading bp: %s\n", s_filename.c_str());

		return bp;
	}

	void bpScene::save_to_file(bpScene* _bp, const wchar_t* filename)
	{
		auto bp = (bpScenePrivate*)_bp;

		bp->filename = filename;

		pugi::xml_document file;
		auto file_root = file.append_child("BP");

		std::function<void(pugi::xml_node, bpNodePrivate*)> save_group;
		save_group = [&](pugi::xml_node n_group, bpNodePrivate* parent) {
			auto n_nodes = n_group.append_child("nodes");
			for (auto n : parent->children)
			{
				auto n_node = n_nodes.append_child("node");
				n_node.append_attribute("node_type").set_value(n->node_type);
				n_node.append_attribute("id").set_value(n->id.v);
				n_node.append_attribute("type").set_value(n->type.v);
				n_node.append_attribute("pos").set_value(to_string(n->pos).c_str());

				pugi::xml_node n_datas;
				for (auto& in : n->inputs)
				{
					if (in->links[0])
						continue;
					auto type = in->type;
					if (type->tag != TypePointer)
					{
						if (in->default_value && memcmp(in->default_value, in->data, in->size) == 0)
							continue;
						if (!n_datas)
							n_datas = n_node.append_child("datas");
						auto n_data = n_datas.append_child("data");
						n_data.append_attribute("name").set_value(in->name.v);
						n_data.append_attribute("value").set_value(type->serialize(in->data).c_str());
					}
				}

				if (n->children.s > 0)
					save_group(n_group.append_child("group"), (bpNodePrivate*)n);
			}

			auto n_links = n_group.append_child("links");
			for (auto n : parent->children)
			{
				for (auto& in : n->inputs)
				{
					auto out = in->links[0];
					if (out)
					{
						auto n_link = n_links.append_child("link");
						n_link.append_attribute("out").set_value(out->get_address().v);
						n_link.append_attribute("in").set_value(in->get_address().v);
					}
				}
			}
		};


		save_group(file_root.append_child("group"), (bpNodePrivate*)bp->root);

		file.save_file(filename);

		//if (bp->need_rebuild_update_list)
		//	build_update_list(bp);
		//struct Var
		//{
		//	std::string type;
		//	std::string name;
		//	int ref_count;
		//};
		//struct Line
		//{
		//	int assign_var_idx; // -1 means it is not an assignment
		//	std::string content;
		//};
		//std::vector<Var> vars;
		//std::vector<Line> lines;
		//for (auto n : bp->update_list)
		//{
		//	auto var_id = [](bpSlotPrivate* s) {
		//		auto n = s->node;
		//		if (n->node_type != NodeReal)
		//			return std::string(n->udt->link_name()) + "->" + (s->setter ? "set_" : "") + s->name;
		//		return "_" + n->id + "_" + s->name;
		//	};

		//	switch (n->node_type)
		//	{
		//	case NodeRefWrite:
		//		for (auto& in : n->inputs)
		//		{
		//			auto out = (bpSlotPrivate*)in->links[0];
		//			std::string value;
		//			if (!out)
		//			{
		//				auto type = in->type;
		//				if (type->tag() == TypePointer || (in->default_value && memcmp(in->default_value, in->data, in->size) == 0))
		//					continue;
		//				value = type->serialize(in->data);
		//			}
		//			else
		//				value = var_id(out);
		//			auto line = var_id(in.get());
		//			if (in->setter)
		//				line += "(" + value + ");";
		//			else
		//				line += " = " + value + ";";
		//			lines.push_back({ -1, line });
		//		}
		//		break;
		//	case NodeReal:
		//	{
		//		std::string parameters;
		//		auto ntype = break_node_type(n->type, &parameters);
		//		if (ntype == 0)
		//		{
		//			auto f = n->udt->find_function("bp_update");
		//			assert(f && check_function(f, "D#void", {}));
		//			std::string function_code = f->code();
		//			for (auto& out : n->outputs)
		//			{
		//				auto id = var_id(out.get());

		//				vars.push_back({ out->type->get_cpp_name(), id, 0 });
		//				std::regex reg("\\b" + out->name + "\\b");
		//				function_code = std::regex_replace(function_code, reg, id);
		//			}
		//			for (auto& in : n->inputs)
		//			{
		//				auto out = (bpSlotPrivate*)in->links[0];
		//				std::string value;
		//				if (!out)
		//					value = in->type->serialize(in->data);
		//				else
		//					value = var_id(out);
		//				std::regex reg(R"(\b)" + in->name + R"(\b)");
		//				function_code = std::regex_replace(function_code, reg, value);
		//			}
		//			auto function_lines = SUS::split(function_code, '\n');
		//			for (auto& l : function_lines)
		//				lines.push_back({ -1, l });
		//		}
		//		else
		//		{
		//			switch (ntype)
		//			{
		//			case 'S':
		//			case 'M':
		//			{
		//				auto out_id = "_" + n->id + "_out";
		//				auto res_id = "_" + n->id + "_res";
		//				vars.push_back({ "uint", out_id, 0 });
		//				vars.push_back({ "float", res_id, 0 });
		//				std::string in_value;
		//				{
		//					auto in = n->inputs[0].get();
		//					auto out = (bpSlotPrivate*)in->links[0];
		//					in_value = out ? var_id(out) : std::to_string(*(int*)in->data);
		//					lines.push_back({ -1, out_id + " = " + in_value + ";" });
		//				}
		//				{
		//					auto in = n->inputs[1].get();
		//					auto out = (bpSlotPrivate*)in->links[0];
		//					if (ntype == 'S')
		//					{
		//						if (!out)
		//							lines.push_back({ -1, res_id + " = " + out_id + " == " + std::to_string(*(int*)in->data) + " ? 1.f : 0.f;" });
		//						else
		//							lines.push_back({ -1, res_id + " = " + in_value + " == " + var_id(out) + " ? 1.f : 0.f;" });
		//					}
		//					else
		//					{
		//						if (!out)
		//							lines.push_back({ -1, res_id + " = (" + out_id + " & " + std::to_string(*(int*)in->data) + ") ? 1.f : 0.f;" });
		//						else
		//							lines.push_back({ -1, res_id + " = (" + in_value + " & " + var_id(out) + ") ? 1.f : 0.f;" });
		//					}
		//				}
		//			}
		//				break;
		//			case 'V':
		//				assert(0); // WIP
		//				break;
		//			case 'A':
		//				assert(0); // WIP
		//				break;
		//			}
		//		}
		//	}
		//		break;
		//	}
		//}

		//for (auto i = 0; i < vars.size(); i++)
		//{
		//	std::regex reg(R"(^)" + vars[i].name + R"( = (.*);)");
		//	for (auto& l : lines)
		//	{
		//		if (l.assign_var_idx == -1)
		//		{
		//			std::smatch res;
		//			if (std::regex_search(l.content, res, reg))
		//			{
		//				l.content = res[1].str();
		//				l.assign_var_idx = i;
		//			}
		//		}
		//	}
		//}

		//for (auto i = 0; i < lines.size(); i++)
		//{
		//	auto& l = lines[i];
		//	if (l.assign_var_idx != -1)
		//	{
		//		auto& v = vars[l.assign_var_idx];
		//		std::regex reg(R"(\b)" + v.name + R"(\b)");
		//		auto ref_count = 0, last_ref_line = -1;
		//		for (auto j = i + 1; j < lines.size(); j++)
		//		{
		//			std::smatch res;
		//			auto str = lines[j].content;
		//			while (std::regex_search(str, res, reg))
		//			{
		//				last_ref_line = j;
		//				ref_count++;
		//				str = res.suffix();
		//			}
		//		}
		//		v.ref_count += ref_count;
		//		if (ref_count == 1)
		//		{
		//			lines[last_ref_line].content = std::regex_replace(lines[last_ref_line].content, reg, "(" + l.content + ")");
		//			v.ref_count--;
		//		}
		//		if (ref_count <= 1)
		//			l.content.clear();
		//	}
		//}

		//std::ofstream h_file(filename + std::wstring(L".h"));
		//h_file << "// THIS FILE IS AUTO-GENERATED\n";
		//for (auto& v : vars)
		//{
		//	if (v.ref_count == 0)
		//		continue;
		//	h_file << v.type + " " + v.name + ";\n";
		//}
		//for (auto& l : lines)
		//{
		//	if (l.content.empty())
		//		continue;
		//	if (l.assign_var_idx == -1)
		//		h_file << l.content + "\n";
		//	else
		//		h_file << vars[l.assign_var_idx].name + " = " + l.content + ";\n";
		//}
		//h_file.close();
	}

	void bpScene::destroy(bpScene *bp)
	{
		delete(bpScenePrivate*)bp;
	}

	struct FLAME_R(R_MakeVec2i)
	{
		FLAME_RV(int, x, i);
		FLAME_RV(int, y, i);

		FLAME_RV(Vec2i, out, o);

		FLAME_FOUNDATION_EXPORTS void FLAME_RF(bp_update)()
		{
			out = Vec2i(x, y);
		}
	};

	struct FLAME_R(R_BreakVec2i)
	{
		FLAME_RV(Vec2i, in, i);

		FLAME_RV(int, x, o);
		FLAME_RV(int, y, o);

		FLAME_FOUNDATION_EXPORTS void FLAME_RF(bp_update)()
		{
			x = in[0];
			y = in[1];
		}
	};

	struct FLAME_R(R_MakeVec3i)
	{
		FLAME_RV(int, x, i);
		FLAME_RV(int, y, i);
		FLAME_RV(int, z, i);

		FLAME_RV(Vec3i, out, o);

		FLAME_FOUNDATION_EXPORTS void FLAME_RF(bp_update)()
		{
			out = Vec3i(x, y, z);
		}
	};

	struct FLAME_R(R_BreakVec3i)
	{
		FLAME_RV(Vec3i, in, i);

		FLAME_RV(int, x, o);
		FLAME_RV(int, y, o);
		FLAME_RV(int, z, o);

		FLAME_FOUNDATION_EXPORTS void FLAME_RF(bp_update)()
		{
			x = in[0];
			y = in[1];
			z = in[2];
		}
	};

	struct FLAME_R(R_MakeVec4i)
	{
		FLAME_RV(int, x, i);
		FLAME_RV(int, y, i);
		FLAME_RV(int, z, i);
		FLAME_RV(int, w, i);

		FLAME_RV(Vec4i, out, o);

		FLAME_FOUNDATION_EXPORTS void FLAME_RF(bp_update)()
		{
			out = Vec4i(x, y, z, w);
		}
	};

	struct FLAME_R(R_BreakVec4i)
	{
		FLAME_RV(Vec4i, in, i);

		FLAME_RV(int, x, o);
		FLAME_RV(int, y, o);
		FLAME_RV(int, z, o);
		FLAME_RV(int, w, o);

		FLAME_FOUNDATION_EXPORTS void FLAME_RF(bp_update)()
		{
			x = in[0];
			y = in[1];
			z = in[2];
			w = in[3];
		}
	};

	struct FLAME_R(R_MakeVec2u)
	{
		FLAME_RV(uint, x, i);
		FLAME_RV(uint, y, i);

		FLAME_RV(Vec2u, out, o);

		FLAME_FOUNDATION_EXPORTS void FLAME_RF(bp_update)()
		{
			out = Vec2u(x, y);
		}
	};

	struct FLAME_R(R_BreakVec2u)
	{
		FLAME_RV(Vec2u, in, i);

		FLAME_RV(uint, x, o);
		FLAME_RV(uint, y, o);

		FLAME_FOUNDATION_EXPORTS void FLAME_RF(bp_update)()
		{
			x = in[0];
			y = in[1];
		}
	};

	struct FLAME_R(R_MakeVec3u)
	{
		FLAME_RV(uint, x, i);
		FLAME_RV(uint, y, i);
		FLAME_RV(uint, z, i);

		FLAME_RV(Vec3u, out, o);

		FLAME_FOUNDATION_EXPORTS void FLAME_RF(bp_update)()
		{
			out = Vec3u(x, y, z);
		}
	};

	struct FLAME_R(R_BreakVec3u)
	{
		FLAME_RV(Vec3u, in, i);

		FLAME_RV(uint, x, o);
		FLAME_RV(uint, y, o);
		FLAME_RV(uint, z, o);

		FLAME_FOUNDATION_EXPORTS void FLAME_RF(bp_update)()
		{
			x = in[0];
			y = in[1];
			z = in[2];
		}
	};

	struct FLAME_R(R_MakeVec4u)
	{
		FLAME_RV(uint, x, i);
		FLAME_RV(uint, y, i);
		FLAME_RV(uint, z, i);
		FLAME_RV(uint, w, i);

		FLAME_RV(Vec4u, out, o);

		FLAME_FOUNDATION_EXPORTS void FLAME_RF(bp_update)()
		{
			out = Vec4u(x, y, z, w);
		}
	};

	struct FLAME_R(R_BreakVec4u)
	{
		FLAME_RV(Vec4u, in, i);

		FLAME_RV(uint, x, o);
		FLAME_RV(uint, y, o);
		FLAME_RV(uint, z, o);
		FLAME_RV(uint, w, o);

		FLAME_FOUNDATION_EXPORTS void FLAME_RF(bp_update)()
		{
			x = in[0];
			y = in[1];
			z = in[2];
			w = in[3];
		}
	};

	struct FLAME_R(R_MakeVec2f)
	{
		FLAME_RV(float, x, i);
		FLAME_RV(float, y, i);

		FLAME_RV(Vec2f, out, o);

		FLAME_FOUNDATION_EXPORTS void FLAME_RF(bp_update)()
		{
			out = Vec2f(x, y);
		}
	};

	struct FLAME_R(R_BreakVec2f)
	{
		FLAME_RV(Vec2f, in, i);

		FLAME_RV(float, x, o);
		FLAME_RV(float, y, o);

		FLAME_FOUNDATION_EXPORTS void FLAME_RF(bp_update)()
		{
			x = in[0];
			y = in[1];
		}
	};

	struct FLAME_R(R_MakeVec3f)
	{
		FLAME_RV(float, x, i);
		FLAME_RV(float, y, i);
		FLAME_RV(float, z, i);

		FLAME_RV(Vec3f, out, o);

		FLAME_FOUNDATION_EXPORTS void FLAME_RF(bp_update)()
		{
			out = Vec3f(x, y, z);
		}
	};

	struct FLAME_R(R_BreakVec3f)
	{
		FLAME_RV(Vec3f, in, i);

		FLAME_RV(float, x, o);
		FLAME_RV(float, y, o);
		FLAME_RV(float, z, o);

		FLAME_FOUNDATION_EXPORTS void FLAME_RF(bp_update)()
		{
			x = in[0];
			y = in[1];
			z = in[2];
		}
	};

	struct FLAME_R(R_MakeVec4f)
	{
		FLAME_RV(float, x, i);
		FLAME_RV(float, y, i);
		FLAME_RV(float, z, i);
		FLAME_RV(float, w, i);

		FLAME_RV(Vec4f, out, o);

		FLAME_FOUNDATION_EXPORTS void FLAME_RF(bp_update)()
		{
			out = Vec4f(x, y, z, w);
		}
	};

	struct FLAME_R(R_BreakVec4f)
	{
		FLAME_RV(Vec4f, in, i);

		FLAME_RV(float, x, o);
		FLAME_RV(float, y, o);
		FLAME_RV(float, z, o);
		FLAME_RV(float, w, o);

		FLAME_FOUNDATION_EXPORTS void FLAME_RF(bp_update)()
		{
			x = in[0];
			y = in[1];
			z = in[2];
			w = in[3];
		}
	};

	struct FLAME_R(R_MakeVec2c)
	{
		FLAME_RV(uchar, x, i);
		FLAME_RV(uchar, y, i);

		FLAME_RV(Vec2c, out, o);

		FLAME_FOUNDATION_EXPORTS void FLAME_RF(bp_update)()
		{
			out = Vec2c(x, y);
		}
	};

	struct FLAME_R(R_BreakVec2c)
	{
		FLAME_RV(Vec2c, in, i);

		FLAME_RV(uchar, x, o);
		FLAME_RV(uchar, y, o);

		FLAME_FOUNDATION_EXPORTS void FLAME_RF(bp_update)()
		{
			x = in[0];
			y = in[1];
		}
	};

	struct FLAME_R(R_MakeVec3c)
	{
		FLAME_RV(uchar, x, i);
		FLAME_RV(uchar, y, i);
		FLAME_RV(uchar, z, i);

		FLAME_RV(Vec3c, out, o);

		FLAME_FOUNDATION_EXPORTS void FLAME_RF(bp_update)()
		{
			out = Vec3c(x, y, z);
		}
	};

	struct FLAME_R(R_BreakVec3c)
	{
		FLAME_RV(Vec3c, in, i);

		FLAME_RV(uchar, x, o);
		FLAME_RV(uchar, y, o);
		FLAME_RV(uchar, z, o);

		FLAME_FOUNDATION_EXPORTS void FLAME_RF(bp_update)()
		{
			x = in[0];
			y = in[1];
			z = in[2];
		}
	};

	struct FLAME_R(R_MakeVec4c)
	{
		FLAME_RV(uchar, x, i);
		FLAME_RV(uchar, y, i);
		FLAME_RV(uchar, z, i);
		FLAME_RV(uchar, w, i);

		FLAME_RV(Vec4c, out, o);

		FLAME_FOUNDATION_EXPORTS void FLAME_RF(bp_update)()
		{
			out = Vec4c(x, y, z, w);
		}
	};

	struct FLAME_R(R_BreakVec4c)
	{
		FLAME_RV(Vec4c, in, i);

		FLAME_RV(uchar, x, o);
		FLAME_RV(uchar, y, o);
		FLAME_RV(uchar, z, o);
		FLAME_RV(uchar, w, o);

		FLAME_FOUNDATION_EXPORTS void FLAME_RF(bp_update)()
		{
			x = in[0];
			y = in[1];
			z = in[2];
			w = in[3];
		}
	};

	struct FLAME_R(R_Add)
	{
		FLAME_RV(float, a, i);
		FLAME_RV(float, b, i);

		FLAME_RV(float, out, o);

		FLAME_FOUNDATION_EXPORTS void FLAME_RF(bp_update)()
		{
			out = a + b;
		}
	};

	struct FLAME_R(R_Multiple)
	{
		FLAME_RV(float, a, i);
		FLAME_RV(float, b, i);

		FLAME_RV(float, out, o);

		FLAME_FOUNDATION_EXPORTS void FLAME_RF(bp_update)()
		{
			out = a * b;
		}
	};

	struct FLAME_R(R_Time)
	{
		FLAME_RV(float, delta, o);
		FLAME_RV(float, total, o);

		FLAME_FOUNDATION_EXPORTS void FLAME_RF(bp_update)()
		{
			delta = looper().delta_time;
			total = bp_time;
		}
	};

	struct FLAME_R(R_Sin)
	{
		FLAME_RV(float, t, i);

		FLAME_RV(float, out, o);

		FLAME_FOUNDATION_EXPORTS void FLAME_RF(bp_update)()
		{
			out = sin(t * M_PI / 180.f);
		}
	};

	struct FLAME_R(R_Linear1d)
	{
		FLAME_RV(float, a, i);
		FLAME_RV(float, b, i);
		FLAME_RV(float, t, i);

		FLAME_RV(float, out, o);

		FLAME_FOUNDATION_EXPORTS void FLAME_RF(bp_update)()
		{
			out = a + (b - a) * clamp(t, 0.f, 1.f);
		}
	};

	struct FLAME_R(R_Linear2d)
	{
		FLAME_RV(Vec2f, a, i);
		FLAME_RV(Vec2f, b, i);
		FLAME_RV(float, t, i);

		FLAME_RV(Vec2f, out, o);

		FLAME_FOUNDATION_EXPORTS void FLAME_RF(bp_update)()
		{
			out = a + (b - a) * clamp(t, 0.f, 1.f);
		}
	};

	struct FLAME_R(R_Trace)
	{
		FLAME_RV(float, target, i);
		FLAME_RV(float, step, i);
		FLAME_RV(float, v, i);

		FLAME_RV(float, out, o);

		FLAME_FOUNDATION_EXPORTS void FLAME_RF(bp_update)()
		{
			out = v + min(abs(target - v), step) * sign(target - v);
		}
	};

	struct FLAME_R(R_KeyListener)
	{
		FLAME_RV(Key, key, i);

		FLAME_FOUNDATION_EXPORTS void FLAME_RF(bp_update)()
		{

		}
	};

	struct FLAME_R(R_Print)
	{
		FLAME_RV(StringA, text, i);

		FLAME_FOUNDATION_EXPORTS void FLAME_RF(bp_update)()
		{
			printf("%s\n", text.v);
		}
	};
}

