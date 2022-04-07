#pragma once

#include "xml.h"
#include "json.h"
#include "serialize.h"

namespace flame
{
	struct DataSoup
	{
		std::vector<uint> soup;

		inline void xml_append(void* data, uint size, pugi::xml_node n)
		{
			n.append_attribute("data_offset").set_value(soup.size());
			n.append_attribute("data_size").set_value(size);
			soup.insert(soup.end(), (uint*)data, (uint*)data + size);
		}

		template<typename T>
		inline void xml_append_v(const std::vector<T>& v, pugi::xml_node n)
		{
			n.append_attribute("data_offset").set_value(soup.size());
			uint size = v.size() * sizeof(T) / sizeof(uint);
			n.append_attribute("data_size").set_value(size);
			soup.insert(soup.end(), (uint*)v.data(), (uint*)v.data() + size);
		}

		inline void xml_read(void* dst, pugi::xml_node n)
		{
			auto offset = n.attribute("data_offset").as_uint();
			auto size = n.attribute("data_size").as_uint();
			memcpy(dst, &soup[offset], sizeof(uint) * size);
		}

		template<typename T>
		inline void xml_read_v(const std::vector<T>& v, pugi::xml_node n)
		{
			auto offset = n_positions.attribute("data_offset").as_uint();
			auto size = n_positions.attribute("data_size").as_uint();
			v.resize(size * sizeof(uint) / sizeof(T));
			memcpy(v.data(), &soup[offset], sizeof(uint) * size);
		}

		inline void save(std::ofstream& dst)
		{
			for (auto i = 0; i < soup.size(); i++)
			{
				dst << str_hex(soup[i]) << " ";
				if (i % 10 == 9)
					dst << std::endl;
			}
			if (soup.size() % 10 != 0)
				dst << std::endl;
			dst << std::endl;
		}

		inline void load(LineReader& src)
		{
			for (auto& l : src.lines)
			{
				for (auto& b : SUS::split(l))
					soup.push_back(s2u_hex<uint>(b));
			}
		}
	};
}
