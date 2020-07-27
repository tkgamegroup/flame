#pragma once

#include <flame/math.h>

#include <pugixml.hpp>
#include <nlohmann/json.hpp>

#include <regex>
#include <locale>
#include <codecvt>
#include <sstream>
#include <fstream>
#include <iomanip>

namespace flame
{
	inline int fmt(char* buf, int buf_size, bool v)
	{
		return sprintf_s(buf, buf_size, "%s", v ? "true" : "false");
	}

	inline int fmt(char* buf, int buf_size, uchar v)
	{
		return sprintf_s(buf, buf_size, "%d", v);
	}

	inline int fmt(char* buf, int buf_size, int v)
	{
		return sprintf_s(buf, buf_size, "%d", v);
	}

	inline int fmt(char* buf, int buf_size, uint v)
	{
		return sprintf_s(buf, buf_size, "%d", v);
	}

	inline int fmt(char* buf, int buf_size, int64 v)
	{
		return sprintf_s(buf, buf_size, "%lld", v);
	}

	inline int fmt(char* buf, int buf_size, uint64 v)
	{
		return sprintf_s(buf, buf_size, "%llu", v);
	}

	inline int fmt(char* buf, int buf_size, float v)
	{
		auto ret = sprintf_s(buf, buf_size, "%.6f", v);
		if (ret > 0)
		{
			for (ret--; ret >= 0; ret--)
			{
				auto ch = buf[ret];
				if (ch == '.')
				{
					buf[ret + 1] = '0';
					ret += 2;
					break;
				}
				if (ch == '0')
					buf[ret] = 0;
				else
				{
					ret += 1;
					break;
				}
			}
		}
		return ret;
	}

	template <uint N, class T>
	inline int fmt(char* buf, int buf_size, const Vec<N, T>& v)
	{
		auto p = buf;
		auto s = buf_size;
		auto ret = 0;
		for (auto i = 0; i < N; i++)
		{
			ret += fmt(p, s, v[i]);
			p += ret;
			s -= ret;
			if (i < N - 1)
			{
				*p = ',';
				p++;
				s--;
			}
		}
		return ret;
	}

	template <class T>
	inline std::string to_string(T v)
	{
		char buf[32];
		fmt(buf, sizeof(buf), v);
		return buf;
	}

	template <uint N, class T>
	inline std::string to_string(const Vec<N, T>& v)
	{
		char buf[32];
		fmt(buf, sizeof(buf), v);
		return buf;
	}

	inline int fmt(wchar_t* buf, int buf_size, bool v)
	{
		return swprintf_s(buf, buf_size, L"%d", v ? L"1" : L"0");
	}

	inline int fmt(wchar_t* buf, int buf_size, uchar v)
	{
		return swprintf_s(buf, buf_size, L"%d", v);
	}

	inline int fmt(wchar_t* buf, int buf_size, int v)
	{
		return swprintf_s(buf, buf_size, L"%d", v);
	}

	inline int fmt(wchar_t* buf, int buf_size, uint v)
	{
		return swprintf_s(buf, buf_size, L"%d", v);
	}

	inline int fmt(wchar_t* buf, int buf_size, int64 v)
	{
		return swprintf_s(buf, buf_size, L"%lld", v);
	}

	inline int fmt(wchar_t* buf, int buf_size, uint64 v)
	{
		return swprintf_s(buf, buf_size, L"%llu", v);
	}

	inline int fmt(wchar_t* buf, int buf_size, float v)
	{
		auto ret = swprintf_s(buf, buf_size, L"%.6f", v);
		if (ret > 0)
		{
			for (ret--; ret >= 0; ret--)
			{
				auto ch = buf[ret];
				if (ch == '.')
				{
					buf[ret + 1] = '0';
					ret += 2;
					break;
				}
				if (ch == '0')
					buf[ret] = 0;
				else
				{
					ret += 1;
					break;
				}
			}
		}
		return ret;
	}

	template <uint N, class T>
	inline int fmt(wchar_t* buf, int buf_size, const Vec<N, T>& v)
	{
		auto p = buf;
		auto s = buf_size;
		auto ret = 0;
		for (auto i = 0; i < N; i++)
		{
			ret += fmt(p, s, v[i]);
			p += ret;
			s -= ret;
			if (i < N - 1)
			{
				*p = ',';
				p++;
				s--;
			}
		}
		return ret;
	}

	template <class T>
	inline std::wstring to_wstring(T v)
	{
		wchar_t buf[20];
		fmt(buf, sizeof(buf), v);
		return buf;
	}

