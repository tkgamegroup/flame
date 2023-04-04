#pragma once

#include "typeinfo.h"
#include "../xml.h"
#include "../json.h"

namespace flame
{
	struct ExcludeSpec
	{
		std::vector<std::pair<uint, uint>> excludes;

		inline bool skip(uint nh1, uint nh2) const
		{
			for (auto& e : excludes)
			{
				if (e.first == nh1 && e.second == nh2)
					return true;
			}
			return false;
		}
	};

#ifndef FLAME_NO_XML
	struct SerializeXmlSpec : ExcludeSpec
	{
		std::function<bool(const std::string& name, uint name_hash, TypeInfo* type, void* src, std::string& out)>	general_delegate;
		std::map<uint, std::function<std::string(void* src)>>														named_data_delegates;
		std::map<TypeInfo*, std::function<std::string(void* src)>>													typed_delegates;
		std::map<TypeInfo*, std::function<void(void* src, pugi::xml_node dst)>>										typed_obj_delegates;
	};

	struct UnserializeXmlSpec : ExcludeSpec
	{
		std::function<bool(const std::string& name, uint name_hash, TypeInfo* type, const std::string& src, void* dst)>	general_delegate;
		std::map<uint, std::function<void(const std::string& src, void* dst)>>											named_data_delegates;
		std::map<TypeInfo*, std::function<void(const std::string& src, void* dst)>>										typed_delegates;
		std::map<TypeInfo*, std::function<void* (pugi::xml_node src, void* dst_o)>>										typed_obj_delegates;
	};
#endif

	struct TextSerializeNode
	{
		std::string name;
		std::vector<TextSerializeNode> children;

		std::string value() const
		{
			return children.empty() ? name : (children[0].children.empty() ? children[0].name : "");
		}

		std::string value(std::string_view n) const
		{
			for (auto& c : children)
			{
				if (c.name == n)
					return c.value();
			}
			return "";
		}

		void set_value(const std::string& v)
		{
			children.emplace_back().name = v;
		}

		void add_value(const std::string& n, const std::string& v)
		{
			auto& c = children.emplace_back();
			c.name = n;
			c.set_value(v);
		}
	};

	struct SerializeTextSpec : ExcludeSpec
	{
		std::map<uint, std::function<TextSerializeNode(void* src)>>			named_delegates;
		std::map<TypeInfo*, std::function<TextSerializeNode(void* src)>>	typed_delegates;
		bool force_print_bar = false;
	};

	struct UnserializeTextSpec
	{
		std::map<uint, std::function<void* (const TextSerializeNode& src)>>			named_delegates;
		std::map<TypeInfo*, std::function<void* (const TextSerializeNode& src)>>	typed_delegates;
		std::vector<std::pair<std::string, std::string>>* out_default_defines = nullptr;
	};

	struct SerializeBinarySpec : ExcludeSpec
	{
		std::map<uint, std::function<void(void* src, std::ofstream& dst)>>		named_delegates;
		std::map<TypeInfo*, std::function<void(void* src, std::ofstream& dst)>> typed_delegates;
	};

	struct UnserializeBinarySpec : ExcludeSpec
	{
		std::map<uint, std::function<void(std::ofstream& src, void* dst)>>		named_delegates;
		std::map<TypeInfo*, std::function<void(std::ofstream& src, void* dst)>> typed_delegates;
	};

#ifndef FLAME_NO_XML
	inline void serialize_xml(const UdtInfo& ui, void* src, pugi::xml_node dst, const SerializeXmlSpec& spec = {});

