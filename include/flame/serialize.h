#pragma once

#include <flame/math.h>

#include <pugixml.hpp>
#include <nlohmann/json.hpp>

#include <stdarg.h>
#include <regex>
#include <locale>
#include <codecvt>

namespace flame
{
	inline std::string to_string(int v)
	{
		char buf[20];
		sprintf(buf, "%d", v);
		return buf;
	}

	inline std::string to_string(uint v)
	{
		char buf[20];
		sprintf(buf, "%d", v);
		return buf;
	}

	inline std::string to_string(float v, int precision = 6)
	{
		char buf[20];
		sprintf(buf, "%.*f", precision, v);
		return buf;
	}

	inline std::string to_string(uchar v)
	{
		char buf[20];
		sprintf(buf, "%d", v);
		return buf;
	}

	template<uint N>
	inline std::string to_string(const Vec<N, uint>& v)
	{
		auto ret = to_string(v[0]);
		for (auto i = 1; i < N; i++)
			ret += ";" + to_string(v[i]);
		return ret;
	}

	template<uint N>
	inline std::string to_string(const Vec<N, int>& v)
	{
		auto ret = to_string(v[0]);
		for (auto i = 1; i < N; i++)
			ret += ";" + to_string(v[i]);
		return ret;
	}

	template<uint N>
	inline std::string to_string(const Vec<N, float>& v, int precision = 6)
	{
		auto ret = to_string(v[0], precision);
		for (auto i = 1; i < N; i++)
			ret += ";" + to_string(v[i], precision);
		return ret;
	}

	template<uint N>
	inline std::string to_string(const Vec<N, uchar>& v)
	{
		auto ret = to_string(v[0]);
		for (auto i = 1; i < N; i++)
			ret += ";" + to_string(v[i]);
		return ret;
	}

	inline std::wstring to_wstring(int v)
	{
		wchar_t buf[20];
		swprintf(buf, L"%d", v);
		return buf;
	}

	inline std::wstring to_wstring(uint v)
	{
		wchar_t buf[20];
		swprintf(buf, L"%d", v);
		return buf;
	}


	inline std::wstring to_wstring(float v, int precision = 6)
	{
		wchar_t buf[20];
		swprintf(buf, L"%.*f", precision, v);
		return buf;
	}

	inline std::wstring to_wstring(uchar v)
	{
		wchar_t buf[20];
		swprintf(buf, L"%d", v);
		return buf;
	}

	template<uint N>
	inline std::wstring to_wstring(const Vec<N, uint>& v)
	{
		auto ret = to_wstring(v[0]);
		for (auto i = 1; i < N; i++)
			ret += L";" + to_wstring(v[i]);
		return ret;
	}

	template<uint N>
	inline std::wstring to_wstring(const Vec<N, int>& v)
	{
		auto ret = to_wstring(v[0]);
		for (auto i = 1; i < N; i++)
			ret += L";" + to_wstring(v[i]);
		return ret;
	}

	template<uint N>
	inline std::wstring to_wstring(const Vec<N, float>& v, int precision = 6)
	{
		auto ret = to_wstring(v[0], precision);
		for (auto i = 1; i < N; i++)
			ret += L";" + to_wstring(v[i], precision);
		return ret;
	}

	template<uint N>
	inline std::wstring to_wstring(const Vec<N, uchar>& v)
	{
		auto ret = to_wstring(v[0]);
		for (auto i = 1; i < N; i++)
			ret += L";" + to_wstring(v[i]);
		return ret;
	}

	inline Vec2f stof2(const char* s)
	{
		Vec2f ret;
		sscanf(s, "%f;%f", &ret.x(), &ret.y());
		return ret;
	}

	inline Vec3f stof3(const char* s)
	{
		Vec3f ret;
		sscanf(s, "%f;%f;%f", &ret.x(), &ret.y(), &ret.z());
		return ret;
	}

	inline Vec4f stof4(const char* s)
	{
		Vec4f ret;
		sscanf(s, "%f;%f;%f;%f", &ret.x(), &ret.y(), &ret.z(), &ret.w());
		return ret;
	}