	template <uint N, class T>
	inline std::wstring to_wstring(const Vec<N, T>& v)
	{
		wchar_t buf[20];
		fmt(buf, sizeof(buf), v);
		return buf;
	}

	template <class T>
	T sto(const char* s);

	template <>
	inline char sto<char>(const char* s)
	{
		return std::stoul(s);
	}

	template <>
	inline uchar sto<uchar>(const char* s)
	{
		return std::stoul(s);
	}

	template <>
	inline wchar_t sto<wchar_t>(const char* s)
	{
		return std::stoul(s);
	}

	template <>
	inline int sto<int>(const char* s)
	{
		return std::stoi(s);
	}

	template <>
	inline uint sto<uint>(const char* s)
	{
		return std::stoul(s);
	}

	template <>
	inline int64 sto<int64>(const char* s)
	{
		return std::stoll(s);
	}

	template <>
	inline uint64 sto<uint64>(const char* s)
	{
		return std::stoull(s);
	}

	template <>
	inline float sto<float>(const char* s)
	{
		return std::stof(s);
	}

	template <>
	inline Vec1c sto<Vec1c>(const char* s)
	{
		Vec1i ret;
		sscanf(s, "%d", &ret.x());
		return Vec1c(ret);
	}

	template <>
	inline Vec2c sto<Vec2c>(const char* s)
	{
		Vec2i ret;
		sscanf(s, "%d,%d", &ret.x(), &ret.y());
		return Vec2c(ret);
	}

	template <>
	inline Vec3c sto<Vec3c>(const char* s)
	{
		Vec3i ret;
		sscanf(s, "%d,%d,%d", &ret.x(), &ret.y(), &ret.z());
		return Vec3c(ret);
	}

	template <>
	inline Vec4c sto<Vec4c>(const char* s)
	{
		Vec4i ret;
		sscanf(s, "%d,%d,%d,%d", &ret.x(), &ret.y(), &ret.z(), &ret.w());
		return Vec4c(ret);
	}

	template <>
	inline Vec1i sto<Vec1i>(const char* s)
	{
		Vec1i ret;
		sscanf(s, "%d", &ret.x());
		return ret;
	}

	template <>
	inline Vec2i sto<Vec2i>(const char* s)
	{
		Vec2i ret;
		sscanf(s, "%d,%d", &ret.x(), &ret.y());
		return ret;
	}

	template <>
	inline Vec3i sto<Vec3i>(const char* s)
	{
		Vec3i ret;
		sscanf(s, "%d,%d,%d", &ret.x(), &ret.y(), &ret.z());
		return ret;
	}

	template <>
	inline Vec4i sto<Vec4i>(const char* s)
	{
		Vec4i ret;
		sscanf(s, "%d,%d,%d,%d", &ret.x(), &ret.y(), &ret.z(), &ret.w());
		return ret;
	}

	template <>
	inline Vec1u sto<Vec1u>(const char* s)
	{
		Vec1u ret;
		sscanf(s, "%u", &ret.x());
		return ret;
	}

	template <>
	inline Vec2u sto<Vec2u>(const char* s)
	{
		Vec2u ret;
		sscanf(s, "%u,%u", &ret.x(), &ret.y());
		return ret;
	}

	template <>
	inline Vec3u sto<Vec3u>(const char* s)
	{
		Vec3u ret;
		sscanf(s, "%u,%u,%u", &ret.x(), &ret.y(), &ret.z());
		return ret;
	}

	template <>
	inline Vec4u sto<Vec4u>(const char* s)
	{
		Vec4u ret;
		sscanf(s, "%u,%u,%u,%u", &ret.x(), &ret.y(), &ret.z(), &ret.w());
		return ret;
	}

	template <>
	inline Vec1f sto<Vec1f>(const char* s)
	{
		Vec1f ret;
		sscanf(s, "%f", &ret.x());
		return ret;
	}

	template <>
	inline Vec2f sto<Vec2f>(const char* s)
	{
		Vec2f ret;
		sscanf(s, "%f,%f", &ret.x(), &ret.y());
		return ret;
	}

	template <>
	inline Vec3f sto<Vec3f>(const char* s)
	{
		Vec3f ret;
		sscanf(s, "%f,%f,%f", &ret.x(), &ret.y(), &ret.z());
		return ret;
	}

	template <>
	inline Vec4f sto<Vec4f>(const char* s)
	{
		Vec4f ret;
		sscanf(s, "%f,%f,%f,%f", &ret.x(), &ret.y(), &ret.z(), &ret.w());
		return ret;
	}