	inline void serialize_xml(const UdtInfo& ui, uint offset, TypeInfo* type, const std::string& name, uint name_hash, const std::string& default_value, int getter_idx, void* src, pugi::xml_node dst, const SerializeXmlSpec& spec = {})
	{
		auto data_serialize = [&](TypeInfo* ti, void* p) {
			if (auto it = spec.typed_delegates.find(ti); it != spec.typed_delegates.end())
				return it->second(p);
			return ti->serialize(p);
		};

		switch (type->tag)
		{
		case TagE:
		case TagD:
			if (getter_idx != -1)
				type->call_getter(&ui.functions[getter_idx], src, nullptr);
			if (auto value = data_serialize(type, getter_idx == -1 ? (char*)src + offset : type->get_v()); value != default_value)
				dst.append_attribute(name.c_str()).set_value(value.c_str());
			break;
		case TagU:
			if (auto ui = type->retrive_ui(); ui)
				serialize_xml(*ui, (char*)src + offset, dst.append_child(name.c_str()), spec);
			break;
		case TagR:
			if (auto ti = (TypeInfo_Pair*)type; ti)
			{
				dst = dst.append_child(name.c_str());
				auto p = (char*)src + offset;
				dst.append_attribute("first").set_value(data_serialize(ti->ti1, ti->first(p)).c_str());
				dst.append_attribute("second").set_value(data_serialize(ti->ti2, ti->second(p)).c_str());
			}
			break;
		case TagT:
			if (auto ti = (TypeInfo_Tuple*)type; ti)
			{
				dst = dst.append_child(name.c_str());
				auto i = 0; auto p = (char*)src + offset;
				for (auto& t : ti->tis)
				{
					dst.append_attribute(('_' + str(i)).c_str()).set_value(data_serialize(t.first, p + t.second).c_str());
					i++;
				}
			}
			break;
		case TagO:
		{
			auto& vo = *(VirtualUdt<int>*)((char*)src + offset);
			auto n = dst.append_child(name.c_str());
			n.append_attribute("type").set_value(vo.type ? vo.type->name.c_str() : "");
			if (vo.data)
				serialize_xml(*vo.type->retrive_ui(), vo.data, n, spec);
		}
			break;
		case TagPU:
			if (auto it = spec.typed_obj_delegates.find(type); it != spec.typed_obj_delegates.end())
				it->second(*(void**)((char*)src + offset), dst.append_child(name.c_str()));
			break;
		case TagAU:
			if (auto ati = (TypeInfo_ArrayOfUdt*)type; ati->extent > 0)
			{
				auto ti = ati->ti;
				auto ui = ti->ui;
				if (ui)
				{
					auto p = (char*)src + offset;
					auto n = dst.append_child(name.c_str());
					for (auto i = 0; i < ati->extent; i++)
					{
						serialize_xml(*ui, p, n.append_child("item"), spec);
						p += ti->size;
					}
				}
			}
			break;
		case TagVE:
			if (auto& vec = *(std::vector<int>*)((char*)src + offset); !vec.empty())
			{
				auto ti = ((TypeInfo_VectorOfEnum*)type)->ti;
				auto n = dst.append_child(name.c_str());
				for (auto i = 0; i < vec.size(); i++)
				{
					auto nn = n.append_child("item");
					nn.append_attribute("v").set_value(ti->serialize(&vec[i]).c_str());
				}
			}
			break;
		case TagVD:
			if (auto& vec = *(std::vector<char>*)((char*)src + offset); !vec.empty())
			{
				auto ti = ((TypeInfo_VectorOfData*)type)->ti;
				auto n = dst.append_child(name.c_str());
				auto p = (char*)vec.data();
				auto len = vec.size() / ti->size;
				for (auto i = 0; i < len; i++)
				{
					n.append_child("item").append_attribute("v").set_value(ti->serialize(p).c_str());
					p += ti->size;
				}
			}
			break;
		case TagVU:
			if (auto& vec = *(std::vector<char>*)((char*)src + offset); !vec.empty())
			{
				if (auto ui = type->retrive_ui(); ui)
				{
					auto n = dst.append_child(name.c_str());
					auto p = (char*)vec.data();
					auto len = vec.size() / ui->size;
					for (auto i = 0; i < len; i++)
					{
						serialize_xml(*ui, p, n.append_child("item"), spec);
						p += ui->size;
					}
				}
			}
			break;
		case TagVR:
			if (auto& vec = *(std::vector<char>*)((char*)src + offset); !vec.empty())
			{
				auto ti = ((TypeInfo_VectorOfPair*)type)->ti;
				auto n = dst.append_child(name.c_str());
				auto p = (char*)vec.data();
				auto len = vec.size() / ti->size;
				for (auto i = 0; i < len; i++)
				{
					auto nn = n.append_child("item");
					nn.append_attribute("first").set_value(data_serialize(ti->ti1, ti->first(p)).c_str());
					nn.append_attribute("second").set_value(data_serialize(ti->ti2, ti->second(p)).c_str());
					p += ti->size;
				}
			}
			break;
		case TagVT:
			if (auto& vec = *(std::vector<char>*)((char*)src + offset); !vec.empty())
			{
				auto ti = ((TypeInfo_VectorOfTuple*)type)->ti;
				auto n = dst.append_child(name.c_str());
				auto p = (char*)vec.data();
				auto len = vec.size() / ti->size;
				for (auto i = 0; i < len; i++)
				{
					auto nn = n.append_child("item");

					auto j = 0; auto p = (char*)src + offset;
					for (auto& t : ti->tis)
					{
						nn.append_attribute(('_' + str(j)).c_str()).set_value(data_serialize(t.first, p + t.second).c_str());
						j++;
					}

					p += ti->size;
				}
			}
			break;
		case TagVPU:
			if (auto& vec = *(std::vector<void*>*)((char*)src + offset); !vec.empty())
			{
				auto ti = ((TypeInfo_VectorOfPointerOfUdt*)type)->ti;
				if (ti)
				{
					if (auto it = spec.typed_obj_delegates.find(ti); it != spec.typed_obj_delegates.end())
					{
						auto n = dst.append_child(name.c_str());
						for (auto v : vec)
							it->second(v, n.append_child("item"));
					}
					else if (ti->retrive_ui() == &ui)
					{
						auto n = dst.append_child(name.c_str());
						for (auto v : vec)
							serialize_xml(ui, v, n.append_child("item"), spec);
					}
				}
			}
			break;
		}
	}

