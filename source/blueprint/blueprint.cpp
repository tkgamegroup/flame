#include "blueprint_private.h"

#include <functional>
#include <pugixml.hpp>

namespace flame
{
	bpSlotPrivate::bpSlotPrivate(bpNodePrivate* node, bpSlotIO io, uint index, TypeInfo* type, std::string_view name, uint offset, const void* dv) :
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
		//	(*_setter)->set(d);
		//else
			type->copy(data, d);
	}

	bool bpSlotPrivate::link_to(bpSlotPtr target)
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
				std::erase_if(o->links, [&](const auto& i) {
					return i == this;
				});
				//if (type->get_name() == std::string("ListenerHub")) TODO
				//	(*(ListenerHub<void(Capture&)>**)data)->remove(listener);
			}
		}

		links[0] = target;
		if (target)
		{
			target->links.push_back(this);
			//if (type->get_name() == std::string("ListenerHub")) TODO
			//{
			//	auto p = target->data;
			//	memcpy(data, &p, sizeof(void*));
			//	listener = (*(ListenerHub<void(Capture&)>**)data)->add([](Capture& c) {
			//		c.thiz<bpNode>()->update();
			//	}, Capture().set_thiz(node));
			//}
		}

		if (!target && type->get_tag() == TagPointer)
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

	bpNodePrivate::bpNodePrivate(bpScenePrivate* scene, bpNodePrivate* parent, std::string_view id, bpNodeType type, std::string_view type_parameter, bpObjectRule object_rule) :
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
			inputs.emplace_back(new bpSlotPrivate(this, bpSlotIn, 0, TypeInfo::get(TagEnumSingle, type_parameter), "in", offsetof(Dummy, in), nullptr));
			inputs.emplace_back(new bpSlotPrivate(this, bpSlotIn, 1, TypeInfo::get(TagEnumSingle, type_parameter), "chk", offsetof(Dummy, chk), nullptr));
			outputs.emplace_back(new bpSlotPrivate(this, bpSlotOut, 0, TypeInfo::get(TagEnumSingle, type_parameter), "out", offsetof(Dummy, out), nullptr));
			outputs.emplace_back(new bpSlotPrivate(this, bpSlotOut, 1, TypeInfo::get(TagD, "float"), "res", offsetof(Dummy, res), nullptr));
			update_addr = f2a(&Dummy::update);
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
			inputs.emplace_back(new bpSlotPrivate(this, bpSlotIn, 0, TypeInfo::get(TagEnumMulti, type_parameter), "in", offsetof(Dummy, in), nullptr));
			inputs.emplace_back(new bpSlotPrivate(this, bpSlotIn, 1, TypeInfo::get(TagEnumSingle, type_parameter), "chk", offsetof(Dummy, chk), nullptr));
			outputs.emplace_back(new bpSlotPrivate(this, bpSlotOut, 0, TypeInfo::get(TagEnumMulti, type_parameter), "out", offsetof(Dummy, out), nullptr));
			outputs.emplace_back(new bpSlotPrivate(this, bpSlotOut, 1, TypeInfo::get(TagD, "float"), "res", offsetof(Dummy, res), nullptr));
			update_addr = f2a(&Dummy::update);
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
					type->destroy(in, false);
					type->destroy(out, false);
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
			auto type = TypeInfo::get(TagD, type_parameter);
			auto type_size = type->get_size();

			auto size = sizeof(Dummy) + type_size * 2;
			object = malloc(size);
			memset(object, 0, size);
			inputs.emplace_back(new bpSlotPrivate(this, bpSlotIn, 0, type, "in", sizeof(Dummy), nullptr));
			outputs.emplace_back(new bpSlotPrivate(this, bpSlotOut, 0, type, "out", sizeof(Dummy) + type_size, nullptr));
			dtor_addr = f2a(&Dummy::dtor);
			update_addr = f2a(&Dummy::update);
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
						type->destroy((char*)out.v + size * i, false);
					for (auto i = 0; i < length; i++)
						type->destroy(next_var(p, size), false);
					free(out.v);
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
						free(out.v);
						out.v = (int*)malloc(m_size);
						memset(out.v, 0, m_size);
					}
					for (auto i = 0; i < length; i++)
					{
						type->copy(next_var(p, size), (char*)out.v + size * i);
					}
				}
			};