	inline Vec2u stou2(const char* s)
	{
		Vec2u ret;
		sscanf(s, "%u;%u", &ret.x(), &ret.y());
		return ret;
	}

	inline Vec3u stou3(const char* s)
	{
		Vec3u ret;
		sscanf(s, "%u;%u;%u", &ret.x(), &ret.y(), &ret.z());
		return ret;
	}

	inline Vec4u stou4(const char* s)
	{
		Vec4u ret;
		sscanf(s, "%u;%u;%u;%u", &ret.x(), &ret.y(), &ret.z(), &ret.w());
		return ret;
	}

	inline Vec2i stoi2(const char* s)
	{
		Vec2i ret;
		sscanf(s, "%d;%d", &ret.x(), &ret.y());
		return ret;
	}

	inline Vec3i stoi3(const char* s)
	{
		Vec3i ret;
		sscanf(s, "%d;%d;%d", &ret.x(), &ret.y(), &ret.z());
		return ret;
	}

	inline Vec4i stoi4(const char* s)
	{
		Vec4i ret;
		sscanf(s, "%d;%d;%d;%d", &ret.x(), &ret.y(), &ret.z(), &ret.w());
		return ret;
	}

	inline Vec2c stoc2(const char* s)
	{
		Vec2i ret;
		sscanf(s, "%d;%d", &ret.x(), &ret.y());
		return Vec2c(ret);
	}

	inline Vec3c stoc3(const char* s)
	{
		Vec3i ret;
		sscanf(s, "%d;%d;%d", &ret.x(), &ret.y(), &ret.z());
		return Vec3c(ret);
	}

	inline Vec4c stoc4(const char* s)
	{
		Vec4i ret;
		sscanf(s, "%d;%d;%d;%d", &ret.x(), &ret.y(), &ret.z(), &ret.w());
		return Vec4c(ret);
	}

	template<class T>
	T sto(const char* s); 

	template<>
	inline int sto<int>(const char* s)
	{
		return std::stoi(s);
	}

	template<>
	inline uint sto<uint>(const char* s)
	{
		return std::stoul(s);
	}

	template<>
	inline float sto<float>(const char* s)
	{
		return std::stof(s);
	}

	template<>
	inline uchar sto<uchar>(const char* s)
	{
		return std::stoul(s);
	}

	inline Vec2f stof2(const wchar_t* s)
	{
		Vec2f ret;
		swscanf(s, L"%f;%f", &ret.x(), &ret.y());
		return ret;
	}

	inline Vec3f stof3(const wchar_t* s)
	{
		Vec3f ret;
		swscanf(s, L"%f;%f;%f", &ret.x(), &ret.y(), &ret.z());
		return ret;
	}

	inline Vec4f stof4(const wchar_t* s)
	{
		Vec4f ret;
		swscanf(s, L"%f;%f;%f;%f", &ret.x(), &ret.y(), &ret.z(), &ret.w());
		return ret;
	}

	inline Vec2u stou2(const wchar_t* s)
	{
		Vec2u ret;
		swscanf(s, L"%u;%u", &ret.x(), &ret.y());
		return ret;
	}

	inline Vec3u stou3(const wchar_t* s)
	{
		Vec3u ret;
		swscanf(s, L"%u;%u;%u", &ret.x(), &ret.y(), &ret.z());
		return ret;
	}

	inline Vec4u stou4(const wchar_t* s)
	{
		Vec4u ret;
		swscanf(s, L"%u;%u;%u;%u", &ret.x(), &ret.y(), &ret.z(), &ret.w());
		return ret;
	}

	inline Vec2i stoi2(const wchar_t* s)
	{
		Vec2i ret;
		swscanf(s, L"%d;%d", &ret.x(), &ret.y());
		return ret;
	}

	inline Vec3i stoi3(const wchar_t* s)
	{
		Vec3i ret;
		swscanf(s, L"%d;%d;%d", &ret.x(), &ret.y(), &ret.z());
		return ret;
	}

	inline Vec4i stoi4(const wchar_t* s)
	{
		Vec4i ret;
		swscanf(s, L"%d;%d;%d;%d", &ret.x(), &ret.y(), &ret.z(), &ret.w());
		return ret;
	}