	void serialize_xml(const UdtInfo& ui, void* src, pugi::xml_node dst, const SerializeXmlSpec& spec)
	{
		if (!ui.attributes.empty())
		{
			for (auto& a : ui.attributes)
			{
				if (spec.skip(ui.name_hash, a.name_hash))
					continue;
				serialize_xml(ui, a.var_off(), a.type, a.name, a.name_hash, a.default_value, a.getter_idx, src, dst, spec);
			}
		}
		else
		{
			for (auto& vi : ui.variables)
			{
				if (spec.skip(ui.name_hash, vi.name_hash))
					continue;
				serialize_xml(ui, vi.offset, vi.type, vi.name, vi.name_hash, vi.default_value, -1, src, dst, spec);
			}
		}
	}

	template<typename T>
	inline void serialize_xml(T* src, pugi::xml_node dst, const SerializeXmlSpec& spec = {})
	{
		auto ti = TypeInfo::get<T>();
		switch (ti->tag)
		{
		case TagU:
			serialize_xml(*ti->retrive_ui(), src, dst, spec);
			break;
		default:
			if (ti->tag >= TagV_Beg && ti->tag <= TagV_End)
			{
				UdtInfo ui;
				auto& vi = ui.variables.emplace_back();
				vi.type = ti;
				vi.offset = 0;
				serialize_xml(ui, src, dst, spec);
			}
			else
				assert(0);
		}
	}

	inline void unserialize_xml(const UdtInfo& ui, pugi::xml_node src, void* dst, const UnserializeXmlSpec& spec = {});