	template <class T>
	T sto(const wchar_t* s);

	template <>
	inline uchar sto<uchar>(const wchar_t* s)
	{
		return std::stoul(s);
	}

	template <>
	inline int sto<int>(const wchar_t* s)
	{
		return std::stoi(s);
	}

	template <>
	inline uint sto<uint>(const wchar_t* s)
	{
		return std::stoul(s);
	}

	template <>
	inline int64 sto<int64>(const wchar_t* s)
	{
		return std::stoll(s);
	}

	template <>
	inline uint64 sto<uint64>(const wchar_t* s)
	{
		return std::stoull(s);
	}

	template <>
	inline float sto<float>(const wchar_t* s)
	{
		return std::stof(s);
	}

	template <>
	inline Vec1c sto<Vec1c>(const wchar_t* s)
	{
		Vec1i ret;
		swscanf(s, L"%d", &ret.x());
		return Vec1c(ret);
	}

	template <>
	inline Vec2c sto<Vec2c>(const wchar_t* s)
	{
		Vec2i ret;
		swscanf(s, L"%d,%d", &ret.x(), &ret.y());
		return Vec2c(ret);
	}

	template <>
	inline Vec3c sto<Vec3c>(const wchar_t* s)
	{
		Vec3i ret;
		swscanf(s, L"%d,%d,%d", &ret.x(), &ret.y(), &ret.z());
		return Vec3c(ret);
	}

	template <>
	inline Vec4c sto<Vec4c>(const wchar_t* s)
	{
		Vec4i ret;
		swscanf(s, L"%d,%d,%d,%d", &ret.x(), &ret.y(), &ret.z(), &ret.w());
		return Vec4c(ret);
	}

	template <>
	inline Vec1i sto<Vec1i>(const wchar_t* s)
	{
		Vec1i ret;
		swscanf(s, L"%d", &ret.x());
		return ret;
	}

	template <>
	inline Vec2i sto<Vec2i>(const wchar_t* s)
	{
		Vec2i ret;
		swscanf(s, L"%d,%d", &ret.x(), &ret.y());
		return ret;
	}

	template <>
	inline Vec3i sto<Vec3i>(const wchar_t* s)
	{
		Vec3i ret;
		swscanf(s, L"%d,%d,%d", &ret.x(), &ret.y(), &ret.z());
		return ret;
	}

	template <>
	inline Vec4i sto<Vec4i>(const wchar_t* s)
	{
		Vec4i ret;
		swscanf(s, L"%d,%d,%d,%d", &ret.x(), &ret.y(), &ret.z(), &ret.w());
		return ret;
	}

	template <>
	inline Vec1u sto<Vec1u>(const wchar_t* s)
	{
		Vec1u ret;
		swscanf(s, L"%u", &ret.x());
		return ret;
	}

	template <>
	inline Vec2u sto<Vec2u>(const wchar_t* s)
	{
		Vec2u ret;
		swscanf(s, L"%u,%u", &ret.x(), &ret.y());
		return ret;
	}

	template <>
	inline Vec3u sto<Vec3u>(const wchar_t* s)
	{
		Vec3u ret;
		swscanf(s, L"%u,%u,%u", &ret.x(), &ret.y(), &ret.z());
		return ret;
	}

	template <>
	inline Vec4u sto<Vec4u>(const wchar_t* s)
	{
		Vec4u ret;
		swscanf(s, L"%u,%u,%u,%u", &ret.x(), &ret.y(), &ret.z(), &ret.w());
		return ret;
	}

	template <>
	inline Vec1f sto<Vec1f>(const wchar_t* s)
	{
		Vec1f ret;
		swscanf(s, L"%f", &ret.x());
		return ret;
	}

	template <>
	inline Vec2f sto<Vec2f>(const wchar_t* s)
	{
		Vec2f ret;
		swscanf(s, L"%f,%f", &ret.x(), &ret.y());
		return ret;
	}

	template <>
	inline Vec3f sto<Vec3f>(const wchar_t* s)
	{
		Vec3f ret;
		swscanf(s, L"%f,%f,%f", &ret.x(), &ret.y(), &ret.z());
		return ret;
	}

	template <>
	inline Vec4f sto<Vec4f>(const wchar_t* s)
	{
		Vec4f ret;
		swscanf(s, L"%f,%f,%f,%f", &ret.x(), &ret.y(), &ret.z(), &ret.w());
		return ret;
	}

