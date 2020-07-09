#include <flame/serialize.h>
#include <flame/foundation/foundation.h>
#include <flame/foundation/typeinfo.h>
#include "blueprint_private.h"

namespace flame
{
	bpSlotPrivate::bpSlotPrivate(bpNodePrivate* node, bpSlotIO io, uint index, TypeInfo* type, const std::string& name, uint offset, const void* dv) :
		node(node),
		io(io),
		index(index),
		type(type),
		name(name),
		offset(offset)
	{
		user_data = nullptr;

		if (dv)
		{
			auto size = type->get_size();
			default_value = new char[size];
			memcpy(default_value, dv, size);
		}

		data = (char*)node->object + offset;

		if (io == bpSlotIn)
			links.push_back(nullptr);
	}

	bpSlotPrivate::bpSlotPrivate(bpNodePrivate* node, bpSlotIO io, uint index, VariableInfo* vi) :
		bpSlotPrivate(node, io, index, vi->get_type(), vi->get_name(), vi->get_offset(), vi->get_default_value())
	{
	}
	 
	bpSlotPrivate::~bpSlotPrivate()
	{
		delete[]default_value;
	}

	void bpSlotPrivate::set_data(const void* d)
	{
		//if (_setter)
		//	_setter->set(d);
		//else
			type->copy(d, data);
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
			if (!bp_can_link(type, target->type))
				return false;
		}

		{
			auto o = links[0];
			if (o)
			{
				erase_if(o->links, this);
				if (type->get_name() == std::string("ListenerHub"))
					(*(ListenerHub<void(Capture&)>**)data)->remove(listener);
			}
		}

		links[0] = target;
		if (target)
		{
			target->links.push_back(this);
			if (type->get_name() == std::string("ListenerHub"))
			{
				auto p = target->data;
				memcpy(data, &p, sizeof(void*));
				listener = (*(ListenerHub<void(Capture&)>**)data)->add([](Capture& c) {
					c.thiz<bpNode>()->update();
				}, Capture().set_thiz(node));
			}
		}

