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
			case TagEnum:
			case TagEnumFlags:
			case TagData:
			{
				auto str = vi.type->serialize(p);
				if (str != vi.default_value)
					dst.append_attribute(vi.name.c_str()).set_value(str.c_str());
			}
				break;
			case TagUdt:
			{
				auto ui = ((TypeInfo_Udt*)vi.type)->ui;
				if (ui)
					serialize_xml(ui, p, dst.append_child(vi.name.c_str()));
			}
				break;
			case TagVector:
			{
				auto ti = ((TypeInfo_Vector*)vi.type)->ti;
				if (ti)
				{
					auto& vec = *(std::vector<char>*)p;
					if (!vec.empty())
					{
						auto n = dst.append_child(vi.name.c_str());
						p = (char*)vec.data();
						auto len = (vec.end() - vec.begin()) / ti->size;
						switch (ti->tag)
						{
						case TagEnum:
						case TagEnumFlags:
						case TagData:
							for (auto i = 0; i < len; i++)
							{
								auto nn = n.append_child("item");
								nn.append_attribute("v").set_value(ti->serialize(p).c_str());
								p += ti->size;
							}
							break;
						case TagUdt:
						{
							auto ui = ((TypeInfo_Udt*)ti)->ui;
							if (ui) 
							{
								for (auto i = 0; i < len; i++)
								{
									auto nn = n.append_child("item");
									serialize_xml(ui, p, nn);
									p += ti->size;
								}
							}
						}
							break;
						}
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
			case TagEnum:
			case TagEnumFlags:
			case TagData:
				dst << indent << vi.name << std::endl;
				dst << indent << " - " << vi.type->serialize(p) << std::endl;
				break;
			case TagUdt:
			{
				auto ui = ((TypeInfo_Udt*)vi.type)->ui;
				if (ui)
				{
					dst << indent << vi.name << std::endl;
					serialize_text(ui, p, dst, indent + "  ");
				}
			}
				break;
			case TagVector:
				auto ti = ((TypeInfo_Vector*)vi.type)->ti;
				if (ti)
				{
					auto& vec = *(std::vector<char>*)p;
					p = (char*)vec.data();
					auto len = (vec.end() - vec.begin()) / ti->size;
					switch (ti->tag)
					{
					case TagEnum:
					case TagEnumFlags:
					case TagData:
						for (auto i = 0; i < len; i++)
						{
							dst << indent << " - " << ti->serialize(p) << std::endl;
							p += ti->size;
						}
						break;
					case TagUdt:
					{
						auto ui = ((TypeInfo_Udt*)ti)->ui;
						if (ui)
						{
							for (auto i = 0; i < len; i++)
							{
								dst << indent << " - " << std::endl;
								serialize_text(ui, p, dst, indent + "  ");
								p += ti->size;
							}
						}
					}
						break;
					}
				}
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
		case TagUdt:
			serialize_text(((TypeInfo_Udt*)ti)->ui, src, dst);
			break;
		case TagVector:
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
	}
}