	inline Vec2c stoc2(const wchar_t* s)
	{
		Vec2i ret;
		swscanf(s, L"%d;%d", &ret.x(), &ret.y());
		return Vec2c(ret);
	}

	inline Vec3c stoc3(const wchar_t* s)
	{
		Vec3i ret;
		swscanf(s, L"%d;%d;%d", &ret.x(), &ret.y(), &ret.z());
		return Vec3c(ret);
	}

	inline Vec4c stoc4(const wchar_t* s)
	{
		Vec4i ret;
		swscanf(s, L"%d;%d;%d;%d", &ret.x(), &ret.y(), &ret.z(), &ret.w());
		return Vec4c(ret);
	}

	template<class T>
	T sto(const wchar_t* s);

	template<>
	inline int sto<int>(const wchar_t* s)
	{
		return std::stoi(s);
	}

	template<>
	inline uint sto<uint>(const wchar_t* s)
	{
		return std::stoul(s);
	}

	template<>
	inline float sto<float>(const wchar_t* s)
	{
		return std::stof(s);
	}

	template<>
	inline uchar sto<uchar>(const wchar_t* s)
	{
		return std::stoul(s);
	}

	template<typename CH>
	std::basic_string<CH> scut(const std::basic_string<CH>& str, int length) // < 0 means from end
	{
		if (length < 0)
			length = str.size() + length;
		return std::basic_string<CH>(str.begin(), str.begin() + length);
	}

	template<typename CH>
	bool sendswith(const std::basic_string<CH>& str, const std::basic_string<CH>& oth)
	{
		return str.size() > oth.size() && str.compare(str.size() - oth.size(), oth.size(), oth) == 0;
	}

	template<typename CH>
	std::vector<std::basic_string<CH>> ssplit(const std::basic_string<CH>& str, CH delimiter = ' ')
	{
		std::basic_istringstream<CH> iss(str);
		std::vector<std::basic_string<CH>> ret;

		std::basic_string<CH> s;
		while (std::getline(iss, s, delimiter))
			ret.push_back(s);

		return ret;
	}

	template<typename CH>
	std::vector<std::basic_string<CH>> ssplit_lastone(const std::basic_string<CH>& str, CH delimiter = ' ')
	{
		auto i = str.size() - 1;
		for (; i >= 0; i--)
		{
			if (str[i] == delimiter)
				break;
		}
		std::vector<std::basic_string<CH>> ret;
		ret.push_back(i > 0 ? std::basic_string<CH>(str.begin(), str.begin() + i) : std::basic_string<CH>());
		ret.push_back(i < str.size() - 1 ? std::basic_string<CH>(str.begin() + i + 1, str.end()) : std::basic_string<CH>());
		return ret;
	}

	template<typename CH>
	std::vector<std::basic_string<CH>> ssplit_dbnull(const CH* str)
	{
		std::vector<std::basic_string<CH>> ret;

		auto p = str, q = str;
		while (true)
		{
			if (*p == 0)
			{
				ret.push_back(q);
				p++;
				if (*p == 0)
					break;
				q = p;
			}
			else
				p++;
		}

		return ret;
	}

	template<typename CH>
	std::vector<std::basic_string<CH>> ssplit_regex(const std::basic_string<CH>& str, const std::basic_regex<CH>& reg, uint req_idx = 0)
	{
		std::vector<std::basic_string<CH>> ret;

		std::match_results<typename std::basic_string<CH>::const_iterator> match;
		auto s = str;

		while (std::regex_search(s, match, reg))
		{
			ret.push_back(match[req_idx]);
			s = match.suffix();
		}

		return ret;
	}

	inline std::wstring a2w(const std::string& str)
	{
		setlocale(LC_ALL, "chs");
		auto len = mbstowcs(nullptr, str.c_str(), 0) + 1;
		std::wstring wstr;
		wstr.resize(len);
		mbstowcs((wchar_t*)wstr.data(), str.c_str(), len);
		setlocale(LC_ALL, "");
		return wstr;
	}

	inline std::wstring s2w(const std::string& str)
	{
		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
		return converter.from_bytes(str);
	}

