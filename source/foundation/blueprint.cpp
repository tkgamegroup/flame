#include <flame/serialize.h>
#include "foundation_private.h"
#include "blueprint_private.h"
#include "typeinfo_private.h"

namespace flame
{
	bpSlotPrivate::bpSlotPrivate(bpNodePrivate* node, bpSlotIO io, uint index, TypeInfoPrivate* type, const std::string& name, uint offset, uint size, const void* default_value) :
		_node(node),
		_io(io),
		_index(index),
		_type(type),
		_name(name),
		_offset(offset),
		_size(size)
	{
		user_data = nullptr;

		if (default_value)
		{
			_default_value = new char[size];
			memcpy(_default_value, default_value, size);
		}

		_data = (char*)node->_object + offset;

		if (io == bpSlotIn)
			_links.push_back(nullptr);
	}

	bpSlotPrivate::bpSlotPrivate(bpNodePrivate* node, bpSlotIO io, uint index, VariableInfoPrivate* vi) :
		bpSlotPrivate(node, io, index, vi->_type, vi->_name, vi->_offset, vi->_type->_size, vi->_default_value)
	{
	}
	 
	bpSlotPrivate::~bpSlotPrivate()
	{
		delete[]_default_value;
		delete _setter;
	}

	void bpSlotPrivate::_set_data(const void* d)
	{
		if (!_setter)
			_type->copy(d, _data);
		else
			_setter->set(d);
	}

	bool bpSlotPrivate::_link_to(bpSlotPrivate* target)
	{
		assert(_io == bpSlotIn);
		if (_io == bpSlotIn)
			return false;

		if (_links[0] == target)
			return true;

		if (target)
		{
			if (target->_io == bpSlotIn)
				return false;
			if (_node && _node == target->_node)
				return false;
		}

		if (target)
		{
			if (!bp_can_link(_type, target->_type))
				return false;
		}

		{
			auto o = _links[0];
			if (o)
			{
				erase_if(o->_links, this);
				if (_type->_name == "ListenerHub")
					(*(ListenerHub<void(Capture&)>**)_data)->remove(_listener);
			}
		}

		_links[0] = target;
		if (target)
		{
			target->_links.push_back(this);
			if (_type->_name == "ListenerHub")
			{
				auto p = target->_data;
				memcpy(_data, &p, sizeof(void*));
				_listener = (*(ListenerHub<void(Capture&)>**)_data)->add([](Capture& c) {
					c.thiz<bpNode>()->update();
				}, Capture().set_thiz(_node));
			}
		}

		if (!target && _type->_tag == TypePointer)
			memset(_data, 0, sizeof(void*));

		return true;
	}

	static void* next_var(void* &p, uint s)
	{
		auto ret = p;
		p = (char*)p + s;
		return ret;
	}

	template <class T>
	static T* next_var(void*& p)
	{
		return (T*)next_var(p, sizeof(T));
	}