		if (!target && type->get_tag() == TypePointer)
			memset(data, 0, sizeof(void*));

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
		scene(scene),
		parent(parent),
		id(id),
		object_rule(object_rule),
		type(type)
	{
		user_data = nullptr;

		switch (type)
		{
		case bpNodeEnumSingle:
		{
			assert(object_rule == bpObjectEntity);
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
			auto size = udt->get_size();
			object = malloc(size);
			memset(object, 0, size);
			inputs.emplace_back(new bpSlotPrivate(this, bpSlotIn, 0, TypeInfo::get(TypeEnumSingle, type_parameter.c_str()), "in", offsetof(Dummy, in), nullptr));
			inputs.emplace_back(new bpSlotPrivate(this, bpSlotIn, 1, TypeInfo::get(TypeEnumSingle, type_parameter.c_str()), "chk", offsetof(Dummy, chk), nullptr));
			outputs.emplace_back(new bpSlotPrivate(this, bpSlotOut, 0, TypeInfo::get(TypeEnumSingle, type_parameter.c_str()), "out", offsetof(Dummy, out), nullptr));
			outputs.emplace_back(new bpSlotPrivate(this, bpSlotOut, 1, TypeInfo::get(TypeData, "float"), "res", offsetof(Dummy, res), nullptr));
			update_addr = f2v(&Dummy::update);
		}
			break;
		case bpNodeEnumMulti:
		{
			assert(object_rule == bpObjectEntity);
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
			object = malloc(size);
			memset(object, 0, size);
			inputs.emplace_back(new bpSlotPrivate(this, bpSlotIn, 0, TypeInfo::get(TypeEnumMulti, type_parameter.c_str()), "in", offsetof(Dummy, in), nullptr));
			inputs.emplace_back(new bpSlotPrivate(this, bpSlotIn, 1, TypeInfo::get(TypeEnumSingle, type_parameter.c_str()), "chk", offsetof(Dummy, chk), nullptr));
			outputs.emplace_back(new bpSlotPrivate(this, bpSlotOut, 0, TypeInfo::get(TypeEnumMulti, type_parameter.c_str()), "out", offsetof(Dummy, out), nullptr));
			outputs.emplace_back(new bpSlotPrivate(this, bpSlotOut, 1, TypeInfo::get(TypeData, "float"), "res", offsetof(Dummy, res), nullptr));
			update_addr = f2v(&Dummy::update);
		}
			break;
		case bpNodeVariable:
		{
			assert(object_rule == bpObjectEntity);
#pragma pack(1)
			struct Dummy
			{
				TypeInfo* type;

				void dtor()
				{
					auto p = var_end(&type);
					auto size = type->get_size();
					auto in = next_var(p, size);
					auto out = next_var(p, size);
					type->destruct(in);
					type->destruct(out);
				}

				void update()
				{
					auto p = var_end(&type);
					auto size = type->get_size();
					auto in = next_var(p, size);
					auto out = next_var(p, size);
					type->copy(in, out);
				}
			};
#pragma pack()
			auto type = TypeInfo::get(TypeData, type_parameter.c_str());
			auto type_size = type->get_size();

			auto size = sizeof(Dummy) + type_size * 2;
			object = malloc(size);
			memset(object, 0, size);
			inputs.emplace_back(new bpSlotPrivate(this, bpSlotIn, 0, type, "in", sizeof(Dummy), nullptr));
			outputs.emplace_back(new bpSlotPrivate(this, bpSlotOut, 0, type, "out", sizeof(Dummy) + type_size, nullptr));
			dtor_addr = f2v(&Dummy::dtor);
			update_addr = f2v(&Dummy::update);
			auto& obj = *(Dummy*)object;
			obj.type = type;
		}
			break;
		case bpNodeArray:
		{
			assert(object_rule == bpObjectEntity);
			auto sp = SUS::split(type_parameter, ',');
#pragma pack(1)
			struct Dummy
			{
				TypeInfo* type;
				uint length;

				void dtor()
				{
					auto p = var_end(&length);
					auto size = type->get_size();
					auto& out = *next_var<Array<int>>(p);
					for (auto i = 0; i < out.s; i++)
						type->destruct((char*)out.v + size * i);
					for (auto i = 0; i < length; i++)
						type->destruct(next_var(p, size));
					f_free(out.v);
				}

				void update()
				{
					auto p = var_end(&length);
					auto size = type->get_size();
					auto& out = *next_var<Array<int>>(p);
					if (out.s != length)
					{
						out.s = length;
						auto m_size = size * length;
						f_free(out.v);
						out.v = (int*)f_malloc(m_size);
						memset(out.v, 0, m_size);
					}
					for (auto i = 0; i < length; i++)
					{
						type->copy(next_var(p, size), (char*)out.v + size * i);
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
			auto type = TypeInfo::get(tag, base_name.c_str());
			uint type_size = type->get_size();
			auto length = stoi(sp[0]);
			auto size = sizeof(Dummy) + sizeof(Array<int>) + type_size * length;
			object = malloc(size);
			memset(object, 0, size);
			outputs.emplace_back(new bpSlotPrivate(this, bpSlotOut, 0, TypeInfo::get(TypeArrayOfData, type_name.c_str()), "out", sizeof(Dummy), nullptr));
			for (auto i = 0; i < length; i++)
				inputs.emplace_back(new bpSlotPrivate(this, bpSlotIn, i, type, std::to_string(i), sizeof(Dummy) + sizeof(Array<int>) + type_size * i, nullptr));
			dtor_addr = f2v(&Dummy::dtor);
			update_addr = f2v(&Dummy::update);
			auto& obj = *(Dummy*)object;
			obj.type = type;
			obj.length = length;
		}
			break;
		case bpNodeGroup:
		{
			assert(object_rule == bpObjectEntity);
#pragma pack(1)
			struct Dummy
			{
				ListenerHub<void()>* signal;
			};
#pragma pack()
			auto size = sizeof(Dummy);
			object = malloc(size);
			memset(object, 0, size);
			inputs.emplace_back(new bpSlotPrivate(this, bpSlotIn, 0, TypeInfo::get(TypePointer, "ListenerHub"), "signal", offsetof(Dummy, signal), nullptr));
		}
			break;
		case bpNodeUdt:
		{
			udt = find_udt(type_parameter.c_str());

			if (!udt)
			{
				assert(0);
				printf("add node: udt not found: %s\n", type_parameter.c_str());
			}

			library_address = (void*)udt->get_library()->get_address();

			if (object_rule == bpObjectEntity)
			{
				auto size = udt->get_size();
				object = malloc(size);
				memset(object, 0, size);

				{
					auto f = udt->find_function("ctor");
					if (f && f->check(TypeInfo::get(TypeData, "void"), nullptr))
						cmf(p2f<void(__Dummy__::*)()>((char*)library_address + (uint)f->get_rva()), object);
				}

				{
					auto f = udt->find_function("dtor");
					if (f && f->check(TypeInfo::get(TypeData, "void"), nullptr))
						dtor_addr = (char*)library_address + (uint)f->get_rva();
				}

				{
					auto f = udt->find_function("bp_update");
					assert(f && f->check(TypeInfo::get(TypeData, "void"), nullptr));
					update_addr = (char*)library_address + (uint)f->get_rva();
				}

				auto vars_count = udt->get_variables_count();
				for (auto i = 0; i < vars_count; i++)
				{
					auto v = udt->get_variable(i);
					if (v->get_flags() & VariableFlagOutput)
						outputs.emplace_back(new bpSlotPrivate(this, bpSlotOut, outputs.size(), v));
					else
						inputs.emplace_back(new bpSlotPrivate(this, bpSlotIn, inputs.size(), v));
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

				if (object_rule == bpObjectRefRead)
				{
					auto vars_count = udt->get_variables_count();
					for (auto i = 0; i < vars_count; i++)
						outputs.emplace_back(new bpSlotPrivate(this, bpSlotOut, outputs.size(), udt->get_variable(i)));
				}
				else if (object_rule == bpObjectRefWrite)
				{
					auto vars_count = udt->get_variables_count();
					for (auto i = 0; i < vars_count; i++)
					{
						auto v = udt->get_variable(i);
						auto type = v->get_type();
						if (type->get_tag() != TypeData)
							continue;
						auto input = new bpSlotPrivate(this, bpSlotIn, inputs.size(), v);
						auto f_set = udt->find_function((std::string("set_") + v->get_name()).c_str());
						if (f_set)
						{
							auto f_set_addr = (char*)library_address + (uint)f_set->get_rva();
							//Setter* setter = nullptr;
							//switch (type->_name_hash) // TODO
							//{
							//case FLAME_CHASH("bool"):
							//	setter = new Setter_t<bool>;
							//	break;
							//case FLAME_CHASH("int"):
							//	setter = new Setter_t<int>;
							//	break;
							//case FLAME_CHASH("flame::Vec<2,int>"):
							//	setter = new Setter_t<Vec2i>;
							//	break;
							//case FLAME_CHASH("flame::Vec<3,int>"):
							//	setter = new Setter_t<Vec3i>;
							//	break;
							//case FLAME_CHASH("flame::Vec<4,int>"):
							//	setter = new Setter_t<Vec4i>;
							//	break;
							//case FLAME_CHASH("uint"):
							//	setter = new Setter_t<uint>;
							//	break;
							//case FLAME_CHASH("flame::Vec<2,uint>"):
							//	setter = new Setter_t<Vec2u>;
							//	break;
							//case FLAME_CHASH("flame::Vec<3,uint>"):
							//	setter = new Setter_t<Vec3u>;
							//	break;
							//case FLAME_CHASH("flame::Vec<4,unt>"):
							//	setter = new Setter_t<Vec4u>;
							//	break;
							//case FLAME_CHASH("float"):
							//	setter = new Setter_t<float>;
							//	break;
							//case FLAME_CHASH("flame::Vec<2,float>"):
							//	setter = new Setter_t<Vec2f>;
							//	break;
							//case FLAME_CHASH("flame::Vec<3,float>"):
							//	setter = new Setter_t<Vec3f>;
							//	break;
							//case FLAME_CHASH("flame::Vec<4,float>"):
							//	setter = new Setter_t<Vec4f>;
							//	break;
							//case FLAME_CHASH("uchar"):
							//	setter = new Setter_t<uchar>;
							//	break;
							//case FLAME_CHASH("flame::Vec<2,uchar>"):
							//	setter = new Setter_t<Vec2c>;
							//	break;
							//case FLAME_CHASH("flame::Vec<3,uchar>"):
							//	setter = new Setter_t<Vec3c>;
							//	break;
							//case FLAME_CHASH("flame::Vec<4,uchar>"):
							//	setter = new Setter_t<Vec4c>;
							//	break;
							//}
							//setter->o = object;
							//setter->f = f_set_addr;
							//input->_setter = setter;
						}
						memcpy(input->default_value, input->data, input->type->get_size());
						inputs.emplace_back(input);
					}
				}
			}
		}
			break;
		}
	}

	bpNodePrivate::~bpNodePrivate()
	{
		if (object_rule == bpObjectEntity)
		{
			if (dtor_addr)
				cmf(p2f<void(__Dummy__::*)()>(dtor_addr), object);
			free(object);
		}
	}

	bool bpNodePrivate::set_id__(const std::string& _id)
	{
		if (_id.empty())
			return false;
		if (id == _id)
			return true;
		if (parent->find_child__(_id))
			return false;
		id = _id;
		return true;
	}

	bpSlotPrivate* bpNodePrivate::_find_input(const std::string& name) const
	{
		for (auto& in : inputs)
		{
			if (in->name == name)
				return in.get();
		}
		return nullptr;
	}

	bpSlotPrivate* bpNodePrivate::_find_output(const std::string& name) const
	{
		for (auto& out : outputs)
		{
			if (out->name == name)
				return out.get();
		}
		return nullptr;
	}

	static void get_order(bpNodePrivate* n, uint& order)
	{
		if (n->order != 0xffffffff)
			return;
		for (auto& i : n->inputs)
		{
			auto o = i->links[0];
			if (o)
			{
				auto n = o->node;
				if (n)
					get_order(n, order);
			}
		}
		n->order = order++;
	}

	static void build_update_list(bpNodePrivate* n)
	{
		for (auto& c : n->children)
			c->order = 0xffffffff;
		auto order = 0U;
		for (auto& c : n->children)
			get_order(c.get(), order);
		n->update_list.clear();
		for (auto& c : n->children)
		{
			std::vector<bpNodePrivate*>::iterator it;
			for (it = n->update_list.begin(); it != n->update_list.end(); it++)
			{
				if (c->order < (*it)->order)
					break;
			}
			n->update_list.emplace(it, c.get());
		}
		n->need_rebuild_update_list = false;
	}

	static bool check_or_create_id(bpNodePrivate* parent, std::string& id)
	{
		if (!id.empty())
		{
			if (parent->find_child__(id))
				return false;
		}
		else
		{
			id = std::to_string(::rand());
			while (parent->find_child__(id))
				id = std::to_string(::rand());
		}
		return true;
	}

	bpNodePrivate* bpNodePrivate::add_child__(const std::string& id, bpNodeType type, const std::string& type_parameter, bpObjectRule object_rule)
	{
		auto _id = id;
		if (!check_or_create_id(this, _id))
		{
			printf("cannot add node, id repeated\n");
			return nullptr;
		}

		auto n = new bpNodePrivate(scene, this, _id, type, type_parameter, object_rule);
		n->guid = generate_guid();
		children.emplace_back(n);

		need_rebuild_update_list = true;

		return n;
	}
	void bpNodePrivate::remove_child__(bpNodePrivate* n)
	{
		auto it = std::find_if(children.begin(), children.end(), [&](const auto& t) {
			return t.get() == n;
		});
		if (it != children.end())
		{
			for (auto& in : n->inputs)
			{
				auto o = in->links[0];
				if (o)
				{
					for (auto it = o->links.begin(); it != o->links.end(); it++)
					{
						if (*it == in.get())
						{
							o->links.erase(it);
							break;
						}
					}
				}
			}
			for (auto& o : n->outputs)
			{
				for (auto l : o->links)
					l->links[0] = nullptr;
			}
			children.erase(it);

			need_rebuild_update_list = true;
		}
	}
	bpNodePrivate* bpNodePrivate::find_child__(const std::string& name) const
	{
		for (auto& n : children)
		{
			if (n->id == name)
				return n.get();
		}
		return nullptr;
	}
	bpNodePrivate* bpNodePrivate::find_child__(const Guid& guid) const
	{
		for (auto& n : children)
		{
			if (memcmp(&n->guid, &guid, sizeof(Guid)) == 0)
				return n.get();
			auto res = n->find_child__(guid);
			if (res)
				return res;
		}
		return nullptr;
	}

	void bpNodePrivate::update()
	{
		for (auto& in : inputs)
		{
			auto out = in->links[0];
			if (out)
			{
				if (out->type->get_tag() == TypeData && in->type->get_tag() == TypePointer)
					memcpy(in->data, &out->data, sizeof(void*));
				else
				{
					//if (in->_setter)
					//	in->_setter->set(out->data);
					//else
					memcpy(in->data, out->data, in->type->get_size());
				}
			}
		}

		if (update_addr)
			cmf(p2f<void(__Dummy__::*)()>(update_addr), object);

		if (need_rebuild_update_list)
			build_update_list(this);

		for (auto n : update_list)
			n->_update();
	}

	bpScenePrivate::bpScenePrivate()
	{
		time = 0.f;

		root.reset(new bpNodePrivate(this, nullptr, "root", bpNodeGroup, "", bpObjectEntity));
	}

	static float bp_time = 0.f;

	void bpScenePrivate::update()
	{
		bp_time = time;

		root->update();

		time += get_looper()->get_delta_time();
	}

	void bpScenePrivate::save()
	{
		pugi::xml_document file;
		auto file_root = file.append_child("BP");

		std::function<void(pugi::xml_node, bpNodePrivate*)> save_group;
		save_group = [&](pugi::xml_node n_group, bpNodePrivate* parent) {
			auto n_nodes = n_group.append_child("nodes");
			for (auto& n : parent->children)
			{
				auto n_node = n_nodes.append_child("node");
				n_node.append_attribute("id").set_value(n->id.c_str());
				n_node.append_attribute("type").set_value(n->type);
				n_node.append_attribute("type_parameter").set_value(n->type_parameter.c_str());
				n_node.append_attribute("object_rule").set_value(n->object_rule);
				n_node.append_attribute("pos").set_value(to_string(n->pos).c_str());

				pugi::xml_node n_datas;
				for (auto& in : n->inputs)
				{
					if (in->links[0])
						continue;
					auto type = in->type;
					if (type->get_tag() != TypePointer)
					{
						if (in->default_value && memcmp(in->default_value, in->data, in->type->get_size()) == 0)
							continue;
						if (!n_datas)
							n_datas = n_node.append_child("datas");
						auto n_data = n_datas.append_child("data");
						n_data.append_attribute("name").set_value(in->name.c_str());
						n_data.append_attribute("value").set_value(type->serialize_s(in->data).c_str());
					}
				}

				if (!n->children.empty())
					save_group(n_group.append_child("group"), n.get());
			}

			auto n_links = n_group.append_child("links");
			for (auto& n : parent->children)
			{
				for (auto& in : n->inputs)
				{
					auto out = in->links[0];
					if (out)
					{
						auto n_link = n_links.append_child("link");
						n_link.append_attribute("out").set_value((out->node->id + "." + out->name).c_str());
						n_link.append_attribute("in").set_value((n->id + "." + in->name).c_str());
					}
				}
			}
		};


		save_group(file_root.append_child("group"), root.get());

		file.save_file(filename.c_str());

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
		bp->filename = filename;
		
		std::function<void(pugi::xml_node, bpNodePrivate*)> load_group;
		load_group = [&](pugi::xml_node n_group, bpNodePrivate* parent) {
			for (auto n_node : n_group.child("nodes"))
			{
				auto n = parent->add_child__(n_node.attribute("id").value(), (bpNodeType)n_node.attribute("type").as_int(), n_node.attribute("type_parameter").value(), (bpObjectRule)n_node.attribute("object_rule").as_int());
				if (n)
				{
					n->pos = sto<Vec2f>(n_node.attribute("pos").value());
					for (auto n_data : n_node.child("datas"))
					{
						auto input = n->_find_input(n_data.attribute("name").value());
						auto type = input->type;
						auto tag = type->get_tag();
						if (tag == TypeEnumSingle || tag == TypeEnumMulti || tag == TypeData)
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

		load_group(file_root.child("group"), bp->root.get());

		printf("end loading bp: %s\n", s_filename.c_str());

		return bp;
	}

	struct FLAME_R(R_MakeVec2i)
	{
		FLAME_RV(int, x, i);
		FLAME_RV(int, y, i);

		FLAME_RV(Vec2i, out, o);

		FLAME_BLUEPRINT_EXPORTS void FLAME_RF(bp_update)()
		{
			out = Vec2i(x, y);
		}
	};

	struct FLAME_R(R_BreakVec2i)
	{
		FLAME_RV(Vec2i, in, i);

		FLAME_RV(int, x, o);
		FLAME_RV(int, y, o);

		FLAME_BLUEPRINT_EXPORTS void FLAME_RF(bp_update)()
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

		FLAME_BLUEPRINT_EXPORTS void FLAME_RF(bp_update)()
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

		FLAME_BLUEPRINT_EXPORTS void FLAME_RF(bp_update)()
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

		FLAME_BLUEPRINT_EXPORTS void FLAME_RF(bp_update)()
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

		FLAME_BLUEPRINT_EXPORTS void FLAME_RF(bp_update)()
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

		FLAME_BLUEPRINT_EXPORTS void FLAME_RF(bp_update)()
		{
			out = Vec2u(x, y);
		}
	};

	struct FLAME_R(R_BreakVec2u)
	{
		FLAME_RV(Vec2u, in, i);

		FLAME_RV(uint, x, o);
		FLAME_RV(uint, y, o);

		FLAME_BLUEPRINT_EXPORTS void FLAME_RF(bp_update)()
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

		FLAME_BLUEPRINT_EXPORTS void FLAME_RF(bp_update)()
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

		FLAME_BLUEPRINT_EXPORTS void FLAME_RF(bp_update)()
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

		FLAME_BLUEPRINT_EXPORTS void FLAME_RF(bp_update)()
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

		FLAME_BLUEPRINT_EXPORTS void FLAME_RF(bp_update)()
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

		FLAME_BLUEPRINT_EXPORTS void FLAME_RF(bp_update)()
		{
			out = Vec2f(x, y);
		}
	};

	struct FLAME_R(R_BreakVec2f)
	{
		FLAME_RV(Vec2f, in, i);

		FLAME_RV(float, x, o);
		FLAME_RV(float, y, o);

		FLAME_BLUEPRINT_EXPORTS void FLAME_RF(bp_update)()
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

		FLAME_BLUEPRINT_EXPORTS void FLAME_RF(bp_update)()
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

		FLAME_BLUEPRINT_EXPORTS void FLAME_RF(bp_update)()
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

		FLAME_BLUEPRINT_EXPORTS void FLAME_RF(bp_update)()
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

		FLAME_BLUEPRINT_EXPORTS void FLAME_RF(bp_update)()
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

		FLAME_BLUEPRINT_EXPORTS void FLAME_RF(bp_update)()
		{
			out = Vec2c(x, y);
		}
	};

	struct FLAME_R(R_BreakVec2c)
	{
		FLAME_RV(Vec2c, in, i);

		FLAME_RV(uchar, x, o);
		FLAME_RV(uchar, y, o);

		FLAME_BLUEPRINT_EXPORTS void FLAME_RF(bp_update)()
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

		FLAME_BLUEPRINT_EXPORTS void FLAME_RF(bp_update)()
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

		FLAME_BLUEPRINT_EXPORTS void FLAME_RF(bp_update)()
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

		FLAME_BLUEPRINT_EXPORTS void FLAME_RF(bp_update)()
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

		FLAME_BLUEPRINT_EXPORTS void FLAME_RF(bp_update)()
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

		FLAME_BLUEPRINT_EXPORTS void FLAME_RF(bp_update)()
		{
			out = a + b;
		}
	};

	struct FLAME_R(R_Multiple)
	{
		FLAME_RV(float, a, i);
		FLAME_RV(float, b, i);

		FLAME_RV(float, out, o);

		FLAME_BLUEPRINT_EXPORTS void FLAME_RF(bp_update)()
		{
			out = a * b;
		}
	};

	struct FLAME_R(R_Time)
	{
		FLAME_RV(float, delta, o);
		FLAME_RV(float, total, o);

		FLAME_BLUEPRINT_EXPORTS void FLAME_RF(bp_update)()
		{
			delta = get_looper()->get_delta_time();
			total = bp_time;
		}
	};

	struct FLAME_R(R_Sin)
	{
		FLAME_RV(float, t, i);

		FLAME_RV(float, out, o);

		FLAME_BLUEPRINT_EXPORTS void FLAME_RF(bp_update)()
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

		FLAME_BLUEPRINT_EXPORTS void FLAME_RF(bp_update)()
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

		FLAME_BLUEPRINT_EXPORTS void FLAME_RF(bp_update)()
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

		FLAME_BLUEPRINT_EXPORTS void FLAME_RF(bp_update)()
		{
			out = v + min(abs(target - v), step) * sign(target - v);
		}
	};

	struct FLAME_R(R_Print)
	{
		FLAME_RV(StringA, text, i);

		FLAME_BLUEPRINT_EXPORTS void FLAME_RF(bp_update)()
		{
			printf("%s\n", text.v);
		}
	};
}

