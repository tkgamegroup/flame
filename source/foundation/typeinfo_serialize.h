#pragma once

#include "typeinfo.h"
#include "../xml.h"
#include "../json.h"

namespace flame
{
	inline void serialize_xml(UdtInfo* ui, void* src, pugi::xml_node dst)
	{
		for (auto& vi : ui->variables)
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
				serialize_xml(((TypeInfo_Udt*)vi.type)->ui, p, dst.append_child(vi.name.c_str()));
				break;
			case TagVE:
			{
				auto& vec = *(std::vector<int>*)p;
				if (!vec.empty())
				{
					auto ti = ((TypeInfo_VectorOfEnum*)vi.type)->ti;
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
				auto& vec = *(std::vector<char>*)p;
				if (!vec.empty())
				{
					auto ti = ((TypeInfo_VectorOfData*)vi.type)->ti;
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
				auto& vec = *(std::vector<char>*)p;
				if (!vec.empty())
				{
					auto ti = ((TypeInfo_VectorOfUdt*)vi.type)->ti;
					auto n = dst.append_child(vi.name.c_str());
					p = (char*)vec.data();
					auto len = (vec.end() - vec.begin()) / ti->size;
					for (auto i = 0; i < len; i++)
					{
						auto nn = n.append_child("item");
						serialize_xml(ti->ui, p, nn);
						p += ti->size;
					}
				}
			}
				break;
			}
		}
	}

	inline void serialize_text(UdtInfo* ui, void* src, std::ofstream& dst, const std::string& indent = "")
	{
		for (auto& vi : ui->variables)
		{
			auto p = (char*)src + vi.offset;
			switch (vi.type->tag)
			{
			case TagE:
			case TagD:
			{
				auto str = vi.type->serialize(p);
				if (str != vi.default_value)
				{
					if (!vi.name.empty())
						dst << indent << vi.name << std::endl;
					dst << indent << " - " << str << std::endl;
				}
			}
				break;
			case TagU:
				if (!vi.name.empty())
					dst << indent << vi.name << std::endl;
				serialize_text(((TypeInfo_Udt*)vi.type)->ui, p, dst, indent + "  ");
				break;
			case TagVE:
			{
				auto& vec = *(std::vector<int>*)p;
				if (!vec.empty())
				{
					if (!vi.name.empty())
						dst << indent << vi.name << std::endl;
					auto ti = ((TypeInfo_VectorOfEnum*)vi.type)->ti;
					for (auto i = 0; i < vec.size(); i++)
						dst << indent << " - " << ti->serialize(&vec[i]) << std::endl;
					dst << std::endl;
				}
			}
				break;
			case TagVD:
			{
				auto& vec = *(std::vector<char>*)p;
				if (!vec.empty())
				{
					if (!vi.name.empty())
						dst << indent << vi.name << std::endl;
					auto ti = ((TypeInfo_VectorOfData*)vi.type)->ti;
					auto len = (vec.end() - vec.begin()) / ti->size;
					p = (char*)vec.data();
					for (auto i = 0; i < len; i++)
					{
						dst << indent << " - " << ti->serialize(p) << std::endl;
						p += ti->size;
					}
					dst << std::endl;
				}
			}
				break;
			case TagVU:
			{
				auto& vec = *(std::vector<char>*)p;
				if (!vec.empty())
				{
					if (!vi.name.empty())
						dst << indent << vi.name << std::endl;
					auto ti = ((TypeInfo_VectorOfUdt*)vi.type)->ti;
					auto len = (vec.end() - vec.begin()) / ti->size;
					p = (char*)vec.data();
					for (auto i = 0; i < len; i++)
					{
						dst << indent << " - " << std::endl;
						serialize_text(ui, p, dst, indent + "  ");
						p += ti->size;
					}
					dst << std::endl;
				}
			}
				break;
			}
		}
		dst << std::endl;
	}

	template <class T>
	inline void serialize_text(T* src, std::ofstream& dst)
	{
		auto ti = TypeInfo::get<T>();
		switch (ti->tag)
		{
		case TagU:
			serialize_text(((TypeInfo_Udt*)ti)->ui, src, dst);
			break;
		default:
			if (ti->tag >= TagV_Beg && ti->tag <= TagV_End)
			{
				UdtInfo ui;
				auto& vi = ui.variables.emplace_back();
				vi.type = ti;
				vi.offset = 0;
				serialize_text(&ui, src, dst);
			}
			else 
				assert(0);
		}
	}

	inline void unserialize_text(UdtInfo* ui, std::ifstream& src, void* dst)
	{
		std::string line;
		auto read_var = [&](VariableInfo& vi) {
			switch (vi.type->tag)
			{
			case TagE:
			case TagD:
			{
				std::getline(src, line);
				SUS::ltrim(line);
				vi.type->unserialize(line.substr(2), (char*)dst + vi.offset);
			}
				break;
			case TagU:
			{
				auto ui = ((TypeInfo_Udt*)vi.type)->ui;
				unserialize_text(ui, src, (char*)dst + vi.offset);
			}
				break;
			case TagVE:
			{
				auto& vec = *(std::vector<int>*)((char*)dst + vi.offset);
				auto ti = ((TypeInfo_VectorOfEnum*)vi.type)->ti;
				while (!src.eof())
				{
					std::getline(src, line);
					SUS::ltrim(line);
					if (line.empty())
						break;

					vec.resize(vec.size() + 1);
					ti->unserialize(line.substr(2), &vec[vec.size() - 1]);
				}
			}
				break;
			case TagVD:
			{
				auto& vec = *(std::vector<char>*)((char*)dst + vi.offset);
				auto ti = ((TypeInfo_VectorOfData*)vi.type)->ti;
				auto len = 0;
				while (!src.eof())
				{
					std::getline(src, line);
					SUS::ltrim(line);
					if (line.empty())
						break;

					len++;
					vec.resize(len * ti->size);
					ti->unserialize(line.substr(2), (char*)vec.data() + (len - 1) * ti->size);
				}
			}
				break;
			case TagVU:
			{
				auto& vec = *(std::vector<char>*)((char*)dst + vi.offset);
				auto ti = ((TypeInfo_VectorOfData*)vi.type)->ti;
				auto len = 0;
				while (!src.eof())
				{
					std::getline(src, line);
					SUS::ltrim(line);
					if (line.empty())
						break;

					len++;
					vec.resize(len * ti->size);
					unserialize_text(ui, src, (char*)vec.data() + (len - 1) * ti->size);
				}
			}
				break;
			}
		};

		if (ui->variables.size() == 1 && ui->variables[0].name.empty())
			read_var(ui->variables[0]);
		else
		{
			while (!src.eof())
			{

				std::getline(src, line);
				SUS::ltrim(line);
				if (line.empty())
					return;

				read_var(*ui->find_variable(line));
			}
		}
	}

	template <class T>
	inline void unserialize_text(std::ifstream& src, T* dst)
	{
		auto ti = TypeInfo::get<T>();
		switch (ti->tag)
		{
		case TagU:
			unserialize_text(((TypeInfo_Udt*)ti)->ui, src, dst);
			break;
		default:
			if (ti->tag >= TagV_Beg && ti->tag <= TagV_End)
			{
				UdtInfo ui;
				auto& vi = ui.variables.emplace_back();
				vi.type = ti;
				vi.offset = 0;
				unserialize_text(&ui, src, dst);
			}
			else
				assert(0);
		}
	}
}