	inline void unserialize_xml(const UdtInfo& ui, uint offset, TypeInfo* type, const std::string& name, uint name_hash, int setter_idx, pugi::xml_node src, void* dst, const UnserializeXmlSpec& spec = {})
	{
		auto data_unserialize = [&](TypeInfo* type, const std::string& value, void* p) {
			if (auto it = spec.typed_delegates.find(type); it != spec.typed_delegates.end())
				it->second(value, p);
			else
				type->unserialize(value, p);
		};

		switch (type->tag)
		{
		case TagE:
		case TagD:
			if (auto a = src.attribute(name.c_str()); a)
			{
				if (!spec.general_delegate || !spec.general_delegate(name, name_hash, type, a.value(), (char*)dst + offset))
				{
					data_unserialize(type, a.value(), setter_idx == -1 ? (char*)dst + offset : type->get_v());
					if (setter_idx != -1)
						type->call_setter(&ui.functions[setter_idx], dst, nullptr);
				}
			}
			break;
		case TagU:
			if (auto c = src.child(name.c_str()); c)
			{
				if (auto ui = type->retrive_ui(); ui)
				{
					if (setter_idx == -1)
						unserialize_xml(*ui, c, (char*)dst + offset, spec);
				}
			}
			break;
		case TagO:
			if (auto c = src.child(name.c_str()); c)
			{
				if (setter_idx == -1)
				{
					auto& vo = *(VirtualUdt<int>*)((char*)dst + offset);
					if (auto a = c.attribute("type"); a)
					{
						if (auto ui = find_udt(sh(a.value())); ui)
						{
							vo.type = TypeInfo::get(TagU, ui->name, *ui->db);
							vo.create();
							unserialize_xml(*ui, c, vo.data, spec);
						}
					}
				}
			}
			break;
		case TagPU:
			if (auto c = src.child(name.c_str()); c)
			{
				auto ti = (TypeInfo_PointerOfUdt*)type;
				if (auto it = spec.typed_obj_delegates.find(ti); it != spec.typed_obj_delegates.end())
				{
					auto v = it->second(c, dst);
					if (v != INVALID_POINTER)
						*(void**)((char*)dst + offset) = v;
				}
				else if (ti->retrive_ui() == &ui)
				{
					auto obj = ui.create_object();
					if (obj)
					{
						unserialize_xml(ui, c, obj, spec);
						*(void**)((char*)dst + offset) = obj;
					}
				}
			}
			break;
		case TagAU:
			if (auto c = src.child(name.c_str()); c)
			{
				if (auto ui = type->retrive_ui(); ui)
				{
					if (setter_idx == -1)
					{
						auto p = (char*)dst + offset;
						for (auto cc : c.children())
						{
							unserialize_xml(*ui, cc, p, spec);
							p += ui->size;
						}
					}
				}
			}
			break;
		case TagVE:
			if (auto c = src.child(name.c_str()); c)
			{
				auto ti = ((TypeInfo_VectorOfEnum*)type)->ti;
				auto read = [&](std::vector<int>& vec) {
					for (auto cc : c.children())
					{
						vec.resize(vec.size() + 1);
						ti->unserialize(cc.attribute("v").value(), &vec[vec.size() - 1]);
					}
				};
				if (setter_idx == -1)
					read(*(std::vector<int>*)((char*)dst + offset));
				else
				{
					std::vector<int> vec;
					read(vec);
					type->call_setter(&ui.functions[setter_idx], dst, &vec);
				}
			}
			break;
		case TagVD:
			if (auto c = src.child(name.c_str()); c)
			{
				auto ti = ((TypeInfo_VectorOfData*)type)->ti;
				auto read = [&](std::vector<char>& vec) {
					auto len = 0;
					for (auto cc : c.children())
					{
						len++;
						vec.resize(len * ti->size);
						auto pd = (char*)vec.data() + (len - 1) * ti->size;
						ti->create(pd);
						ti->unserialize(cc.attribute("v").value(), pd);
					}
					return len;
				};
				if (setter_idx == -1)
					read(*(std::vector<char>*)((char*)dst + offset));
				else
				{
					std::vector<char> vec;
					auto len = read(vec);
					type->call_setter(&ui.functions[setter_idx], dst, &vec);
					for (auto i = 0; i < len; i++)
						ti->destroy((char*)vec.data() + i * ti->size, false);
				}
			}
			break;
		case TagVU:
			if (auto c = src.child(name.c_str()); c)
			{
				auto ti = ((TypeInfo_VectorOfUdt*)type)->ti;
				auto read = [&](std::vector<char>& vec) {
					auto len = 0;
					for (auto cc : c.children())
					{
						len++;
						vec.resize(len * ti->size);
						auto pd = (char*)vec.data() + (len - 1) * ti->size;
						ti->create(pd);
						unserialize_xml(*ti->ui, cc, pd, spec);
					}
					return len;
				};
				if (setter_idx == -1)
					read(*(std::vector<char>*)((char*)dst + offset));
				else
				{
					std::vector<char> vec;
					auto len = read(vec);
					type->call_setter(&ui.functions[setter_idx], dst, &vec);
					for (auto i = 0; i < len; i++)
						ti->destroy((char*)vec.data() + i * ti->size, false);
				}
			}
			break;
		case TagVR:
			if (auto c = src.child(name.c_str()); c)
			{
				auto ti = ((TypeInfo_VectorOfPair*)type)->ti;
				auto read = [&](std::vector<char>& vec) {
					auto len = 0;
					for (auto cc : c.children())
					{
						len++;
						vec.resize(len * ti->size);
						auto pd = (char*)vec.data() + (len - 1) * ti->size;
						ti->create(pd);
						data_unserialize(ti->ti1, cc.attribute("first").value(), ti->first(pd));
						data_unserialize(ti->ti2, cc.attribute("second").value(), ti->second(pd));
					}
					return len;
				};
				if (setter_idx == -1)
					read(*(std::vector<char>*)((char*)dst + offset));
				else
				{
					std::vector<char> vec;
					auto len = read(vec);
					type->call_setter(&ui.functions[setter_idx], dst, &vec);
					for (auto i = 0; i < len; i++)
						ti->destroy((char*)vec.data() + i * ti->size, false);
				}
			}
			break;
		case TagVT:
			if (auto c = src.child(name.c_str()); c)
			{
				auto ti = ((TypeInfo_VectorOfTuple*)type)->ti;
				auto read = [&](std::vector<char>& vec) {
					auto len = 0;
					for (auto cc : c.children())
					{
						len++;
						vec.resize(len * ti->size);
						auto pd = (char*)vec.data() + (len - 1) * ti->size;
						ti->create(pd);
						auto i = 0;
						for (auto& t : ti->tis)
						{
							data_unserialize(t.first, cc.attribute(('_' + str(i)).c_str()).value(), pd + t.second);
							i++;
						}
					}
					return len;
				};
				if (setter_idx == -1)
					read(*(std::vector<char>*)((char*)dst + offset));
				else
				{
					std::vector<char> vec;
					auto len = read(vec);
					type->call_setter(&ui.functions[setter_idx], dst, &vec);
					for (auto i = 0; i < len; i++)
						ti->destroy((char*)vec.data() + i * ti->size, false);
				}
			}
			break;
		case TagVPU:
			if (auto c = src.child(name.c_str()); c)
			{
				auto ti = ((TypeInfo_VectorOfPointerOfUdt*)type)->ti;
				if (auto it = spec.typed_obj_delegates.find(ti); it != spec.typed_obj_delegates.end())
				{
					auto& vec = *(std::vector<void*>*)((char*)dst + offset);
					for (auto cc : c.children())
					{
						auto v = it->second(cc, dst);
						if (v != INVALID_POINTER)
							vec.push_back(v);
					}
				}
				else if (ti->retrive_ui() == &ui)
				{
					auto& vec = *(std::vector<void*>*)((char*)dst + offset);
					for (auto cc : c.children())
					{
						auto obj = ui.create_object();
						if (obj)
						{
							unserialize_xml(ui, cc, obj, spec);
							vec.push_back(obj);
						}
					}
				}
			}
			break;
		}
	}

	void unserialize_xml(const UdtInfo& ui, pugi::xml_node src, void* dst, const UnserializeXmlSpec& spec)
	{
		if (!ui.attributes.empty())
		{
			for (auto& a : ui.attributes)
			{
				if (spec.skip(ui.name_hash, a.name_hash))
					continue;
				unserialize_xml(ui, a.var_off(), a.type, a.name, a.name_hash, a.setter_idx, src, dst, spec);
			}
		}
		else
		{
			for (auto& vi : ui.variables)
			{
				if (spec.skip(ui.name_hash, vi.name_hash))
					continue;
				unserialize_xml(ui, vi.offset, vi.type, vi.name, vi.name_hash, -1, src, dst, spec);
			}
		}
	}

