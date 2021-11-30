#pragma once

#include "typeinfo.h"
#include "../xml.h"
#include "../json.h"

namespace flame
{
	inline void serialize_xml(UdtInfo* ui, void* src, pugi::xml_node dst, 
		const std::function<bool(VariableInfo* vi, TypeInfo* ti, void* src, pugi::xml_node dst)>& pre_callback = {})
	{
		for (auto& vi : ui->variables)
		{
			auto p = (char*)src + vi.offset;
			if (pre_callback && pre_callback(&vi, nullptr, p, dst))
				continue;
			switch (vi.type->tag)
			{
			case TagEnumSingle:
			case TagEnumMulti:
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
					serialize_xml(ui, p, dst.append_child(vi.name.c_str()), pre_callback);
			}
				break;
			case TagVector:
			{
				auto ti = ((TypeInfo_Vector*)vi.type)->ti;
				if (ti)
				{
					auto& vec = *(std::vector<int>*)p;
					if (vec.size() > 0)
					{
						auto n = dst.append_child(vi.name.c_str());
						p = (char*)vec.data();
						switch (ti->tag)
						{
						case TagEnumSingle:
						case TagEnumMulti:
						case TagData:
							for (auto i = 0; i < vec.size(); i++)
							{
								auto nn = n.append_child("item");
								if (pre_callback && pre_callback(nullptr, ti, p, nn))
									continue;
								nn.append_attribute("v").set_value(ti->serialize(p).c_str());
								p += ti->size;
							}
							break;
						case TagUdt:
						{
							auto ui = ((TypeInfo_Udt*)ti)->ui;
							if (ui) 
							{
								for (auto i = 0; i < vec.size(); i++)
								{
									auto nn = n.append_child("item");
									if (pre_callback && pre_callback(nullptr, ti, p, nn))
										continue;
									serialize_xml(ui, p, nn, pre_callback);
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

	inline void serialize_text(UdtInfo* ui, void* src, std::ofstream& dst, const std::string& indent)
	{
		for (auto& vi : ui->variables)
		{
			auto p = (char*)src + vi.offset;
			switch (vi.type->tag)
			{
			case TagEnumSingle:
			case TagEnumMulti:
			case TagData:
				dst << indent << vi.name << std::endl;
				dst << indent << "- " << vi.type->serialize(p) << std::endl;
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
				}
				dst << std::endl;
				break;
			}
		}
		dst << std::endl;
	}
}