#pragma pack()
			auto tag = TagD;
			auto type_name = sp[1];
			auto base_name = type_name;
			if (type_name.back() == '*')
			{
				base_name.erase(base_name.end() - 1);
				tag = TagPointer;
			}
			auto type = TypeInfo::get(tag, base_name);
			uint type_size = type->get_size();
			auto length = stoi(sp[0]);
			auto size = sizeof(Dummy) + sizeof(Array<int>) + type_size * length;
			object = malloc(size);
			memset(object, 0, size);
			outputs.emplace_back(new bpSlotPrivate(this, bpSlotOut, 0, TypeInfo::get(TypeArrayOfData, type_name), "out", sizeof(Dummy), nullptr));
			for (auto i = 0; i < length; i++)
				inputs.emplace_back(new bpSlotPrivate(this, bpSlotIn, i, type, std::to_string(i), sizeof(Dummy) + sizeof(Array<int>) + type_size * i, nullptr));
			dtor_addr = f2a(&Dummy::dtor);
			update_addr = f2a(&Dummy::update);
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
				//ListenerHub<void()>* signal; TODO
			};
#pragma pack()
			auto size = sizeof(Dummy);
			object = malloc(size);
			memset(object, 0, size);
			//inputs.emplace_back(new bpSlotPrivate(this, bpSlotIn, 0, TypeInfo::get(TagPointer, "ListenerHub"), "signal", offsetof(Dummy, signal), nullptr)); TODO
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
					if (f && f->check(TypeInfo::get(TagD, ""), nullptr))
						a2f<void(*)(void*)>(f->get_address())(object);
				}

				{
					auto f = udt->find_function("dtor");
					if (f && f->check(TypeInfo::get(TagD, ""), nullptr))
						dtor_addr = f->get_address();
				}

				{
					auto f = udt->find_function("bp_update");
					assert(f && f->check(TypeInfo::get(TagD, ""), nullptr));
					update_addr = f->get_address();
				}

				auto vars_count = udt->get_variables_count();
				for (auto i = 0; i < vars_count; i++)
				{
					auto v = udt->get_variable(i);
					auto meta = v->get_meta();
					if (meta->has_token("o"))
						outputs.emplace_back(new bpSlotPrivate(this, bpSlotOut, outputs.size(), v));
					else if (meta->has_token("i"))
						inputs.emplace_back(new bpSlotPrivate(this, bpSlotIn, inputs.size(), v));
				}
			}
			else
			{
				// TODO
				//{
				//	auto f = (*udt)->find_function("get_linked_object");
				//	assert(f && check_function(f, ("P#" + udt->name.str()).c_str(), {}));
				//	object = a2f<F_vp_v>((char*)library + (uint)f->rva)();
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
						if (type->get_tag() != TagD)
							continue;
						auto input = new bpSlotPrivate(this, bpSlotIn, inputs.size(), v);
						auto f_set = udt->find_function((std::string("set_") + v->get_name()).c_str());
						if (f_set)
						{
							auto f_set_addr = f_set->get_address();
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
				a2f<void(*)(void*)>(dtor_addr)(object);
			free(object);
		}
	}

	bool bpNodePrivate::set_id(std::string_view _id)
	{
		if (_id.empty())
			return false;
		if (id == _id)
			return true;
		if (parent->find_child(_id))
			return false;
		id = _id;
		return true;
	}

	bpSlotPrivate* bpNodePrivate::find_input(std::string_view name) const
	{
		for (auto& in : inputs)
		{
			if (in->name == name)
				return in.get();
		}
		return nullptr;
	}

	bpSlotPrivate* bpNodePrivate::find_output(std::string_view name) const
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
			if (parent->find_child(id))
				return false;
		}
		else
		{
			id = std::to_string(rand());
			while (parent->find_child(id))
				id = std::to_string(rand());
		}
		return true;
	}

	bpNodePrivate* bpNodePrivate::add_child(std::string_view id, bpNodeType type, std::string_view type_parameter, bpObjectRule object_rule)
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

	void bpNodePrivate::remove_child(bpNodePtr n)
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

	bpNodePrivate* bpNodePrivate::find_child(std::string_view name) const
	{
		for (auto& n : children)
		{
			if (n->id == name)
				return n.get();
		}
		return nullptr;
	}

	bpNodePtr bpNodePrivate::find_child(const Guid& guid) const
	{
		for (auto& n : children)
		{
			if (memcmp(&n->guid, &guid, sizeof(Guid)) == 0)
				return n.get();
			auto res = n->find_child(guid);
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
				if (out->type->get_tag() == TagD && in->type->get_tag() == TagPointer)
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
			a2f<void(*)(void*)>(update_addr)(object);

		if (need_rebuild_update_list)
			build_update_list(this);

		for (auto n : update_list)
			n->update();
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

		time += looper().get_delta_time();
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
					if (type->get_tag() != TagPointer)
					{
						if (in->default_value && memcmp(in->default_value, in->data, in->type->get_size()) == 0)
							continue;
						if (!n_datas)
							n_datas = n_node.append_child("datas");
						auto n_data = n_datas.append_child("data");
						n_data.append_attribute("name").set_value(in->name.c_str());
						std::string str;
						type->serialize(&str, in->data);
						n_data.append_attribute("value").set_value(str.c_str());
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
		//struct Piece
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
		//std::vector<Piece> pieces;
		//std::vector<Line> lines;
		//for (auto n : bp->update_list)
		//{
		//	auto var_id = [](bpSlotPrivate* s) {
		//		auto n = s->node;
		//		if (n->node_type != NodeReal)
		//			return std::string((*n)->(*udt)->link_name()) + "->" + (s->setter ? "set_" : "") + s->name;
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
		//				if ((*type)->tag() == TagPointer || (in->default_value && memcmp(in->default_value, in->data, in->size) == 0))
		//					continue;
		//				value = (*type)->serialize(in->data);
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
		//			auto f = (*n)->(*udt)->find_function("bp_update");
		//			assert(f && check_function(f, "D#void", {}));
		//			std::string function_code = (*f)->code();
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
		//				if (type->tag == TagPointer)
		//					name += "*";
		//				pieces.push_back({ name, id, 0 });
		//				std::regex reg("\\b" + out->name + "\\b");
		//				function_code = std::regex_replace(function_code, reg, id);
		//			}
		//			for (auto& in : n->inputs)
		//			{
		//				auto out = in->links[0];
		//				std::string value;
		//				if (!out)
		//					value = (*in)->(*type)->serialize(in->data);
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
		//				pieces.push_back({ "uint", out_id, 0 });
		//				pieces.push_back({ "float", res_id, 0 });
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

		//for (auto i = 0; i < pieces.size(); i++)
		//{
		//	std::regex reg(R"(^)" + pieces[i].name + R"( = (.*);)");
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
		//		auto& v = pieces[l.assign_var_idx];
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
		//for (auto& v : pieces)
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
		//		h_file << pieces[l.assign_var_idx].name + " = " + l.content + ";\n";
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
			printf("bp file does not exist, abort: %s\n", s_filename.c_str());
			printf("end loading bp: %s\n", s_filename.c_str());
			return nullptr;
		}

		auto bp = new bpScenePrivate();
		bp->filename = filename;
		
		std::function<void(pugi::xml_node, bpNodePrivate*)> load_group;
		load_group = [&](pugi::xml_node n_group, bpNodePrivate* parent) {
			for (auto n_node : n_group.child("nodes"))
			{
				auto n = parent->add_child(n_node.attribute("id").value(), (bpNodeType)n_node.attribute("type").as_int(), n_node.attribute("type_parameter").value(), (bpObjectRule)n_node.attribute("object_rule").as_int());
				if (n)
				{
					n->pos = sto<vec2>(n_node.attribute("pos").value());
					for (auto n_data : n_node.child("datas"))
					{
						auto input = n->find_input(n_data.attribute("name").value());
						auto type = input->type;
						auto tag = type->get_tag();
						if (tag == TagEnumSingle || tag == TagEnumMulti || tag == TagD)
							type->unserialize(input->data, n_data.attribute("value").value());
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

		load_group(file_root.child("group"), bp->root.get());

		printf("end loading bp: %s\n", s_filename.c_str());

		return bp;
	}

	struct R_MakeVec2i // R
	{
		int x; // R i
		int y; // R i

		ivec2 out; // R o

		FLAME_BLUEPRINT_EXPORTS void bp_update() // R code
		{
			out = ivec2(x, y);
		}
	};

	struct R_BreakVec2i // R
	{
		ivec2 in; // R i

		int x; // R o
		int y; // R o

		FLAME_BLUEPRINT_EXPORTS void bp_update() // R code
		{
			x = in[0];
			y = in[1];
		}
	};

	struct R_MakeVec3i // R
	{
		int x; // R i
		int y; // R i
		int z; // R i

		ivec3 out; // R o

		FLAME_BLUEPRINT_EXPORTS void bp_update() // R code
		{
			out = ivec3(x, y, z);
		}
	};

	struct R_BreakVec3i // R
	{
		ivec3 in; // R i

		int x; // R o
		int y; // R o
		int z; // R o

		FLAME_BLUEPRINT_EXPORTS void bp_update() // R code
		{
			x = in[0];
			y = in[1];
			z = in[2];
		}
	};

	struct R_MakeVec4i // R
	{
		int x; // R i
		int y; // R i
		int z; // R i
		int w; // R i

		ivec4 out; // R o

		FLAME_BLUEPRINT_EXPORTS void bp_update() // R code
		{
			out = ivec4(x, y, z, w);
		}
	};

	struct R_BreakVec4i // R
	{
		ivec4 in; // R i

		int x; // R o
		int y; // R o
		int z; // R o
		int w; // R o

		FLAME_BLUEPRINT_EXPORTS void bp_update() // R code
		{
			x = in[0];
			y = in[1];
			z = in[2];
			w = in[3];
		}
	};

	struct R_MakeVec2u // R
	{
		uint x; // R i
		uint y; // R i

		uvec2 out; // R o

		FLAME_BLUEPRINT_EXPORTS void bp_update() // R code
		{
			out = uvec2(x, y);
		}
	};

	struct R_BreakVec2u // R
	{
		uvec2 in; // R i

		uint x; // R o
		uint y; // R o

		FLAME_BLUEPRINT_EXPORTS void bp_update() // R code
		{
			x = in[0];
			y = in[1];
		}
	};

	struct R_MakeVec3u // R
	{
		uint x; // R i
		uint y; // R i
		uint z; // R i

		uvec3 out; // R o

		FLAME_BLUEPRINT_EXPORTS void bp_update() // R code
		{
			out = uvec3(x, y, z);
		}
	};

	struct R_BreakVec3u // R
	{
		uvec3 in; // R i

		uint x; // R o
		uint y; // R o
		uint z; // R o

		FLAME_BLUEPRINT_EXPORTS void bp_update() // R code
		{
			x = in[0];
			y = in[1];
			z = in[2];
		}
	};

	struct R_MakeVec4u // R
	{
		uint x; // R i
		uint y; // R i
		uint z; // R i
		uint w; // R i

		uvec4 out; // R o

		FLAME_BLUEPRINT_EXPORTS void bp_update() // R code
		{
			out = uvec4(x, y, z, w);
		}
	};

	struct R_BreakVec4u // R
	{
		uvec4 in; // R i

		uint x; // R o
		uint y; // R o
		uint z; // R o
		uint w; // R o

		FLAME_BLUEPRINT_EXPORTS void bp_update() // R code
		{
			x = in[0];
			y = in[1];
			z = in[2];
			w = in[3];
		}
	};

	struct R_MakeVec2f // R
	{
		float x; // R i
		float y; // R i

		vec2 out; // R o

		FLAME_BLUEPRINT_EXPORTS void bp_update() // R code
		{
			out = vec2(x, y);
		}
	};

	struct R_BreakVec2f // R
	{
		vec2 in; // R i

		float x; // R o
		float y; // R o

		FLAME_BLUEPRINT_EXPORTS void bp_update() // R code
		{
			x = in[0];
			y = in[1];
		}
	};

	struct R_MakeVec3f // R
	{
		float x; // R i
		float y; // R i
		float z; // R i

		vec3 out; // R o

		FLAME_BLUEPRINT_EXPORTS void bp_update() // R code
		{
			out = vec3(x, y, z);
		}
	};

	struct R_BreakVec3f // R
	{
		vec3 in; // R i

		float x; // R o
		float y; // R o
		float z; // R o

		FLAME_BLUEPRINT_EXPORTS void bp_update() // R code
		{
			x = in[0];
			y = in[1];
			z = in[2];
		}
	};

	struct R_MakeVec4f // R
	{
		float x; // R i
		float y; // R i
		float z; // R i
		float w; // R i

		vec4 out; // R o

		FLAME_BLUEPRINT_EXPORTS void bp_update() // R code
		{
			out = vec4(x, y, z, w);
		}
	};

	struct R_BreakVec4f // R
	{
		vec4 in; // R i

		float x; // R o
		float y; // R o
		float z; // R o
		float w; // R o

		FLAME_BLUEPRINT_EXPORTS void bp_update() // R code
		{
			x = in[0];
			y = in[1];
			z = in[2];
			w = in[3];
		}
	};

	struct R_MakeVec2c // R
	{
		uchar x; // R i
		uchar y; // R i

		cvec2 out; // R o

		FLAME_BLUEPRINT_EXPORTS void bp_update() // R code
		{
			out = cvec2(x, y);
		}
	};

	struct R_BreakVec2c // R
	{
		cvec2 in; // R i

		uchar x; // R o
		uchar y; // R o

		FLAME_BLUEPRINT_EXPORTS void bp_update() // R code
		{
			x = in[0];
			y = in[1];
		}
	};

	struct R_MakeVec3c // R
	{
		uchar x; // R i
		uchar y; // R i
		uchar z; // R i

		cvec3 out; // R o

		FLAME_BLUEPRINT_EXPORTS void bp_update() // R code
		{
			out = cvec3(x, y, z);
		}
	};

	struct R_BreakVec3c // R
	{
		cvec3 in; // R i

		uchar x; // R o
		uchar y; // R o
		uchar z; // R o

		FLAME_BLUEPRINT_EXPORTS void bp_update() // R code
		{
			x = in[0];
			y = in[1];
			z = in[2];
		}
	};

	struct R_MakeVec4c // R
	{
		uchar x; // R i
		uchar y; // R i
		uchar z; // R i
		uchar w; // R i

		cvec4 out; // R o

		FLAME_BLUEPRINT_EXPORTS void bp_update() // R code
		{
			out = cvec4(x, y, z, w);
		}
	};

	struct R_BreakVec4c // R
	{
		cvec4 in; // R i

		uchar x; // R o
		uchar y; // R o
		uchar z; // R o
		uchar w; // R o

		FLAME_BLUEPRINT_EXPORTS void bp_update() // R code
		{
			x = in[0];
			y = in[1];
			z = in[2];
			w = in[3];
		}
	};

	struct R_Add // R
	{
		float a; // R i
		float b; // R i

		float out; // R o

		FLAME_BLUEPRINT_EXPORTS void bp_update() // R code
		{
			out = a + b;
		}
	};

	struct R_Multiple // R
	{
		float a; // R i
		float b; // R i

		float out; // R o

		FLAME_BLUEPRINT_EXPORTS void bp_update() // R code
		{
			out = a * b;
		}
	};

	struct R_Time // R
	{
		float delta; // R o
		float total; // R o

		FLAME_BLUEPRINT_EXPORTS void bp_update() // R code
		{
			delta = looper().get_delta_time();
			total = bp_time;
		}
	};

	struct R_Sin // R
	{
		float t; // R i

		float out; // R o

		FLAME_BLUEPRINT_EXPORTS void bp_update() // R code
		{
			out = sin(t * M_PI / 180.f);
		}
	};

	struct R_Linear1d // R
	{
		float a; // R i
		float b; // R i
		float t; // R i

		float out; // R o

		FLAME_BLUEPRINT_EXPORTS void bp_update() // R code
		{
			out = a + (b - a) * clamp(t, 0.f, 1.f);
		}
	};

	struct R_Linear2d // R
	{
		vec2 a; // R i
		vec2 b; // R i
		float t; // R i

		vec2 out; // R o

		FLAME_BLUEPRINT_EXPORTS void bp_update() // R code
		{
			out = a + (b - a) * clamp(t, 0.f, 1.f);
		}
	};

	struct R_Trace // R
	{
		float target; // R i
		float step; // R i
		float v; // R i

		float out; // R o

		FLAME_BLUEPRINT_EXPORTS void bp_update() // R code
		{
			out = v + min(abs(target - v), step) * sign(target - v);
		}
	};

	struct R_Print // R
	{
		StringA text; // R i

		FLAME_BLUEPRINT_EXPORTS void bp_update() // R code
		{
			printf("%s\n", text.v);
		}
	};
}