	template <class T>
	T sto_t(const wchar_t* s)
	{
		try
		{
			return sto<T>(s);
		}
		catch (...)
		{
			return 0;
		}
	}

	template <class CH>
	struct StrUtils
	{
		static std::basic_string<CH> trim(const std::basic_string<CH>& str)
		{
			auto begin = 0;
			auto end = (int)str.size();
			for (; begin < str.size(); begin++)
			{
				auto ch = str[begin];
				if (ch != ' ' && ch != '\t')
					break;
			}
			for (; end > 0; end--)
			{
				auto ch = str[end - 1];
				if (ch != ' ' && ch != '\t' && ch != '\r')
					break;
			}
			if (begin >= end)
				return "";
			return str.substr(begin, end);
		}

		static std::vector<std::basic_string<CH>> split(const std::basic_string<CH>& str, CH delimiter = ' ')
		{
			std::basic_istringstream<CH> iss(str);
			std::vector<std::basic_string<CH>> ret;

			std::basic_string<CH> s;
			while (std::getline(iss, s, delimiter))
				ret.push_back(s);

			return ret;
		}

		static std::vector<std::basic_string<CH>> split_lastone(const std::basic_string<CH>& str, CH delimiter = ' ')
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

		static std::vector<std::basic_string<CH>> split_dbnull(const CH* str)
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

		static std::vector<std::basic_string<CH>> split_regex(const std::basic_string<CH>& str, const std::basic_regex<CH>& reg, uint req_idx = 0)
		{
			std::vector<std::basic_string<CH>> ret;

			std::match_results<typename std::basic_string<CH>::const_iterator> res;
			auto s = str;

			while (std::regex_search(s, res, reg))
			{
				ret.push_back(res[req_idx]);
				s = res.suffix();
			}

			return ret;
		}

		static void remove_ch(std::basic_string<CH>& str, CH ch = ' ')
		{
			str.erase(std::remove(str.begin(), str.end(), ch), str.end());
		}

		static void remove_spaces(std::basic_string<CH>& str)
		{
			remove_ch(str, ' ');
			remove_ch(str, '\t');
		}
	};

	using SUS = StrUtils<char>;
	using SUW = StrUtils<wchar_t>;

	inline std::string sfmt(const char* fmt, ...)
	{
		static char buf[1024];
		va_list ap;
		va_start(ap, &fmt);
		vsprintf(buf, fmt, ap);
		va_end(ap);
		return buf;
	}

