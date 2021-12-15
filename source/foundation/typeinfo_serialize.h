#pragma once

#include "typeinfo.h"
#include "../xml.h"
#include "../json.h"

namespace flame
{
	struct SerializeNode
	{
		std::string name;
		std::vector<SerializeNode> children;

		std::string value() const
		{
			return children.empty() ? name : children[0].value();
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

	struct SerializeSpec
	{
		std::map<TypeInfo*, std::function<SerializeNode(void* src)>> map;
	};

	struct UnserializeSpec
	{
		std::map<TypeInfo*, std::function<void*(const SerializeNode& src)>> map;
	};

	inline void serialize_xml(const UdtInfo& ui, void* src, pugi::xml_node dst, const SerializeSpec& spec = {})
	{
		for (auto& vi : ui.variables)
		{
			auto p = (char*)src + vi.offset;
			switch (vi.type->tag)
			{
			case TagE:
			case TagD:
			{
				auto str = vi.type->serialize(p);
				if (str != vi.default_value)
					dst.append_attribute(vi.name.c_str()).set_value(str.c_str());
			}
				break;
			case TagU:
				serialize_xml(*((TypeInfo_Udt*)vi.type)->ui, p, dst.append_child(vi.name.c_str()), spec);
				break;
			case TagVE:
			{
					auto ti = ((TypeInfo_VectorOfEnum*)vi.type)->ti;
				auto& vec = *(std::vector<int>*)p;
				if (!vec.empty())
				{
					auto n = dst.append_child(vi.name.c_str());
					for (auto i = 0; i < vec.size(); i++)
					{
						auto nn = n.append_child("item");
						nn.append_attribute("v").set_value(ti->serialize(&vec[i]).c_str());
					}
				}
			}
				break;
			case TagVD:
			{
				auto ti = ((TypeInfo_VectorOfData*)vi.type)->ti;
				auto& vec = *(std::vector<char>*)p;
				if (!vec.empty())
				{
					auto n = dst.append_child(vi.name.c_str());
					p = (char*)vec.data();
					auto len = (vec.end() - vec.begin()) / ti->size;
					for (auto i = 0; i < len; i++)
					{
						auto nn = n.append_child("item");
						nn.append_attribute("v").set_value(ti->serialize(p).c_str());
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
						auto n = dst.append_child(vi.name.c_str());
						p = (char*)vec.data();
						auto len = (vec.end() - vec.begin()) / ti->size;
						for (auto i = 0; i < len; i++)
						{
							auto nn = n.append_child("item");
							serialize_xml(*ui, p, nn, spec);
							p += ti->size;
						}
					}
				}
			}
				break;
			case TagVPU:
				break;
			}
		}
	}

	inline void serialize_text(const UdtInfo& ui, void* src, std::ofstream& dst, const std::string& indent, const SerializeSpec& spec = {})
	{
		auto indent2 = indent;
		if (ui.variables.size() > 1)
			indent2 += "  ";

		for (auto& vi : ui.variables)
		{
			auto print_name = [&]() {
				if (!vi.name.empty())
					dst << indent << vi.name << std::endl;
			};
			std::function<void(const SerializeNode&, const std::string&)> print_value;
			print_value = [&](const SerializeNode& src, const std::string& indent) {
				for (auto& c : src.children)
				{
					if (!c.name.empty())
						dst << indent << c.name << std::endl;
					print_value(c, indent + "  ");
				}
			};

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
				auto ui = ((TypeInfo_Udt*)vi.type)->ui;
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
				if (auto it = spec.map.find(ti); it != spec.map.end())
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
				if (auto it = spec.map.find(ti); it != spec.map.end())
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

	template <class T>
	inline void serialize_text(T* src, std::ofstream& dst, const SerializeSpec& spec = {})
	{
		auto ti = TypeInfo::get<T>();
		switch (ti->tag)
		{
		case TagU:
			serialize_text(*((TypeInfo_Udt*)ti)->ui, src, dst, "", spec);
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

	inline void unserialize_text(const UdtInfo& ui, LineReader& src, uint indent, void* dst, const UnserializeSpec& spec = {})
	{
		auto indent2 = indent;
		if (ui.variables.size() > 1)
			indent2 += 2;

		auto read_var = [&](const VariableInfo& vi) {
			std::function<SerializeNode(uint indent)> read_value;
			read_value = [&](uint indent) {
				SerializeNode ret;
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
					auto c = read_value(indent + 2);
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
				auto ui = ((TypeInfo_Udt*)vi.type)->ui;
				if (ui)
					unserialize_text(*ui, src, indent2, p);
			}
				break;
			case TagPU:
			{
				auto ti = (TypeInfo_PointerOfUdt*)vi.type;
				if (auto it = spec.map.find(ti); it != spec.map.end())
				{
					auto v = it->second(read_value(indent2));
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
				if (auto it = spec.map.find(ti); it != spec.map.end())
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
							auto v = it->second(read_value(indent2));
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

	template <class T>
	inline void unserialize_text(LineReader& src, T* dst, const UnserializeSpec& spec = {}, const std::vector<std::string>& _defines = {})
	{
		std::vector<std::pair<std::string, std::string>> defines;
		while (true)
		{
			if (!src.next_line())
				return;
			auto ilen = SUS::indent_length(src.line());
			if (ilen == 0 && src.line()[0] == '%')
			{
				auto sp = SUS::split(src.line().substr(1), '=');
				if (sp.size() == 2)
					defines.emplace_back(sp[0], sp[1]);
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
			unserialize_text(*((TypeInfo_Udt*)ti)->ui, src, 0, dst, spec);
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