	inline std::string w2s(const std::wstring& wstr)
	{
		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
		return converter.to_bytes(wstr);
	}

	inline std::string sfmt(const std::string& fmt, ...)
	{
		static char buf[1024];
		va_list ap;
		va_start(ap, &fmt);
		vsprintf(buf, fmt.c_str(), ap);
		va_end(ap);
		return buf;
	}

	inline std::wstring wsfmt(const std::wstring& fmt, ...)
	{
		static wchar_t buf[1024];
		va_list ap;
		va_start(ap, &fmt);
		vswprintf(buf, fmt.c_str(), ap);
		va_end(ap);
		return buf;
	}

	inline uint data_size(uint type_hash)
	{
		switch (type_hash)
		{
		case cH("bool"):
			return sizeof(bool);
		case cH("int"):
		case cH("uint"):
			return sizeof(int);
		case cH("Vec(1+int)"):
		case cH("Vec(1+uint)"):
			return sizeof(Vec1i);
		case cH("Vec(2+int)"):
		case cH("Vec(2+uint)"):
			return sizeof(Vec2i);
		case cH("Vec(3+int)"):
		case cH("Vec(3+uint)"):
			return sizeof(Vec3i);
		case cH("Vec(4+int)"):
		case cH("Vec(4+uint)"):
			return sizeof(Vec4i);
		case cH("longlong"):
		case cH("ulonglong"):
			return sizeof(longlong);
		case cH("float"):
			return sizeof(float);
		case cH("Vec(1+float)"):
			return sizeof(Vec1f);
		case cH("Vec(2+float)"):
			return sizeof(Vec2f);
		case cH("Vec(3+float)"):
			return sizeof(Vec3f);
		case cH("Vec(4+float)"):
			return sizeof(Vec4f);
		case cH("uchar"):
			return sizeof(uchar);
		case cH("Vec(1+uchar)"):
			return sizeof(Vec1c);
		case cH("Vec(2+uchar)"):
			return sizeof(Vec2c);
		case cH("Vec(3+uchar)"):
			return sizeof(Vec3c);
		case cH("Vec(4+uchar)"):
			return sizeof(Vec4c);
		case cH("StringA"):
			return sizeof(StringA);
		case cH("StringW"):
			return sizeof(StringW);
		default:
			assert(0);
		}
	}

	inline void data_copy(uint type_hash, const void* src, void* dst, uint size = 0)
	{
		switch (type_hash)
		{
		case cH("StringA"):
			*(StringA*)dst = *(StringA*)src;
			return;
		case cH("StringW"):
			*(StringW*)dst = *(StringW*)src;
			return;
		}

		memcpy(dst, src, size ? size : data_size(type_hash));
	}

	inline void data_dtor(uint type_hash, void* p)
	{
		switch (type_hash)
		{
		case cH("StringA"):
			((StringA*)p)->~String();
			return;
		case cH("StringW"):
			((StringW*)p)->~String();
			return;
		}
	}