	bpNodePrivate::bpNodePrivate(bpScenePrivate* scene, bpNodePrivate* parent, const std::string& id, bpNodeType type, const std::string& type_parameter, bpObjectRule object_rule) :
		_scene(scene),
		_parent(parent),
		_id(id),
		_object_rule(object_rule),
		_type(type)
	{
		user_data = nullptr;

		switch (type)
		{
		case bpNodeEnumSingle:
		{
			assert(_object_rule == bpObjectEntity);
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
			auto size = _udt->_size;
			_object = malloc(size);
			memset(_object, 0, size);
			_inputs.emplace_back(new bpSlotPrivate(this, bpSlotIn, 0, TypeInfoPrivate::_get(TypeEnumSingle, type_parameter), "in", offsetof(Dummy, in), sizeof(Dummy::in), nullptr));
			_inputs.emplace_back(new bpSlotPrivate(this, bpSlotIn, 1, TypeInfoPrivate::_get(TypeEnumSingle, type_parameter), "chk", offsetof(Dummy, chk), sizeof(Dummy::chk), nullptr));
			_outputs.emplace_back(new bpSlotPrivate(this, bpSlotOut, 0, TypeInfoPrivate::_get(TypeEnumSingle, type_parameter), "out", offsetof(Dummy, out), sizeof(Dummy::out), nullptr));
			_outputs.emplace_back(new bpSlotPrivate(this, bpSlotOut, 1, TypeInfoPrivate::_get(TypeData, "float"), "res", offsetof(Dummy, res), sizeof(Dummy::res), nullptr));
			_update_addr = f2v(&Dummy::update);
		}
			break;
		case bpNodeEnumMulti:
		{
			assert(_object_rule == bpObjectEntity);
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
			auto size = sizeof(Dummy);
			_object = malloc(size);
			memset(_object, 0, size);
			_inputs.emplace_back(new bpSlotPrivate(this, bpSlotIn, 0, TypeInfoPrivate::_get(TypeEnumMulti, type_parameter), "in", offsetof(Dummy, in), sizeof(Dummy::in), nullptr));
			_inputs.emplace_back(new bpSlotPrivate(this, bpSlotIn, 1, TypeInfoPrivate::_get(TypeEnumSingle, type_parameter), "chk", offsetof(Dummy, chk), sizeof(Dummy::chk), nullptr));
			_outputs.emplace_back(new bpSlotPrivate(this, bpSlotOut, 0, TypeInfoPrivate::_get(TypeEnumMulti, type_parameter), "out", offsetof(Dummy, out), sizeof(Dummy::out), nullptr));
			_outputs.emplace_back(new bpSlotPrivate(this, bpSlotOut, 1, TypeInfoPrivate::_get(TypeData, "float"), "res", offsetof(Dummy, res), sizeof(Dummy::res), nullptr));
			_update_addr = f2v(&Dummy::update);
		}
			break;
		case bpNodeVariable:
		{
			assert(_object_rule == bpObjectEntity);
#pragma pack(1)
			struct Dummy
			{
				TypeInfoPrivate* type;

				void dtor()
				{
					auto p = var_end(&type);
					auto in = next_var(p, type->_size);
					auto out = next_var(p, type->_size);
					type->destruct(in);
					type->destruct(out);
				}

				void update()
				{
					auto p = var_end(&type);
					auto in = next_var(p, type->_size);
					auto out = next_var(p, type->_size);
					type->copy(in, out);
				}
			};
#pragma pack()
			auto type = TypeInfoPrivate::_get(TypeData, type_parameter);
			auto type_size = type->_size;

			auto size = sizeof(Dummy) + type_size * 2;
			_object = malloc(size);
			memset(_object, 0, size);
			_inputs.emplace_back(new bpSlotPrivate(this, bpSlotIn, 0, TypeInfoPrivate::_get(TypeData, type_parameter), "in", sizeof(Dummy), type_size, nullptr));
			_outputs.emplace_back(new bpSlotPrivate(this, bpSlotOut, 0, TypeInfoPrivate::_get(TypeData, type_parameter), "out", sizeof(Dummy) + type_size, type_size, nullptr));
			_dtor_addr = f2v(&Dummy::dtor);
			_update_addr = f2v(&Dummy::update);
			auto& obj = *(Dummy*)_object;
			obj.type = type;
		}
			break;
		case bpNodeArray:
		{
			assert(_object_rule == bpObjectEntity);
			auto sp = SUS::split(type_parameter, ',');
#pragma pack(1)
			struct Dummy
			{
				TypeInfoPrivate* type;
				uint length;

				void dtor()
				{
					auto p = var_end(&length);
					auto& out = *next_var<Array<int>>(p);
					for (auto i = 0; i < out.s; i++)
						type->destruct((char*)out.v + type->_size * i);
					for (auto i = 0; i < length; i++)
						type->destruct(next_var(p, type->_size));
					f_free(out.v);
				}

				void update()
				{
					auto p = var_end(&length);
					auto& out = *next_var<Array<int>>(p);
					if (out.s != length)
					{
						out.s = length;
						auto m_size = type->_size * length;
						f_free(out.v);
						out.v = (int*)f_malloc(m_size);
						memset(out.v, 0, m_size);
					}
					for (auto i = 0; i < length; i++)
					{
						type->copy(next_var(p, type->_size), (char*)out.v + type->_size * i);
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
			auto type = TypeInfoPrivate::_get(tag, base_name);
			uint type_size = type->_size;
			auto length = stoi(sp[0]);
			auto size = sizeof(Dummy) + sizeof(Array<int>) + type_size * length;
			_object = malloc(size);
			memset(_object, 0, size);
			_outputs.emplace_back(new bpSlotPrivate(this, bpSlotOut, 0, TypeInfoPrivate::_get(TypeArrayOfData, type_name), "out", sizeof(Dummy), sizeof(Array<int>), nullptr));
			for (auto i = 0; i < length; i++)
				_inputs.emplace_back(new bpSlotPrivate(this, bpSlotIn, i, TypeInfoPrivate::_get(tag, base_name), std::to_string(i), sizeof(Dummy) + sizeof(Array<int>) + type_size * i, type_size, nullptr));
			_dtor_addr = f2v(&Dummy::dtor);
			_update_addr = f2v(&Dummy::update);
			auto& obj = *(Dummy*)_object;
			obj.type = type;
			obj.length = length;
		}
			break;
		case bpNodeGroup:
		{
			assert(_object_rule == bpObjectEntity);
#pragma pack(1)
			struct Dummy
			{
				ListenerHub<void()>* signal;
			};
#pragma pack()
			auto size = sizeof(Dummy);
			_object = malloc(size);
			memset(_object, 0, size);
			_inputs.emplace_back(new bpSlotPrivate(this, bpSlotIn, 0, TypeInfoPrivate::_get(TypePointer, "ListenerHub"), "signal", offsetof(Dummy, signal), sizeof(Dummy::signal), nullptr));
		}
			break;
		case bpNodeUdt:
		{
			_udt = _find_udt(FLAME_HASH(type_parameter.c_str()));

			if (!_udt)
			{
				assert(0);
				printf("add node: udt not found: %s\n", type_parameter.c_str());
			}

			_library = (void*)_udt->_db->_library;

			if (_object_rule == bpObjectEntity)
			{
				auto size = _udt->_size;
				_object = malloc(size);
				memset(_object, 0, size);

				{
					auto f = _udt->_find_function("ctor");
					if (f && f->check(FLAME_TYPE_HASH_dn("void"), 0))
						cmf(p2f<MF_v_v>((char*)_library + (uint)f->_rva), _object);
				}

				{
					auto f = _udt->_find_function("dtor");
					if (f && f->check(FLAME_TYPE_HASH_dn("void"), 0))
						_dtor_addr = (char*)_library + (uint)f->_rva;
				}

				{
					auto f = _udt->_find_function("bp_update");
					assert(f && f->check(FLAME_TYPE_HASH_dn("void"), 0));
					_update_addr = (char*)_library + (uint)f->_rva;
				}

				for (auto& v : _udt->_variables)
				{
					if (v->_flags & VariableFlagOutput)
						_outputs.emplace_back(new bpSlotPrivate(this, bpSlotOut, _outputs.size(), v.get()));
					else
						_inputs.emplace_back(new bpSlotPrivate(this, bpSlotIn, _inputs.size(), v.get()));
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

				if (_object_rule == bpObjectRefRead)
				{
					for (auto& v : _udt->_variables)
						_outputs.emplace_back(new bpSlotPrivate(this, bpSlotOut, _outputs.size(), v.get()));
				}
				else if (_object_rule == bpObjectRefWrite)
				{
					for (auto& v : _udt->_variables)
					{
						auto type = v->_type;
						if (type->_tag != TypeData)
							continue;
						auto input = new bpSlotPrivate(this, bpSlotIn, _inputs.size(), v.get());
						auto f_set = _udt->_find_function((std::string("set_") + v->_name).c_str());
						if (f_set)
						{
							auto f_set_addr = (char*)_library + (uint)f_set->_rva;
							Setter* setter = nullptr;
							switch (type->_name_hash)
							{
							case FLAME_CHASH("bool"):
								setter = new Setter_t<bool>;
								break;
							case FLAME_CHASH("int"):
								setter = new Setter_t<int>;
								break;
							case FLAME_CHASH("flame::Vec<2,int>"):
								setter = new Setter_t<Vec2i>;
								break;
							case FLAME_CHASH("flame::Vec<3,int>"):
								setter = new Setter_t<Vec3i>;
								break;
							case FLAME_CHASH("flame::Vec<4,int>"):
								setter = new Setter_t<Vec4i>;
								break;
							case FLAME_CHASH("uint"):
								setter = new Setter_t<uint>;
								break;
							case FLAME_CHASH("flame::Vec<2,uint>"):
								setter = new Setter_t<Vec2u>;
								break;
							case FLAME_CHASH("flame::Vec<3,uint>"):
								setter = new Setter_t<Vec3u>;
								break;
							case FLAME_CHASH("flame::Vec<4,unt>"):
								setter = new Setter_t<Vec4u>;
								break;
							case FLAME_CHASH("float"):
								setter = new Setter_t<float>;
								break;
							case FLAME_CHASH("flame::Vec<2,float>"):
								setter = new Setter_t<Vec2f>;
								break;
							case FLAME_CHASH("flame::Vec<3,float>"):
								setter = new Setter_t<Vec3f>;
								break;
							case FLAME_CHASH("flame::Vec<4,float>"):
								setter = new Setter_t<Vec4f>;
								break;
							case FLAME_CHASH("uchar"):
								setter = new Setter_t<uchar>;
								break;
							case FLAME_CHASH("flame::Vec<2,uchar>"):
								setter = new Setter_t<Vec2c>;
								break;
							case FLAME_CHASH("flame::Vec<3,uchar>"):
								setter = new Setter_t<Vec3c>;
								break;
							case FLAME_CHASH("flame::Vec<4,uchar>"):
								setter = new Setter_t<Vec4c>;
								break;
							}
							setter->o = _object;
							setter->f = f_set_addr;
							input->_setter = setter;
						}
						memcpy(input->_default_value, input->_data, input->_size);
						_inputs.emplace_back(input);
					}
				}
			}
		}
			break;
		}
	}

	bpNodePrivate::~bpNodePrivate()
	{
		if (_object_rule == bpObjectEntity)
		{
			if (_dtor_addr)
				cmf(p2f<MF_v_v>(_dtor_addr), _object);
			free(_object);
		}
	}

	bool bpNodePrivate::_set_id(const std::string& id)
	{
		if (id.empty())
			return false;
		if (_id == id)
			return true;
		if (_parent->_find_child(id))
			return false;
		_id = id;
		return true;
	}

	bpSlotPrivate* bpNodePrivate::_find_input(const std::string& name) const
	{
		for (auto& in : _inputs)
		{
			if (in->_name == name)
				return in.get();
		}
		return nullptr;
	}

	bpSlotPrivate* bpNodePrivate::_find_output(const std::string& name) const
	{
		for (auto& out : _outputs)
		{
			if (out->_name == name)
				return out.get();
		}
		return nullptr;
	}

	static void get_order(bpNodePrivate* n, uint& order)
	{
		if (n->_order != 0xffffffff)
			return;
		for (auto& i : n->_inputs)
		{
			auto o = i->_links[0];
			if (o)
			{
				auto n = o->_node;
				if (n)
					get_order(n, order);
			}
		}
		n->_order = order++;
	}

	static void build_update_list(bpNodePrivate* n)
	{
		for (auto& c : n->_children)
			c->_order = 0xffffffff;
		auto order = 0U;
		for (auto& c : n->_children)
			get_order(c.get(), order);
		n->_update_list.clear();
		for (auto& c : n->_children)
		{
			std::vector<bpNodePrivate*>::iterator it;
			for (it = n->_update_list.begin(); it != n->_update_list.end(); it++)
			{
				if (c->_order < (*it)->_order)
					break;
			}
			n->_update_list.emplace(it, c.get());
		}
		n->_need_rebuild_update_list = false;
	}

	void bpNodePrivate::_update()
	{
		for (auto& in : _inputs)
		{
			auto out = in->_links[0];
			if (out)
			{
				if (out->_type->_tag == TypeData && in->_type->_tag == TypePointer)
					memcpy(in->_data, &out->_data, sizeof(void*));
				else
				{
					if (in->_setter)
						in->_setter->set(out->_data);
					else
						memcpy(in->_data, out->_data, in->_size);
				}
			}
		}

		if (_update_addr)
			cmf(p2f<MF_v_v>(_update_addr), _object);

		if (_need_rebuild_update_list)
			build_update_list(this);

		for (auto n : _update_list)
			n->_update();
	}

	static bool check_or_create_id(bpNodePrivate* parent, std::string& id)
	{
		if (!id.empty())
		{
			if (parent->_find_child(id))
				return false;
		}
		else
		{
			id = std::to_string(::rand());
			while (parent->_find_child(id))
				id = std::to_string(::rand());
		}
		return true;
	}

	bpNodePrivate* bpNodePrivate::_add_child(const std::string& _id, bpNodeType type, const std::string& type_parameter, bpObjectRule object_rule)
	{
		std::string id = _id;
		if (!check_or_create_id(this, id))
		{
			printf("cannot add node, id repeated\n");
			return nullptr;
		}

		auto n = new bpNodePrivate(_scene, this, id, type, type_parameter, _object_rule);
		n->_guid = generate_guid();
		_children.emplace_back(n);

		_need_rebuild_update_list = true;

		return n;
	}
	void bpNodePrivate::_remove_child(bpNodePrivate* n)
	{
		auto it = std::find_if(_children.begin(), _children.end(), [&](const auto& t) {
			return t.get() == n;
		});
		if (it != _children.end())
		{
			for (auto& in : n->_inputs)
			{
				auto o = in->_links[0];
				if (o)
				{
					for (auto it = o->_links.begin(); it != o->_links.end(); it++)
					{
						if (*it == in.get())
						{
							o->_links.erase(it);
							break;
						}
					}
				}
			}
			for (auto& o : n->_outputs)
			{
				for (auto l : o->_links)
					l->_links[0] = nullptr;
			}
			_children.erase(it);

			_need_rebuild_update_list = true;
		}
	}
	bpNodePrivate* bpNodePrivate::_find_child(const std::string& name) const
	{
		for (auto& n : _children)
		{
			if (n->_id == name)
				return n.get();
		}
		return nullptr;
	}
	bpNodePrivate* bpNodePrivate::_find_child(const Guid& guid) const
	{
		for (auto& n : _children)
		{
			if (memcmp(&n->_guid, &guid, sizeof(Guid)) == 0)
				return n.get();
			auto res = n->_find_child(guid);
			if (res)
				return res;
		}
		return nullptr;
	}

	bpScenePrivate::bpScenePrivate()
	{
		_time = 0.f;

		_root.reset(new bpNodePrivate(this, nullptr, "root", bpNodeGroup, "", bpObjectEntity));
	}

	static float bp_time = 0.f;

	void bpScenePrivate::_update()
	{
		bp_time = _time;

		_root->update();

		_time += _looper->_delta_time;
	}

	void bpScenePrivate::_save()
	{
		pugi::xml_document file;
		auto file_root = file.append_child("BP");

		std::function<void(pugi::xml_node, bpNodePrivate*)> save_group;
		save_group = [&](pugi::xml_node n_group, bpNodePrivate* parent) {
			auto n_nodes = n_group.append_child("nodes");
			for (auto& n : parent->_children)
			{
				auto n_node = n_nodes.append_child("node");
				n_node.append_attribute("id").set_value(n->_id.c_str());
				n_node.append_attribute("type").set_value(n->_type);
				n_node.append_attribute("type_parameter").set_value(n->_type_parameter.c_str());
				n_node.append_attribute("object_rule").set_value(n->_object_rule);
				n_node.append_attribute("pos").set_value(to_string(n->_pos).c_str());

				pugi::xml_node n_datas;
				for (auto& in : n->_inputs)
				{
					if (in->_links[0])
						continue;
					auto type = in->_type;
					if (type->_tag != TypePointer)
					{
						if (in->_default_value && memcmp(in->_default_value, in->_data, in->_size) == 0)
							continue;
						if (!n_datas)
							n_datas = n_node.append_child("datas");
						auto n_data = n_datas.append_child("data");
						n_data.append_attribute("name").set_value(in->_name.c_str());
						n_data.append_attribute("value").set_value(type->serialize_s(in->_data).c_str());
					}
				}

				if (!n->_children.empty())
					save_group(n_group.append_child("group"), n.get());
			}

			auto n_links = n_group.append_child("links");
			for (auto& n : parent->_children)
			{
				for (auto& in : n->_inputs)
				{
					auto out = in->_links[0];
					if (out)
					{
						auto n_link = n_links.append_child("link");
						n_link.append_attribute("out").set_value((out->_node->_id + "." + out->_name).c_str());
						n_link.append_attribute("in").set_value((n->_id + "." + in->_name).c_str());
					}
				}
			}
		};


		save_group(file_root.append_child("group"), _root.get());

		file.save_file(_filename.c_str());

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
		//			auto out = in->links[0];
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
		//		std::string type_parameter;
		//		auto ntype = break_node_type(n->type, &type_parameter);
		//		if (ntype == 0)
		//		{
		//			auto f = n->udt->find_function("bp_update");
		//			assert(f && check_function(f, "D#void", {}));
		//			std::string function_code = f->code();
		//			for (auto& out : n->outputs)
		//			{
		//				auto id = var_id(out.get());

		//				auto type = out->type;
		//				auto name = type->base_name;
		//				static auto str_flame = std::string("flame::");
		//				if (name.compare(0, str_flame.size(), str_flame.c_str()) == 0)
		//					name.erase(name.begin(), name.begin() + str_flame.size());
		//				std::regex reg_vec(R"(Vec<([0-9]+),(\w+)>)");
		//				std::smatch res;
		//				if (std::regex_search(name, res, reg_vec))
		//				{
		//					auto t = res[2].str();
		//					name = "Vec" + res[1].str() + (t == "uchar" ? 'c' : t[0]);
		//				}
		//				if (type->is_array)
		//					name = "Array<" + name + ">";
		//				if (type->tag == TypePointer)
		//					name += "*";
		//				vars.push_back({ name, id, 0 });
		//				std::regex reg("\\b" + out->name + "\\b");
		//				function_code = std::regex_replace(function_code, reg, id);
		//			}
		//			for (auto& in : n->inputs)
		//			{
		//				auto out = in->links[0];
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
		//					auto out = in->links[0];
		//					in_value = out ? var_id(out) : std::to_string(*(int*)in->data);
		//					lines.push_back({ -1, out_id + " = " + in_value + ";" });
		//				}
		//				{
		//					auto in = n->inputs[1].get();
		//					auto out = in->links[0];
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

	bpScene* bpScene::create(const wchar_t* filename)
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
		bp->_filename = filename;
		
		std::function<void(pugi::xml_node, bpNodePrivate*)> load_group;
		load_group = [&](pugi::xml_node n_group, bpNodePrivate* parent) {
			for (auto n_node : n_group.child("nodes"))
			{
				auto n = parent->_add_child(n_node.attribute("id").value(), (bpNodeType)n_node.attribute("type").as_int(), n_node.attribute("type_parameter").value(), (bpObjectRule)n_node.attribute("object_rule").as_int());
				if (n)
				{
					n->_pos = sto<Vec2f>(n_node.attribute("pos").value());
					for (auto n_data : n_node.child("datas"))
					{
						auto input = n->_find_input(n_data.attribute("name").value());
						auto type = input->_type;
						auto tag = type->_tag;
						if (tag == TypeEnumSingle || tag == TypeEnumMulti || tag == TypeData)
							type->unserialize(n_data.attribute("value").value(), input->_data);
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

				auto o = parent->_find_output(o_addr);
				auto i = parent->_find_input(i_addr);
				if (o && i)
				{
					if (!i->link_to(o))
						printf("link type mismatch: %s - > %s\n", o_addr.c_str(), i_addr.c_str());
				}
				else
					printf("cannot link: %s - > %s\n", o_addr.c_str(), i_addr.c_str());
			}
		};

		load_group(file_root.child("group"), bp->_root.get());

		printf("end loading bp: %s\n", s_filename.c_str());

		return bp;
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
			delta = _looper->_delta_time;
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

	struct FLAME_R(R_Print)
	{
		FLAME_RV(StringA, text, i);

		FLAME_FOUNDATION_EXPORTS void FLAME_RF(bp_update)()
		{
			printf("%s\n", text.v);
		}
	};
}

