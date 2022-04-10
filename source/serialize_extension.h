#pragma once

#include "xml.h"
#include "json.h"
#include "base64.h"
#include "serialize.h"

namespace flame
{
	struct DataSoup
	{
		std::vector<uchar> soup;

		inline void xml_append(void* data, uint size, pugi::xml_node n)
		{
			n.append_attribute("data_offset").set_value(soup.size());
			n.append_attribute("data_size").set_value(size);
			soup.insert(soup.end(), (uchar*)data, (uchar*)data + size);
		}

		template<typename T>
		inline void xml_append_v(const std::vector<T>& v, pugi::xml_node n)
		{
			n.append_attribute("data_offset").set_value(soup.size());
			uint size = v.size() * sizeof(T);
			n.append_attribute("data_size").set_value(size);
			soup.insert(soup.end(), (uchar*)v.data(), (uchar*)v.data() + size);
		}

		inline void xml_read(void* dst, pugi::xml_node n)
		{
			auto offset = n.attribute("data_offset").as_uint();
			auto size = n.attribute("data_size").as_uint();
			if (size > 0)
				memcpy(dst, &soup[offset], size);
		}

		template<typename T>
		inline void xml_read_v(std::vector<T>& v, pugi::xml_node n)
		{
			auto offset = n.attribute("data_offset").as_uint();
			auto size = n.attribute("data_size").as_uint();
			v.resize(size / sizeof(T));
			if (size > 0)
				memcpy(v.data(), &soup[offset], size);
		}

		inline void save(std::ofstream& dst)
		{
			auto str = base64::encode(soup.data(), soup.size());
			auto len = str.size(); auto p = str.c_str();
			while (len > 256)
			{
				dst << std::string_view(p, p + 256) << std::endl;
				len -= 256;
				p += 256;
			}
			dst << p << std::endl;
			dst << std::endl;
		}

		inline void load(LineReader& src)
		{
			std::string str;
			for (auto& l : src.lines)
				str += l;
			soup = base64::decode(str);
		}

		inline void load(const std::filesystem::path& filename)
		{
			auto content = get_file_content(filename);
			soup.resize(content.size());
			memcpy(soup.data(), content.data(), content.size());
		}
	};
}