	std::string TypeInfo::serialize(const std::vector<TypeinfoDatabase*>& dbs, const void* src, int precision) const
	{
		if (is_attribute())
			src = (char*)src + sizeof(AttributeBase);

		switch (tag())
		{
		case TypeEnumSingle:
		{
			auto e = find_enum(dbs, base_hash());
			assert(e);
			return e->find_item(*(int*)src)->name();
		}
		case TypeEnumMulti:
		{
			std::string str;
			auto e = find_enum(dbs, base_hash());
			assert(e);
			auto v = *(int*)src;
			for (auto i = 0; i < e->item_count(); i++)
			{
				if ((v & 1) == 1)
				{
					if (!str.empty())
						str += ";";
					str += e->find_item(1 << i)->name();
				}
				v >>= 1;
			}
			return str;
		}
		case TypeData:
			switch (base_hash())
			{
			case cH("bool"):
				return *(bool*)src ? "1" : "0";
			case cH("int"):
				return std::to_string(*(int*)src);
			case cH("Vec(1+int)"):
				return to_string(*(Vec1i*)src);
			case cH("Vec(2+int)"):
				return to_string(*(Vec2i*)src);
			case cH("Vec(3+int)"):
				return to_string(*(Vec3i*)src);
			case cH("Vec(4+int)"):
				return to_string(*(Vec4i*)src);
			case cH("uint"):
				return std::to_string(*(uint*)src);
			case cH("Vec(1+uint)"):
				return to_string(*(Vec1u*)src);
			case cH("Vec(2+uint)"):
				return to_string(*(Vec2u*)src);
			case cH("Vec(3+uint)"):
				return to_string(*(Vec3u*)src);
			case cH("Vec(4+uint)"):
				return to_string(*(Vec4u*)src);
			case cH("ulonglong"):
				return std::to_string(*(ulonglong*)src);
			case cH("float"):
				return to_string(*(float*)src, precision);
			case cH("Vec(1+float)"):
				return to_string(*(Vec1f*)src, precision);
			case cH("Vec(2+float)"):
				return to_string(*(Vec2f*)src, precision);
			case cH("Vec(3+float)"):
				return to_string(*(Vec3f*)src, precision);
			case cH("Vec(4+float)"):
				return to_string(*(Vec4f*)src, precision);
			case cH("uchar"):
				return std::to_string(*(uchar*)src);
			case cH("Vec(1+uchar)"):
				return to_string(*(Vec1c*)src);
			case cH("Vec(2+uchar)"):
				return to_string(*(Vec2c*)src);
			case cH("Vec(3+uchar)"):
				return to_string(*(Vec3c*)src);
			case cH("Vec(4+uchar)"):
				return to_string(*(Vec4c*)src);
			case cH("std::string"):
				return *(std::string*)src;
			case cH("std::wstring"):
				return w2s(*(std::wstring*)src);
			case cH("StringA"):
				return ((StringA*)src)->str();
			case cH("StringW"):
				return w2s(((StringW*)src)->str());
			default:
				assert(0);
			}
		}
	}

	void TypeInfo::unserialize(const std::vector<TypeinfoDatabase*>& dbs, const std::string& src, void* dst) const
	{
		if (is_attribute())
			dst = (char*)dst + sizeof(AttributeBase);

		switch (tag())
		{
		case TypeEnumSingle:
		{
			auto e = find_enum(dbs, base_hash());
			assert(e);
			e->find_item(src.c_str(), (int*)dst);
		}
			return;
		case TypeEnumMulti:
		{
			auto v = 0;
			auto e = find_enum(dbs, base_hash());
			assert(e);
			auto sp = ssplit(src, ';');
			for (auto& t : sp)
				v |= e->find_item(t.c_str())->value();
			*(int*)dst = v;
		}
			return;
		case TypeData:
			switch (base_hash())
			{
			case cH("bool"):
				*(bool*)dst = (src != "0");
				break;
			case cH("int"):
				*(int*)dst = std::stoi(src);
				break;
			case cH("Vec(1+int)"):
				*(Vec1u*)dst = std::stoi(src.c_str());
				break;
			case cH("Vec(2+int)"):
				*(Vec2u*)dst = stoi2(src.c_str());
				break;
			case cH("Vec(3+int)"):
				*(Vec3u*)dst = stoi3(src.c_str());
				break;
			case cH("Vec(4+int)"):
				*(Vec4u*)dst = stoi4(src.c_str());
				break;
			case cH("uint"):
				*(uint*)dst = std::stoul(src);
				break;
			case cH("Vec(1+uint)"):
				*(Vec1u*)dst = std::stoul(src.c_str());
				break;
			case cH("Vec(2+uint)"):
				*(Vec2u*)dst = stou2(src.c_str());
				break;
			case cH("Vec(3+uint)"):
				*(Vec3u*)dst = stou3(src.c_str());
				break;
			case cH("Vec(4+uint)"):
				*(Vec4u*)dst = stou4(src.c_str());
				break;
			case cH("ulonglong"):
				*(ulonglong*)dst = std::stoull(src);
				break;
			case cH("float"):
				*(float*)dst = std::stof(src.c_str());
				break;
			case cH("Vec(1+float)"):
				*(Vec1f*)dst = std::stof(src.c_str());
				break;
			case cH("Vec(2+float)"):
				*(Vec2f*)dst = stof2(src.c_str());
				break;
			case cH("Vec(3+float)"):
				*(Vec3f*)dst = stof3(src.c_str());
				break;
			case cH("Vec(4+float)"):
				*(Vec4f*)dst = stof4(src.c_str());
				break;
			case cH("uchar"):
				*(uchar*)dst = std::stoul(src);
				break;
			case cH("Vec(1+uchar)"):
				*(Vec1c*)dst = std::stoul(src.c_str());
				break;
			case cH("Vec(2+uchar)"):
				*(Vec2c*)dst = stoc2(src.c_str());
				break;
			case cH("Vec(3+uchar)"):
				*(Vec3c*)dst = stoc3(src.c_str());
				break;
			case cH("Vec(4+uchar)"):
				*(Vec4c*)dst = stoc4(src.c_str());
				break;
			case cH("StringA"):
				*(StringA*)dst = src;
				break;
			case cH("StringW"):
				*(StringW*)dst = s2w(src);
				break;
			default:
				assert(0);
			}
			return;
		}
	}

