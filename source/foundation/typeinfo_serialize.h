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
			{
				auto ui = ((TypeInfo_Udt*)vi.type)->ui;
				if (ui)
					serialize_xml(ui, p, dst.append_child(vi.name.c_str()));
			}
				break;
			//default:
			//	if (vi.type->tag >= TagV_Beg && vi.type->tag <= TagV_End)
			//	{
			//		auto& vec = *(std::vector<char>*)p;
			//		if (!vec.empty())
			//		{
			//			auto n = dst.append_child(vi.name.c_str());
			//			p = (char*)vec.data();
			//			auto len = (vec.end() - vec.begin()) / vi.type->size;
			//			switch (vi.type->tag)
			//			{

			//			}
			//			switch (ti->tag)
			//			{
			//			case TagE:
			//			case TagEnumFlags:
			//			case TagData:
			//				for (auto i = 0; i < len; i++)
			//				{
			//					auto nn = n.append_child("item");
			//					nn.append_attribute("v").set_value(ti->serialize(p).c_str());
			//					p += ti->size;
			//				}
			//				break;
			//			case TagU:
			//			{
			//				auto ui = ((TypeInfo_Udt*)ti)->ui;
			//				if (ui)
			//				{
			//					for (auto i = 0; i < len; i++)
			//					{
			//						auto nn = n.append_child("item");
			//						serialize_xml(ui, p, nn);
			//						p += ti->size;
			//					}
			//				}
			//			}
			//			break;
			//			}
			//		}
			//	}
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
			{
				auto ui = ((TypeInfo_Udt*)vi.type)->ui;
				if (ui)
				{
					if (!vi.name.empty())
						dst << indent << vi.name << std::endl;
					serialize_text(ui, p, dst, indent + "  ");
				}
			}
				break;
			//case TagVector:
			//	if (!vi.name.empty())
			//		dst << indent << vi.name << std::endl;
			//	auto ti = ((TypeInfo_Vector*)vi.type)->ti;
			//	if (ti)
			//	{
			//		auto& vec = *(std::vector<char>*)p;
			//		p = (char*)vec.data();
			//		auto len = (vec.end() - vec.begin()) / ti->size;
			//		switch (ti->tag)
			//		{
			//		case TagE:
			//		case TagEnumFlags:
			//		case TagData:
			//			for (auto i = 0; i < len; i++)
			//			{
			//				dst << indent << " - " << ti->serialize(p) << std::endl;
			//				p += ti->size;
			//			}
			//			break;
			//		case TagU:
			//		{
			//			auto ui = ((TypeInfo_Udt*)ti)->ui;
			//			if (ui)
			//			{
			//				for (auto i = 0; i < len; i++)
			//				{
			//					dst << indent << " - " << std::endl;
			//					serialize_text(ui, p, dst, indent + "  ");
			//					p += ti->size;
			//				}
			//			}
			//		}
			//			break;
			//		}
			//	}
				dst << std::endl;
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
			break;
		default: 
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
			//case TagVector:
			//{
			//	auto ti = ((TypeInfo_Vector*)vi.type)->ti;
			//	auto& vec = *(std::vector<char>*)((char*)dst + vi.offset);
			//	vec.clear();
			//	auto len = 0;
			//	switch (ti->tag)
			//	{
			//	case TagE:
			//	case TagEnumFlags:
			//	case TagData:
			//		while (!src.eof())
			//		{
			//			std::getline(src, line);
			//			SUS::ltrim(line);
			//			if (line.empty())
			//				break;

			//			len++;
			//			vec.resize(len * ti->size);
			//			ti->unserialize(line.substr(2), (char*)vec.data() + (len - 1) * ti->size);
			//		}
			//		break;
			//	case TagU:
			//		while (!src.eof())
			//		{
			//			std::getline(src, line);
			//			SUS::ltrim(line);
			//			if (line.empty())
			//				break;

			//			len++;
			//			vec.resize(len * ti->size);
			//			unserialize_text(ui, src, (char*)vec.data() + (len - 1) * ti->size);
			//		}
			//		break;
			//	}
			//}
			//	break;
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
			break;
		default:
			assert(0);
		}
	}
}