	template<typename T>
	inline void unserialize_xml(pugi::xml_node src, T* dst, const UnserializeXmlSpec& spec = {})
	{
		auto ti = TypeInfo::get<T>();
		switch (ti->tag)
		{
		case TagU:
			unserialize_xml(*ti->retrive_ui(), src, dst, spec);
			break;
		default:
			if (ti->tag >= TagV_Beg && ti->tag <= TagV_End)
			{
				UdtInfo ui;
				auto& vi = ui.variables.emplace_back();
				vi.type = ti;
				vi.offset = 0;
				unserialize_xml(ui, src, dst, spec);
			}
			else
				assert(0);
		}
	}
#endif

	inline void serialize_text(const UdtInfo& ui, void* src, std::ostream& dst, const std::string& indent, const SerializeTextSpec& spec = {})
	{
		auto indent2 = indent + "  ";

		for (auto& vi : ui.variables)
		{
			auto print_name = [&]() {
				if (!vi.name.empty())
					dst << indent << vi.name << std::endl;
			};
			std::function<void(const TextSerializeNode&, const std::string&)> print_value;
			print_value = [&](const TextSerializeNode& src, const std::string& indent) {
				for (auto& c : src.children)
				{
					if (!c.name.empty())
						dst << indent << c.name << std::endl;
					print_value(c, indent + "  ");
				}
			};

			if (spec.skip(ui.name_hash, vi.name_hash))
				continue;

			auto p = (char*)src + vi.offset;
			switch (vi.type->tag)
			{
			case TagE:
			case TagD:
			{
				auto str = vi.type->serialize(p);
				if (str != vi.default_value)
				{
					print_name();
					dst << indent2 << str << std::endl;
				}
			}
				break;
			case TagU:
			{
				if (auto ui = vi.type->retrive_ui(); ui)
				{
					print_name();
					serialize_text(*ui, p, dst, indent2, spec);
				}
			}
				break;
			case TagPU:
			{
				auto ti = (TypeInfo_PointerOfUdt*)vi.type;
				if (auto it = spec.typed_delegates.find(ti); it != spec.typed_delegates.end())
				{
					print_name();
					print_value(it->second(*(void**)p), indent2);
				}
			}
				break;
			case TagVE:
			{
				if (auto& vec = *(std::vector<int>*)p; !vec.empty())
				{
					auto ti = ((TypeInfo_VectorOfEnum*)vi.type)->ti;
					print_name();
					for (auto i = 0; i < vec.size(); i++)
					{
						if (i > 0 && spec.force_print_bar)
							dst << indent << " ---" << std::endl;
						dst << indent2 << ti->serialize(&vec[i]) << std::endl;
					}
				}
			}
				break;
			case TagVD:
			{
				if (auto& vec = *(std::vector<char>*)p; !vec.empty())
				{
					auto ti = ((TypeInfo_VectorOfData*)vi.type)->ti;
					print_name();
					auto len = vec.size() / ti->size;
					p = (char*)vec.data();
					for (auto i = 0; i < len; i++)
					{
						if (i > 0 && spec.force_print_bar)
							dst << indent << " ---" << std::endl;
						dst << indent2 << ti->serialize(p) << std::endl;
						p += ti->size;
					}
				}
			}
				break;
			case TagVU:
			{
				if (auto& vec = *(std::vector<char>*)p; !vec.empty())
				{
					if (auto ui = vi.type->retrive_ui(); ui)
					{
						print_name();
						auto print_bar = ui->variables.size() > 1;
						auto len = vec.size() / ui->size;
						p = (char*)vec.data();
						for (auto i = 0; i < len; i++)
						{
							if (i > 0 && print_bar)
								dst << indent << " ---" << std::endl;
							serialize_text(*ui, p, dst, indent2, spec);
							p += ui->size;
						}
					}
				}
			}
				break;
			case TagVPU:
			{
				if (auto& vec = *(std::vector<void*>*)p; !vec.empty())
				{
					auto ti = ((TypeInfo_VectorOfPointerOfUdt*)vi.type)->ti;
					if (auto it = spec.typed_delegates.find(ti); it != spec.typed_delegates.end())
					{
						print_name();
						auto print_bar = false;
						for (auto i = 0; i < vec.size(); i++)
						{
							if (i > 0 && print_bar)
								dst << indent << " ---" << std::endl;
							auto value = it->second(vec[i]);
							print_value(value, indent2);
							print_bar = !(value.children.size() == 1 && value.children[0].children.empty());
						}
					}
				}
			}
				break;
			}
		}
	}

	template<typename T>
	inline void serialize_text(T* src, std::ostream& dst, const SerializeTextSpec& spec = {})
	{
		auto ti = TypeInfo::get<T>();
		switch (ti->tag)
		{
		case TagU:
			serialize_text(*ti->retrive_ui(), src, dst, "", spec);
			break;
		default:
			if (ti->tag >= TagV_Beg && ti->tag <= TagV_End)
			{
				UdtInfo ui;
				auto& vi = ui.variables.emplace_back();
				vi.type = ti;
				vi.offset = 0;
				serialize_text(ui, src, dst, "", spec);
			}
			else 
				assert(0);
		}
	}