	void TypeInfo::copy_from(const void* src, void* dst) const
	{
		if (is_attribute())
			dst = (char*)dst + sizeof(AttributeBase);

		if (tag() == TypeData)
			data_copy(base_hash(), src, dst);
		else if (tag() == TypeEnumSingle || tag() == TypeEnumMulti)
			memcpy(dst, src, sizeof(int));
		else if (tag() == TypePointer)
			memcpy(dst, src, sizeof(void*));
	}

	/*
	StringA SerializableNode::to_xml_string(SerializableNode* n)
	{
		struct xml_string_writer : pugi::xml_writer
		{
			std::string str;

			virtual void write(const void* data, size_t size)
			{
				str.append((const char*)(data), size);
			}
		};
		xml_string_writer writer;
		doc.print(writer);
	}

	static void from_json(nlohmann::json::reference src, SerializableNode* dst)
	{
		if (src.is_object())
		{
			dst->set_type(SerializableNode::Object);
			for (auto it = src.begin(); it != src.end(); ++it)
			{
				auto c = it.value();
				if (!c.is_object() && !c.is_array())
					dst->new_attr(it.key(), c.is_string() ? c.get<std::string>() : c.dump());
				else
				{
					auto node = dst->new_node(it.key());
					from_json(c, node);
				}
			}
		}
		else if (src.is_array())
		{
			dst->set_type(SerializableNode::Array);
			for (auto& n : src)
			{
				auto node = dst->new_node("");
				from_json(n, node);
			}
		}
	}

	static void to_json(nlohmann::json::reference dst, SerializableNodePrivate* src)
	{
		if (src->type != SerializableNode::Array)
		{
			if (src->type == SerializableNode::Value && !src->attrs.empty())
				dst = src->attrs[0]->value;
			else if (!src->value.empty())
				dst = src->value;
			else
			{
				for (auto& sa : src->attrs)
					dst[sa->name] = sa->value;
			}

			for (auto& sn : src->nodes)
				to_json(dst[sn->name], sn.get());
		}
		else
		{
			for (auto i = 0; i < src->nodes.size(); i++)
				to_json(dst[i], src->nodes[i].get());
		}
	}

	SerializableNode* SerializableNode::create_from_json_string(const std::string& str)
	{
		auto doc = nlohmann::json::parse(str);

		auto n = new SerializableNodePrivate;

		from_json(doc, n);

		return n;
	}

	SerializableNode* SerializableNode::create_from_json_file(const std::wstring& filename)
	{
		auto str = get_file_string(filename);
		if (str.empty())
			return nullptr;

		return create_from_json_string(str);
	}

	StringA SerializableNode::to_json_string(SerializableNode* n)
	{
		nlohmann::json doc;

		to_json(doc, (SerializableNodePrivate*)n);

		return StringA(doc.dump());
	}

	void SerializableNode::save_to_json_file(SerializableNode* n, const std::wstring& filename)
	{
		std::ofstream file(filename);
		nlohmann::json doc;

		to_json(doc, (SerializableNodePrivate*)n);

		file << doc.dump(2);
		file.close();
	}
	*/
}