	inline std::wstring wfmt(const wchar_t* fmt, ...)
	{
		static wchar_t buf[1024];
		va_list ap;
		va_start(ap, &fmt);
		vswprintf(buf, fmt, ap);
		va_end(ap);
		return buf;
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

#ifdef FLAME_WINDOWS
	inline std::string translate(const char* src_locale, const char* dst_locale, const std::string& src)
	{
		std::wstring_convert<std::codecvt_byname<wchar_t, char, mbstate_t>>
			cv1(new std::codecvt_byname<wchar_t, char, mbstate_t>(src_locale));
		std::wstring_convert<std::codecvt_byname<wchar_t, char, mbstate_t>>
			cv2(new std::codecvt_byname<wchar_t, char, mbstate_t>(dst_locale));
		return cv2.to_bytes(cv1.from_bytes(src));
	}

	inline std::string japanese_to_chinese(const std::string& src)
	{
		return translate(".932", ".936", src);
	}
#endif

	enum FileType
	{
		FileTypeUnknown,
		FileTypeFolder,
		FileTypeText,
		FileTypeImage,
		FileTypeModel
	};

	inline bool is_text_file(const std::wstring& _ext)
	{
		auto ext = _ext;
		std::transform(ext.begin(), ext.end(), ext.begin(), tolower);
		if (ext == L".txt" ||
			ext == L".h" || ext == L".c" || ext == L".cpp" || ext == L".hpp" || ext == L".cxx" || ext == L".inl" ||
			ext == L".glsl" || ext == L".vert" || ext == L".tesc" || ext == L".tese" || ext == L".geom" || ext == L".frag" || ext == L".hlsl" ||
			ext == L".xml" || ext == L".json" || ext == L".ini" || ext == L".log" ||
			ext == L".htm" || ext == L".html" || ext == L".css" ||
			ext == L".sln" || ext == L".vcxproj")
			return true;
		return false;
	}

	inline bool is_image_file(const std::wstring& _ext)
	{
		auto ext = _ext;
		std::transform(ext.begin(), ext.end(), ext.begin(), tolower);
		if (ext == L".bmp" || ext == L".jpg" || ext == L".jpeg" || ext == L".png" || ext == L".gif" ||
			ext == L".tga" || ext == L".dds" || ext == L".ktx")
			return true;
		return false;
	}

	inline bool is_model_file(const std::wstring& _ext)
	{
		auto ext = _ext;
		std::transform(ext.begin(), ext.end(), ext.begin(), tolower);
		if (ext == L".obj" || ext == L".pmd" || ext == L".pmx" || ext == L".dae")
			return true;
		return false;
	}

	inline FileType get_file_type(const std::wstring& ext)
	{
		if (is_text_file(ext))
			return FileTypeText;
		if (is_image_file(ext))
			return FileTypeImage;
		if (is_model_file(ext))
			return FileTypeModel;
		return FileTypeUnknown;
	}

	template <class T>
	inline T read(std::ifstream& file)
	{
		T v;
		file.read((char*)&v, sizeof(T));
		return v;
	}

	inline std::string read_string(std::ifstream& file)
	{
		uint size = 0;
		uint q = 1;
		for (auto i = 0; i < 4; i++)
		{
			unsigned char byte;
			file.read((char*)&byte, 1);
			if (byte >= 128)
				byte -= 128;
			else
				i = 4;
			size += q * byte;
			q *= 128;
		}
		std::string v;
		v.resize(size);
		file.read((char*)v.data(), size);
		return v;
	}

	template <class T>
	inline void write(std::ofstream& file, const T& v)
	{
		file.write((char*)&v, sizeof(T));
	}

	inline void write_string(std::ofstream& file, const std::string& v)
	{
		uint size = v.size();
		for (auto i = 0; i < 4; i++)
		{
			unsigned char byte = size % 128;
			size /= 128;
			if (size > 0)
				byte += 128;
			else
				i = 4;
			file.write((char*)&byte, 1);

		}
		file.write((char*)v.data(), v.size());
	}

	inline int64 get_file_length(std::ifstream& f)
	{
		f.seekg(0, std::ios::end);
		auto s = f.tellg();
		f.seekg(0, std::ios::beg);
		return s;
	}

	inline std::string get_file_content(const std::filesystem::path& filename)
	{
#ifdef FLAME_WINDOWS
		std::ifstream file(filename, std::ios::binary);
#else
		std::ifstream file(w2s(filename), std::ios::binary);
#endif
		if (!file.good())
			return "";

		auto length = get_file_length(file);
		std::string ret;
		ret.resize(length);
		file.read(ret.data(), length);
		file.close();
		return ret;
	}

	struct INI_Entry
	{
		std::string key;
		std::string value;
	};

	struct INI_Section
	{
		std::string name;
		std::vector<INI_Entry> entries;
	};

	struct INI_File
	{
		std::vector<INI_Section> sections;

		const std::vector<INI_Entry>& get_section_entries(const std::string& name)
		{
			for (auto& s : sections)
			{
				if (s.name == name)
					return s.entries;
			}
			return std::vector<INI_Entry>();
		}
	};

	inline INI_File parse_ini_file(const std::filesystem::path& filename)
	{
		INI_File ret;

		std::ifstream file(filename);
		if (!file.good())
			return ret;
		while (!file.eof())
		{
			std::string line;
			std::getline(file, line);
			if (!line.empty() && line[0] != ';')
			{
				line = SUS::trim(line);
				if (line.size() > 2 && line.front() == '[' && line.back() == ']')
				{
					INI_Section section;
					section.name = std::string(line.begin() + 1, line.end() - 1);
					ret.sections.push_back(section);
				}
				else
				{
					if (ret.sections.empty())
					{
						INI_Section section;
						ret.sections.push_back(section);
					}

					static std::regex reg_pair(R"(^([\*\w]+)\s*=(.*))");
					static std::regex reg_quot(R"(^\"(.*)\"$)");
					std::smatch res;
					INI_Entry entry;
					if (std::regex_search(line, res, reg_pair))
					{
						entry.key = res[1].str();
						entry.value = res[2].str();
					}
					else
						entry.value = line;
					entry.value = SUS::trim(entry.value);
					if (std::regex_search(entry.value, res, reg_quot))
						entry.value = res[1].str();

					ret.sections.back().entries.push_back(entry);
				}
			}
		}
		file.close();

		return ret;
	}

	// baseXX encoding is from https://github.com/r-lyeh-archived/base

	inline std::string base85_encode(const uchar* data, uint length)
	{
		assert(length % 4 == 0);

		const char encoder[86] =
			"0123456789" "abcdefghij" "klmnopqrst" "uvwxyzABCD"             // 00..39
			"EFGHIJKLMN" "OPQRSTUVWX" "YZ.-:+=^!/" "*?&<>()[]{" "}@%$#";    // 40..84 // free chars: , ; _ ` | ~ \'

		std::string ret;
		ret.resize(((length + 3) / 4) * 5);
		auto dst = &ret[0];

		for (auto i = 0; i < length; i += 4)
		{
			auto d = *(uint*)(data + i);
			for (auto j = 0; j < 5; j++)
			{
				*dst = encoder[d];
				dst++;
				d /= 85;
			}
		}

		return ret;
	}

	inline void base85_decode(const std::string& str, uchar* dst) // dst length is str length * 4 / 5
	{
		assert(str.length() % 5 == 0);

		const unsigned char decoder[128] = {
				0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, // 0x00..0x0F
				0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, // 0x10..0x1F
				0, 68,  0, 84, 83, 82, 72,  0, 75, 76, 70, 65,  0, 63, 62, 69, // 0x20..0x2F
				0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 64,  0, 73, 66, 74, 71, // 0x30..0x3F
			   81, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, // 0x40..0x4F
			   51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 77,  0, 78, 67,  0, // 0x50..0x5F
				0, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, // 0x60..0x6F
			   25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 79,  0, 80,  0,  0, // 0x70..0x7F
		};

		auto src = &str[0];

		auto dst_off = 0;
		while (*src)
		{
			uint v = decoder[src[0]] +
				85 * (decoder[src[1]] +
					85 * (decoder[src[2]] +
						85 * (decoder[src[3]] +
							85 * decoder[src[4]])));
			for (auto i = 0; i < 4; i++)
			{
				dst[dst_off + i] = v & 0xff;
				v = (v >> 8);
			}
			src += 5;
			dst_off += 4;
		}
	}

	inline std::string base64_encode(const std::string& text)
	{
		std::string out;

		const std::string chars =
			"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
			"abcdefghijklmnopqrstuvwxyz"
			"0123456789+/";

		unsigned char const* bytes_to_encode = (unsigned char const*)text.c_str();
		unsigned int in_len = (unsigned int)text.size();
		unsigned int i = 0;
		unsigned int j = 0;
		unsigned char char_array_3[3];
		unsigned char char_array_4[4];

		while (in_len--) {
			char_array_3[i++] = *(bytes_to_encode++);
			if (i == 3) {
				char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
				char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
				char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
				char_array_4[3] = char_array_3[2] & 0x3f;

				for (i = 0; (i < 4); i++)
					out += chars[char_array_4[i]];
				i = 0;
			}
		}

		if (i) {
			for (j = i; j < 3; j++)
				char_array_3[j] = '\0';

			char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
			char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
			char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
			char_array_4[3] = char_array_3[2] & 0x3f;

			for (j = 0; (j < i + 1); j++)
				out += chars[char_array_4[j]];

			while ((i++ < 3))
				out += '=';
		}

		return out;
	}

	inline std::string base64_decode(const std::string& encoded)
	{
		std::string out;

		const std::string chars =
			"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
			"abcdefghijklmnopqrstuvwxyz"
			"0123456789+/";

		unsigned int in_len = (unsigned int)encoded.size();
		unsigned int i = 0;
		unsigned int j = 0;
		unsigned int in_ = 0;
		unsigned char char_array_4[4], char_array_3[3];
		out.clear();

		while (in_len-- && (encoded[in_] != '=') && //is_base64(encoded[in_])) {
			(isalnum(encoded[in_]) || encoded[in_] == '+' || encoded[in_] == '/')) {
			char_array_4[i++] = encoded[in_]; in_++;
			if (i == 4) {
				for (i = 0; i < 4; i++)
					char_array_4[i] = chars.find(char_array_4[i]);

				char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
				char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
				char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

				for (i = 0; (i < 3); i++)
					out += char_array_3[i];
				i = 0;
			}
		}

		if (i) {
			for (j = i; j < 4; j++)
				char_array_4[j] = 0;

			for (j = 0; j < 4; j++)
				char_array_4[j] = chars.find(char_array_4[j]);

			char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
			char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
			char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

			for (j = 0; (j < i - 1); j++) out += char_array_3[j];
		}

		return out;
	}

	// SHA1 is from https://github.com/vog/sha1

	struct SHA1
	{
		uint digest[5];
		std::string buffer;
		uint64 transforms;

		enum { BLOCK_INTS = 16, BLOCK_BYTES = BLOCK_INTS * 4 };

		SHA1()
		{
			reset();
		}

		void reset()
		{
			digest[0] = 0x67452301;
			digest[1] = 0xefcdab89;
			digest[2] = 0x98badcfe;
			digest[3] = 0x10325476;
			digest[4] = 0xc3d2e1f0;

			buffer = "";
			transforms = 0;
		}

		void update(const std::string& s)
		{
			std::istringstream is(s);
			while (true)
			{
				char sbuf[BLOCK_BYTES];
				is.read(sbuf, BLOCK_BYTES - buffer.size());
				buffer.append(sbuf, (std::size_t)is.gcount());
				if (buffer.size() != BLOCK_BYTES)
				{
					return;
				}
				uint block[BLOCK_INTS];
				buffer_to_block(buffer, block);
				transform(digest, block, transforms);
				buffer.clear();
			}
		}

		void _final()
		{
			uint64 total_bits = (transforms * BLOCK_BYTES + buffer.size()) * 8;

			buffer += (char)0x80;
			size_t orig_size = buffer.size();
			while (buffer.size() < BLOCK_BYTES)
				buffer += (char)0x00;

			uint block[BLOCK_INTS];
			buffer_to_block(buffer, block);

			if (orig_size > BLOCK_BYTES - 8)
			{
				transform(digest, block, transforms);
				for (size_t i = 0; i < BLOCK_INTS - 2; i++)
					block[i] = 0;
			}

			block[BLOCK_INTS - 1] = (uint)total_bits;
			block[BLOCK_INTS - 2] = (uint)(total_bits >> 32);
			transform(digest, block, transforms);
		}

		std::string final_bin()
		{
			_final();

			std::string result;
			result.resize(sizeof(uint) * size(digest));
			auto dst = &result[0];
			for (auto i = 0; i < size(digest); i++)
			{
				for (auto j = 0; j < 4; j++)
					dst[j] = ((char*)(&digest[i]))[4 - j - 1];
				dst += 4;
			}

			reset();

			return result;
		}

		std::string final_str()
		{
			_final();

			std::ostringstream result;
			for (auto i = 0; i < size(digest); i++)
			{
				result << std::hex << std::setfill('0') << std::setw(8);
				result << digest[i];
			}

			reset();

			return result.str();
		}

		static uint rol(const uint value, const size_t bits)
		{
			return (value << bits) | (value >> (32 - bits));
		}


		static uint blk(const uint block[BLOCK_INTS], const size_t i)
		{
			return rol(block[(i + 13) & 15] ^ block[(i + 8) & 15] ^ block[(i + 2) & 15] ^ block[i], 1);
		}

		static void R0(const uint block[BLOCK_INTS], const uint v, uint& w, const uint x, const uint y, uint& z, const size_t i)
		{
			z += ((w & (x ^ y)) ^ y) + block[i] + 0x5a827999 + rol(v, 5);
			w = rol(w, 30);
		}

		static void R1(uint block[BLOCK_INTS], const uint v, uint& w, const uint x, const uint y, uint& z, const size_t i)
		{
			block[i] = blk(block, i);
			z += ((w & (x ^ y)) ^ y) + block[i] + 0x5a827999 + rol(v, 5);
			w = rol(w, 30);
		}

		static void R2(uint block[BLOCK_INTS], const uint v, uint& w, const uint x, const uint y, uint& z, const size_t i)
		{
			block[i] = blk(block, i);
			z += (w ^ x ^ y) + block[i] + 0x6ed9eba1 + rol(v, 5);
			w = rol(w, 30);
		}

		static void R3(uint block[BLOCK_INTS], const uint v, uint& w, const uint x, const uint y, uint& z, const size_t i)
		{
			block[i] = blk(block, i);
			z += (((w | x) & y) | (w & x)) + block[i] + 0x8f1bbcdc + rol(v, 5);
			w = rol(w, 30);
		}

		static void R4(uint block[BLOCK_INTS], const uint v, uint& w, const uint x, const uint y, uint& z, const size_t i)
		{
			block[i] = blk(block, i);
			z += (w ^ x ^ y) + block[i] + 0xca62c1d6 + rol(v, 5);
			w = rol(w, 30);
		}

		static void transform(uint digest[], uint block[BLOCK_INTS], uint64& transforms)
		{
			uint a = digest[0];
			uint b = digest[1];
			uint c = digest[2];
			uint d = digest[3];
			uint e = digest[4];

			R0(block, a, b, c, d, e, 0);
			R0(block, e, a, b, c, d, 1);
			R0(block, d, e, a, b, c, 2);
			R0(block, c, d, e, a, b, 3);
			R0(block, b, c, d, e, a, 4);
			R0(block, a, b, c, d, e, 5);
			R0(block, e, a, b, c, d, 6);
			R0(block, d, e, a, b, c, 7);
			R0(block, c, d, e, a, b, 8);
			R0(block, b, c, d, e, a, 9);
			R0(block, a, b, c, d, e, 10);
			R0(block, e, a, b, c, d, 11);
			R0(block, d, e, a, b, c, 12);
			R0(block, c, d, e, a, b, 13);
			R0(block, b, c, d, e, a, 14);
			R0(block, a, b, c, d, e, 15);
			R1(block, e, a, b, c, d, 0);
			R1(block, d, e, a, b, c, 1);
			R1(block, c, d, e, a, b, 2);
			R1(block, b, c, d, e, a, 3);
			R2(block, a, b, c, d, e, 4);
			R2(block, e, a, b, c, d, 5);
			R2(block, d, e, a, b, c, 6);
			R2(block, c, d, e, a, b, 7);
			R2(block, b, c, d, e, a, 8);
			R2(block, a, b, c, d, e, 9);
			R2(block, e, a, b, c, d, 10);
			R2(block, d, e, a, b, c, 11);
			R2(block, c, d, e, a, b, 12);
			R2(block, b, c, d, e, a, 13);
			R2(block, a, b, c, d, e, 14);
			R2(block, e, a, b, c, d, 15);
			R2(block, d, e, a, b, c, 0);
			R2(block, c, d, e, a, b, 1);
			R2(block, b, c, d, e, a, 2);
			R2(block, a, b, c, d, e, 3);
			R2(block, e, a, b, c, d, 4);
			R2(block, d, e, a, b, c, 5);
			R2(block, c, d, e, a, b, 6);
			R2(block, b, c, d, e, a, 7);
			R3(block, a, b, c, d, e, 8);
			R3(block, e, a, b, c, d, 9);
			R3(block, d, e, a, b, c, 10);
			R3(block, c, d, e, a, b, 11);
			R3(block, b, c, d, e, a, 12);
			R3(block, a, b, c, d, e, 13);
			R3(block, e, a, b, c, d, 14);
			R3(block, d, e, a, b, c, 15);
			R3(block, c, d, e, a, b, 0);
			R3(block, b, c, d, e, a, 1);
			R3(block, a, b, c, d, e, 2);
			R3(block, e, a, b, c, d, 3);
			R3(block, d, e, a, b, c, 4);
			R3(block, c, d, e, a, b, 5);
			R3(block, b, c, d, e, a, 6);
			R3(block, a, b, c, d, e, 7);
			R3(block, e, a, b, c, d, 8);
			R3(block, d, e, a, b, c, 9);
			R3(block, c, d, e, a, b, 10);
			R3(block, b, c, d, e, a, 11);
			R4(block, a, b, c, d, e, 12);
			R4(block, e, a, b, c, d, 13);
			R4(block, d, e, a, b, c, 14);
			R4(block, c, d, e, a, b, 15);
			R4(block, b, c, d, e, a, 0);
			R4(block, a, b, c, d, e, 1);
			R4(block, e, a, b, c, d, 2);
			R4(block, d, e, a, b, c, 3);
			R4(block, c, d, e, a, b, 4);
			R4(block, b, c, d, e, a, 5);
			R4(block, a, b, c, d, e, 6);
			R4(block, e, a, b, c, d, 7);
			R4(block, d, e, a, b, c, 8);
			R4(block, c, d, e, a, b, 9);
			R4(block, b, c, d, e, a, 10);
			R4(block, a, b, c, d, e, 11);
			R4(block, e, a, b, c, d, 12);
			R4(block, d, e, a, b, c, 13);
			R4(block, c, d, e, a, b, 14);
			R4(block, b, c, d, e, a, 15);

			digest[0] += a;
			digest[1] += b;
			digest[2] += c;
			digest[3] += d;
			digest[4] += e;

			transforms++;
		}

		static void buffer_to_block(const std::string& buffer, uint block[BLOCK_INTS])
		{
			for (size_t i = 0; i < BLOCK_INTS; i++)
			{
				block[i] = (buffer[4 * i + 3] & 0xff)
					| (buffer[4 * i + 2] & 0xff) << 8
					| (buffer[4 * i + 1] & 0xff) << 16
					| (buffer[4 * i + 0] & 0xff) << 24;
			}
		}
	};
}