	inline void unserialize_text(const UdtInfo& ui, LineReader& src, uint indent, void* dst, const UnserializeTextSpec& spec = {}, const std::vector<std::pair<std::string, std::string>>& _defines = {})
	{
		auto defines = _defines;
		while (true)
		{
			if (!src.next_line())
				return;
			auto ilen = SUS::indent_length(src.line());
			if (ilen == 0 && src.line()[0] == '%')
			{
				auto sp = SUS::split(src.line().substr(1), '=');
				if (sp.size() == 2)
				{
					auto found = false;
					for (auto& d : defines)
					{
						if (d.first == sp[0])
						{
							found = true;
							break;
						}
					}
					if (!found)
						defines.emplace_back(sp[0], sp[1]);

					if (spec.out_default_defines)
						spec.out_default_defines->emplace_back(sp[0], sp[1]);
				}
			}
			else
			{
				src.anchor--;
				break;
			}
		}
		if (!defines.empty() && !spec.out_default_defines)
		{
			for (auto& d : defines)
				d.first = "{" + d.first + "}";
			for (auto& d : defines)
			{
				for (auto& l : src.lines)
					SUS::replace_all(l, d.first, d.second);
			}
		}

		auto indent2 = indent + 2;

		auto read_var = [&](const VariableInfo& vi) {
			std::function<TextSerializeNode(uint indent)> read_pudt;
			read_pudt = [&](uint indent) {
				TextSerializeNode ret;
				while (true)
				{
					if (!src.next_line())
						return ret;
					auto ilen = SUS::indent_length(src.line());
					if (ilen < indent)
					{
						src.anchor--;
						return ret;
					}
					if (ilen > indent)
						continue;

					auto name = src.line().substr(ilen);
					auto c = read_pudt(indent + 2);
					c.name = name;
					ret.children.push_back(c);
				}
				return ret;
			};

			auto p = (char*)dst + vi.offset;

			switch (vi.type->tag)
			{
			case TagE:
			case TagD:
			{
				if (!src.next_line())
					return;
				auto ilen = SUS::indent_length(src.line());
				if (ilen == indent2)
					vi.type->unserialize(src.line().substr(ilen), p);
			}
				break;
			case TagU:
			{
				if (auto ui = vi.type->retrive_ui(); ui)
					unserialize_text(*ui, src, indent2, p, spec);
			}
				break;
			case TagPU:
			{
				auto ti = (TypeInfo_PointerOfUdt*)vi.type;
				if (auto it = spec.typed_delegates.find(ti); it != spec.typed_delegates.end())
				{
					auto v = it->second(read_pudt(indent2));
					if (v != INVALID_POINTER)
						*(void**)p = v;
				}
			}
				break;
			case TagVE:
			{
				auto ti = ((TypeInfo_VectorOfEnum*)vi.type)->ti;
				auto& vec = *(std::vector<int>*)p;
				while (true)
				{
					if (!src.next_line())
						return;
					auto ilen = SUS::indent_length(src.line());
					if (ilen == indent2)
					{
						vec.resize(vec.size() + 1);
						ti->unserialize(src.line().substr(ilen), &vec[vec.size() - 1]);
					}
					else if (ilen > indent)
						;
					else
					{
						src.anchor--;
						break;
					}
				}
			}
				break;
			case TagVD:
			{
				auto ti = ((TypeInfo_VectorOfData*)vi.type)->ti;
				auto& vec = *(std::vector<char>*)p;
				auto len = 0;
				while (true)
				{
					if (!src.next_line())
						return;
					auto ilen = SUS::indent_length(src.line());
					if (ilen == indent2)
					{
						len++;
						vec.resize(len * ti->size);
						auto pd = (char*)vec.data() + (len - 1) * ti->size;
						ti->create(pd);
						ti->unserialize(src.line().substr(ilen), pd);
					}
					else if (ilen > indent)
						;
					else
					{
						src.anchor--;
						break;
					}
				}
			}
				break;
			case TagVU:
			{
				auto ti = ((TypeInfo_VectorOfUdt*)vi.type)->ti;
				auto ui = ti->ui;
				if (ui)
				{
					auto& vec = *(std::vector<char>*)p;
					auto len = 0;
					while (true)
					{
						if (!src.next_line())
							return;
						src.anchor--;
						auto ilen = SUS::indent_length(src.line(1));
						if (ilen == indent2)
						{
							len++;
							vec.resize(len * ti->size);
							auto pd = (char*)vec.data() + (len - 1) * ti->size;
							ti->create(pd);
							unserialize_text(*ui, src, indent2, pd);
						}
						else if (ilen > indent)
							src.anchor++;
						else
						{
							src.anchor--;
							break;
						}
					}
				}
			}
				break;
			case TagVPU:
			{
				auto ti = ((TypeInfo_VectorOfPointerOfUdt*)vi.type)->ti;
				if (auto it = spec.typed_delegates.find(ti); it != spec.typed_delegates.end())
				{
					auto& vec = *(std::vector<void*>*)p;
					while (true)
					{
						if (!src.next_line())
							return;
						src.anchor--;
						auto ilen = SUS::indent_length(src.line(1));
						if (ilen == indent2)
						{
							auto v = it->second(read_pudt(indent2));
							if (v != INVALID_POINTER)
								vec.push_back(v);
						}
						else if (ilen > indent)
							src.anchor++;
						else
						{
							src.anchor--;
							break;
						}
					}
				}
			}
				break;
			}
		};

		if (ui.variables.size() == 1)
			read_var(ui.variables[0]);
		else
		{
			while (true)
			{
				if (!src.next_line())
					return;
				auto ilen = SUS::indent_length(src.line());
				if (ilen < indent)
				{
					src.anchor--;
					return;
				}
				if (ilen > indent)
					continue;

				auto vi = ui.find_variable({ src.line().begin() + ilen, src.line().end() });
				if (vi)
					read_var(*vi);
				else
					printf("cannot find variable: %s\n", src.line().c_str() + ilen);
			}
		}
	}

