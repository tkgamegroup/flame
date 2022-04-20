#pragma once

#include "typeinfo.h"
#include "../xml.h"
#include "../json.h"

namespace flame
{
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

	struct SerializeXmlSpec : ExcludeSpec
	{
		std::map<TypeInfo*, std::function<void(void* src, pugi::xml_node dst)>> delegates;
	};

	struct UnserializeXmlSpec
	{
		std::map<TypeInfo*, std::function<void* (pugi::xml_node src, void* dst_o)>> delegates;
	};

	struct SerializeTextSpec : ExcludeSpec
	{
		std::map<TypeInfo*, std::function<TextSerializeNode(void* src)>> delegates;
	};

	struct UnserializeTextSpec
	{
		std::map<TypeInfo*, std::function<void* (const TextSerializeNode& src)>> delegates;
	};

	inline void serialize_xml(const UdtInfo& ui, void* src, pugi::xml_node dst, const SerializeXmlSpec& spec = {});

	inline void serialize_xml(const UdtInfo& ui, uint offset, TypeInfo* type, const std::string& name, const std::string& default_value, int getter_idx, void* src, pugi::xml_node dst, const SerializeXmlSpec& spec = {})
	{
		switch (type->tag)
		{
		case TagE:
		case TagD:
			if (getter_idx != -1)
				type->call_getter(&ui.functions[getter_idx], src, nullptr);
			if (auto str = type->serialize(getter_idx == -1 ? (char*)src + offset : nullptr); str != default_value)
				dst.append_attribute(name.c_str()).set_value(str.c_str());
			break;
		case TagU:
			serialize_xml(*type->retrive_ui(), (char*)src + offset, dst.append_child(name.c_str()), spec);
			break;
		case TagPU:
			if (auto it = spec.delegates.find(type); it != spec.delegates.end())
				it->second(*(void**)((char*)src + offset), dst.append_child(name.c_str()));
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
		case TagVD:
			if (auto& vec = *(std::vector<char>*)((char*)src + offset); !vec.empty())
			{
				auto ti = ((TypeInfo_VectorOfData*)type)->ti;
				auto n = dst.append_child(name.c_str());
				auto p = (char*)vec.data();
				auto len = (vec.end() - vec.begin()) / ti->size;
				for (auto i = 0; i < len; i++)
				{
					n.append_child("item").append_attribute("v").
						set_value(ti->serialize(p).c_str());
					p += ti->size;
				}
			}
			break;
		case TagVU:
			if (auto& vec = *(std::vector<char>*)((char*)src + offset); !vec.empty())
			{
				auto ti = ((TypeInfo_VectorOfUdt*)type)->ti;
				auto ui = ti->ui;
				if (ui)
				{
					auto n = dst.append_child(name.c_str());
					auto p = (char*)vec.data();
					auto len = (vec.end() - vec.begin()) / ti->size;
					for (auto i = 0; i < len; i++)
					{
						serialize_xml(*ui, p, n.append_child("item"), spec);
						p += ti->size;
					}
				}
			}
			break;
		case TagVPU:
			if (auto& vec = *(std::vector<void*>*)((char*)src + offset); !vec.empty())
			{
				auto ti = ((TypeInfo_VectorOfPointerOfUdt*)type)->ti;
				if (ti)
				{
					if (auto it = spec.delegates.find(ti); it != spec.delegates.end())
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
				serialize_xml(ui, a.var_off(), a.type, a.name, a.default_value, a.getter_idx, src, dst);
			}
		}
		else
		{
			for (auto& vi : ui.variables)
			{
				if (spec.skip(ui.name_hash, vi.name_hash))
					continue;
				serialize_xml(ui, vi.offset, vi.type, vi.name, vi.default_value, -1, src, dst);
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

	inline void unserialize_xml(const UdtInfo& ui, uint offset, TypeInfo* type, const std::string& name, int setter_idx, pugi::xml_node src, void* dst, const UnserializeXmlSpec& spec = {})
	{
		switch (type->tag)
		{
		case TagE:
		case TagD:
			if (auto a = src.attribute(name.c_str()); a)
			{
				if (setter_idx != -1)
				{
					type->unserialize(a.value(), nullptr);
					type->call_setter(&ui.functions[setter_idx], dst, nullptr);
				}
				else
					type->unserialize(a.value(), (char*)dst + offset);
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
		case TagAU:
			if (auto c = src.child(name.c_str()); c)
			{
				if (auto ui = type->retrive_ui(); ui)
				{
					if (setter_idx == -1)
					{
						auto p = (char*)dst + offset;
						for (auto c : c.children())
						{
							unserialize_xml(*ui, c, p, spec);
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
				if (setter_idx == -1)
				{
					auto& vec = *(std::vector<int>*)((char*)dst + offset);
					for (auto c : c.children())
					{
						vec.resize(vec.size() + 1);
						ti->unserialize(c.attribute("v").value(), &vec[vec.size() - 1]);
					}
				}
			}
			break;
		case TagVD:
			if (auto c = src.child(name.c_str()); c)
			{
				auto ti = ((TypeInfo_VectorOfData*)type)->ti;
				auto read = [&](std::vector<char>& vec) {
					auto len = 0;
					for (auto c : c.children())
					{
						len++;
						vec.resize(len * ti->size);
						auto pd = (char*)vec.data() + (len - 1) * ti->size;
						ti->create(pd);
						ti->unserialize(c.attribute("v").value(), pd);
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
					for (auto c : c.children())
					{
						len++;
						vec.resize(len * ti->size);
						auto pd = (char*)vec.data() + (len - 1) * ti->size;
						ti->create(pd);
						unserialize_xml(*ti->ui, c, pd, spec);
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
		case TagPU:
			if (auto c = src.child(name.c_str()); c)
			{
				auto ti = (TypeInfo_PointerOfUdt*)type;
				if (auto it = spec.delegates.find(ti); it != spec.delegates.end())
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
		case TagVPU:
			if (auto c = src.child(name.c_str()); c)
			{
				auto ti = ((TypeInfo_VectorOfPointerOfUdt*)type)->ti;
				if (auto it = spec.delegates.find(ti); it != spec.delegates.end())
				{
					auto& vec = *(std::vector<void*>*)((char*)dst + offset);
					for (auto c : c.children())
					{
						auto v = it->second(c, dst);
						if (v != INVALID_POINTER)
							vec.push_back(v);
					}
				}
				else if (ti->retrive_ui() == &ui)
				{
					auto& vec = *(std::vector<void*>*)((char*)dst + offset);
					for (auto c : c.children())
					{
						auto obj = ui.create_object();
						if (obj)
						{
							unserialize_xml(ui, c, obj, spec);
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
				unserialize_xml(ui, a.var_off(), a.type, a.name, a.setter_idx, src, dst);
		}
		else
		{
			for (auto& vi : ui.variables)
				unserialize_xml(ui, vi.offset, vi.type, vi.name, -1, src, dst);
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
				auto ui = vi.type->retrive_ui();
				if (ui)
				{
					print_name();
					serialize_text(*ui, p, dst, indent2, spec);
				}
			}
				break;
			case TagPU:
			{
				auto ti = (TypeInfo_PointerOfUdt*)vi.type;
				if (auto it = spec.delegates.find(ti); it != spec.delegates.end())
				{
					print_name();
					print_value(it->second(*(void**)p), indent2);
				}
			}
				break;
			case TagVE:
			{
				auto ti = ((TypeInfo_VectorOfEnum*)vi.type)->ti;
				auto& vec = *(std::vector<int>*)p;
				if (!vec.empty())
				{
					print_name();
					for (auto i = 0; i < vec.size(); i++)
						dst << indent2 << ti->serialize(&vec[i]) << std::endl;
				}
			}
				break;
			case TagVD:
			{
				auto ti = ((TypeInfo_VectorOfData*)vi.type)->ti;
				auto& vec = *(std::vector<char>*)p;
				if (!vec.empty())
				{
					print_name();
					auto len = (vec.end() - vec.begin()) / ti->size;
					p = (char*)vec.data();
					for (auto i = 0; i < len; i++)
					{
						dst << indent2 << ti->serialize(p) << std::endl;
						p += ti->size;
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
					if (!vec.empty())
					{
						print_name();
						auto print_bar = ui->variables.size() > 1;
						auto len = (vec.end() - vec.begin()) / ti->size;
						p = (char*)vec.data();
						for (auto i = 0; i < len; i++)
						{
							if (i > 0 && print_bar)
								dst << indent << " ---" << std::endl;
							serialize_text(*ui, p, dst, indent2, spec);
							p += ti->size;
						}
					}
				}
			}
				break;
			case TagVPU:
			{
				auto ti = ((TypeInfo_VectorOfPointerOfUdt*)vi.type)->ti;
				if (auto it = spec.delegates.find(ti); it != spec.delegates.end())
				{
					auto& vec = *(std::vector<void*>*)p;
					if (!vec.empty())
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

	inline void unserialize_text(const UdtInfo& ui, LineReader& src, uint indent, void* dst, const UnserializeTextSpec& spec = {})
	{
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
				auto ui = vi.type->retrive_ui();
				if (ui)
					unserialize_text(*ui, src, indent2, p, spec);
			}
				break;
			case TagPU:
			{
				auto ti = (TypeInfo_PointerOfUdt*)vi.type;
				if (auto it = spec.delegates.find(ti); it != spec.delegates.end())
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
				if (auto it = spec.delegates.find(ti); it != spec.delegates.end())
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
	inline void unserialize_text(LineReader& src, T* dst, const UnserializeTextSpec& spec = {}, const std::vector<std::string>& _defines = {})
	{
		std::vector<std::pair<std::string, std::string>> defines;
		while (true)
		{
			if (!src.next_line())
				return;
			auto ilen = SUS::indent_length(src.line());
			if (ilen == 0 && src.line()[0] == '%')
			{
				if (src.line()[1] != '%')
				{
					auto sp = SUS::split(src.line().substr(1), '=');
					if (sp.size() == 2)
						defines.emplace_back(sp[0], sp[1]);
				}
			}
			else
			{
				src.anchor--;
				break;
			}
		}
		for (auto& d : _defines)
		{
			auto sp = SUS::split(d, '=');
			if (sp.size() == 2)
			{
				auto found = false;
				for (auto& dd : defines)
				{
					if (dd.first == sp[0])
					{
						dd.second = sp[1];
						found = true;
						break;
					}
				}
				if (!found)
					defines.emplace_back(sp[0], sp[1]);
			}
		}
		if (!defines.empty())
		{
			for (auto& d : defines)
				d.first = "{" + d.first + "}";
			for (auto& d : defines)
			{
				for (auto& l : src.lines)
					SUS::replace_all(l, d.first, d.second);
			}
		}

		auto ti = TypeInfo::get<T>();
		switch (ti->tag)
		{
		case TagU:
			unserialize_text(*ti->retrive_ui(), src, 0, dst, spec);
			break;
		default:
			if (ti->tag >= TagV_Beg && ti->tag <= TagV_End)
			{
				UdtInfo ui;
				auto& vi = ui.variables.emplace_back();
				vi.type = ti;
				vi.offset = 0;
				unserialize_text(ui, src, 0, dst, spec);
			}
			else
				assert(0);
		}
	}
}