	template<typename T>
	inline void unserialize_text(LineReader& src, T* dst, const UnserializeTextSpec& spec = {}, const std::vector<std::pair<std::string, std::string>>& _defines = {})
	{
		auto ti = TypeInfo::get<T>();
		switch (ti->tag)
		{
		case TagU:
			unserialize_text(*ti->retrive_ui(), src, 0, dst, spec, _defines);
			break;
		default:
			if (ti->tag >= TagV_Beg && ti->tag <= TagV_End)
			{
				UdtInfo ui;
				auto& vi = ui.variables.emplace_back();
				vi.type = ti;
				vi.offset = 0;
				unserialize_text(ui, src, 0, dst, spec, _defines);
			}
			else
				assert(0);
		}
	}

	inline void serialize_binary(const UdtInfo& ui, void* src, const std::function<void(const void*, uint)>& writter, const SerializeBinarySpec& spec = {})
	{
		uint zero_len = 0;

		auto write_string = [&](void* src, uint length_bytes) {
			auto& str = *(std::string*)src;
			uint len = str.size();
			writter(&len, length_bytes);
			writter(str.data(), len * sizeof(char));
		};
		auto write_wstring = [&](void* src, uint length_bytes) {
			auto& str = *(std::wstring*)src;
			uint len = str.size();
			writter(&len, length_bytes);
			writter(str.data(), len * sizeof(wchar_t));
		};
		auto write_path = [&](void* src, uint length_bytes) {
			auto& str = ((std::filesystem::path*)src)->native();
			uint len = str.size();
			writter(&len, length_bytes);
			writter(str.data(), len * sizeof(wchar_t));
		};

		for (auto& vi : ui.variables)
		{
			if (spec.skip(ui.name_hash, vi.name_hash))
				continue;

			auto length_bytes = sizeof(uint);
			if (std::string str; vi.metas.get("length_bytes"_h, &str))
				length_bytes = s2t<uint>(str);

			auto p = (char*)src + vi.offset;
			switch (vi.type->tag)
			{
			case TagE:
				writter(p, vi.type->size);
				break;
			case TagD:
				if (auto ti = (TypeInfo_Data*)vi.type; ti)
				{
					switch (ti->data_type)
					{
					case DataString:
						write_string(p, length_bytes);
						break;
					case DataWString:
						write_wstring(p, length_bytes);
						break;
					case DataPath:
						write_path(p, length_bytes);
						break;
					default:
						writter(p, vi.type->size);
					}
				}
				break;
			case TagU:
				if (auto ui = vi.type->retrive_ui(); ui)
					serialize_binary(*ui, p, writter, spec);
				break;
			case TagVE:
				if (auto& vec = *(std::vector<int>*)p; !vec.empty())
				{
					uint len = vec.size();
					writter(&len, length_bytes);
					writter(vec.data(), sizeof(uint) * len);
				}
				else
					writter(&zero_len, length_bytes);
				break;
			case TagVD:
				if (auto& vec = *(std::vector<char>*)p; !vec.empty())
				{
					auto ti = ((TypeInfo_VectorOfData*)vi.type)->ti;
					auto len = vec.size() / ti->size;
					writter(&len, length_bytes);
					p = (char*)vec.data();
					switch (ti->data_type)
					{
					case DataString:
						for (auto i = 0; i < len; i++)
						{
							write_string(p, length_bytes);
							p += ti->size;
						}
						break;
					case DataWString:
						for (auto i = 0; i < len; i++)
						{
							write_wstring(p, length_bytes);
							p += ti->size;
						}
						break;
					case DataPath:
						for (auto i = 0; i < len; i++)
						{
							write_path(p, length_bytes);
							p += ti->size;
						}
						break;
					default:
						writter(vec.data(), vec.size());
					}
				}
				else
					writter(&zero_len, length_bytes);
				break;
			case TagVU:
				if (auto& vec = *(std::vector<char>*)p; !vec.empty())
				{
					if (auto ui = vi.type->retrive_ui(); ui)
					{
						auto len = vec.size() / ui->size;
						writter(&len, length_bytes);
						p = (char*)vec.data();
						if (ui->is_pod)
							writter(vec.data(), vec.size());
						else
						{
							for (auto i = 0; i < len; i++)
							{
								serialize_binary(*ui, p, writter, spec);
								p += ui->size;
							}
						}
					}
				}
				else
					writter(&zero_len, length_bytes);
				break;
			}
		}
	}

	template<typename T>
	inline void serialize_binary(T* src, const std::function<void(const void*, uint)>& writter, const SerializeBinarySpec& spec = {})
	{
		serialize_binary(*TypeInfo::get<T>()->retrive_ui(), src, writter, spec);
	}

	inline void serialize_binary(const UdtInfo& ui, void* src, std::ostream& dst, const SerializeBinarySpec& spec = {})
	{
		serialize_binary(ui, src, [&](const void* data, uint size) {
			dst.write((char*)data, size);
		}, spec);
	}

	template<typename T>
	inline void serialize_binary(T* src, std::ostream& dst, const SerializeBinarySpec& spec = {})
	{
		serialize_binary(*TypeInfo::get<T>()->retrive_ui(), src, dst, spec);
	}

	inline void unserialize_binary(const UdtInfo& ui, const std::function<void(void*, uint)>& reader, void* dst, const UnserializeBinarySpec& spec = {})
	{
		uint len = 0;

		auto read_string = [&](void* dst, uint length_bytes) {
			auto& str = *(std::string*)dst;
			reader(&len, length_bytes);
			str.resize(len);
			if (len > 0)
				reader(str.data(), len * sizeof(char));
		};
		auto read_wstring = [&](void* dst, uint length_bytes) {
			auto& str = *(std::wstring*)dst;
			reader(&len, length_bytes);
			str.resize(len);
			if (len > 0)
				reader(str.data(), len * sizeof(wchar_t));
		};
		auto read_path = [&](void* dst, uint length_bytes) {
			auto& path = *(std::filesystem::path*)dst;
			std::wstring str;
			reader(&len, length_bytes);
			str.resize(len);
			if (len > 0)
				reader(str.data(), len * sizeof(wchar_t));
			path = str;
		};

		for (auto& vi : ui.variables)
		{
			if (spec.skip(ui.name_hash, vi.name_hash))
				continue;

			auto length_bytes = sizeof(uint);
			if (std::string str; vi.metas.get("length_bytes"_h, &str))
				length_bytes = s2t<uint>(str);

			auto p = (char*)dst + vi.offset;
			switch (vi.type->tag)
			{
			case TagE:
				reader(p, vi.type->size);
				break;
			case TagD:
				if (auto ti = (TypeInfo_Data*)vi.type; ti)
				{
					switch (ti->data_type)
					{
					case DataString:
						read_string(p, length_bytes);
						break;
					case DataWString:
						read_wstring(p, length_bytes);
						break;
					case DataPath:
						read_path(p, length_bytes);
						break;
					default:
						reader(p, vi.type->size);
					}
				}
				break;
			case TagU:
				if (auto ui = vi.type->retrive_ui(); ui)
					unserialize_binary(*ui, reader, p, spec);
				break;
			case TagVE:
				if (reader(&len, length_bytes); len > 0)
				{
					auto ti = ((TypeInfo_VectorOfEnum*)vi.type)->ti;
					auto& vec = *(std::vector<int>*)p;
					vec.resize(len);
					reader(vec.data(), sizeof(uint) * len);

				}
				break;
			case TagVD:
				if (reader(&len, length_bytes); len > 0)
				{
					auto ti = ((TypeInfo_VectorOfData*)vi.type)->ti;
					auto& vec = *(std::vector<char>*)p;
					vec.resize(len * ti->size);
					p = (char*)vec.data();
					switch (ti->data_type)
					{
					case DataString:
						for (auto i = 0; i < len; i++)
						{
							ti->create(p);
							read_string(p, length_bytes);
							p += ti->size;
						}
						break;
					case DataWString:
						for (auto i = 0; i < len; i++)
						{
							ti->create(p);
							read_wstring(p, length_bytes);
							p += ti->size;
						}
						break;
					case DataPath:
						for (auto i = 0; i < len; i++)
						{
							ti->create(p);
							read_path(p, length_bytes);
							p += ti->size;
						}
						break;
					default:
						reader(vec.data(), vec.size());
					}
				}
				break;
			case TagVU:
				if (reader(&len, length_bytes); len > 0)
				{
					if (auto ui = vi.type->retrive_ui(); ui)
					{
						auto& vec = *(std::vector<char>*)p;
						vec.resize(len * ui->size);
						if (ui->is_pod)
							reader(vec.data(), vec.size());
						else
						{
							p = vec.data();
							for (auto i = 0; i < len; i++)
							{
								ui->create_object(p);
								unserialize_binary(*ui, reader, p, spec);
								p += ui->size;
							}
						}
					}
				}
				break;
			}
		}
	}

	template<typename T>
	inline void unserialize_binary(const std::function<void(void*, uint)>& reader, T* dst, const UnserializeBinarySpec& spec = {})
	{
		unserialize_binary(*TypeInfo::get<T>()->retrive_ui(), reader, dst, spec);
	}

	inline void unserialize_binary(const UdtInfo& ui, std::ifstream& src, void* dst, const UnserializeBinarySpec& spec = {})
	{
		unserialize_binary(ui, [&](void* data, uint size) {
			src.read((char*)data, size);
		}, dst, spec);
	}

	template<typename T>
	inline void unserialize_binary(std::ifstream& src, T* dst, const UnserializeBinarySpec& spec = {})
	{
		unserialize_binary(*TypeInfo::get<T>()->retrive_ui(), src, dst, spec);
	}
}
