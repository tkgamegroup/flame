#pragma once

#include "math.h"

#include <regex>
#include <locale>
#include <codecvt>

#include <filesystem>
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
				if (ch == L'.')
				{
					buf[ret + 1] = L'0';
					ret += 2;
					break;
				}
				if (ch == L'0')
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

	template <class CH, uint N, class T>
	int fmt(CH* buf, int buf_size, const vec<N, T>& v)
	{
		auto p = buf;
		auto s = buf_size;
		for (auto i = 0; i < N; i++)
		{
			auto ret = fmt(p, s, v[i]);
			p += ret;
			s -= ret;
			if (i < N - 1)
			{
				*p = ',';
				p++;
				s--;
			}
		}
		return buf_size - s;
	}

	template <class CH>
	int fmt(CH* buf, int buf_size, const quat& q)
	{
		return fmt(buf, buf_size, vec4(q.w, q.x, q.y, q.z));
	}

	template <class CH>
	int fmt(CH* buf, int buf_size, const Rect& r)
	{
		return fmt(buf, buf_size, vec4(r.a.x, r.a.y, r.b.x, r.b.y));
	}

	template <class T>
	std::string to_string(T v)
	{
		char buf[32];
		fmt(buf, _countof(buf), v);
		return buf;
	}

	template <uint N, class T>
	std::string to_string(const vec<N, T>& v)
	{
		char buf[32 * N];
		fmt(buf, _countof(buf), v);
		return buf;
	}

	inline std::string to_string(const quat& v)
	{
		char buf[32 * 4];
		fmt(buf, _countof(buf), v);
		return buf;
	}

	template <class T, class = typename std::enable_if<std::is_integral<T>::value>::type>
	inline std::string to_hex_string(T v, bool zero_fill = true)
	{
		std::stringstream ss;
		if (zero_fill)
			ss << std::setfill('0') << std::setw(sizeof(T) * 2); 
		ss << std::hex << v;
		return ss.str();
	}

	template <class T>
	std::wstring to_wstring(T v)
	{
		wchar_t buf[32];
		fmt(buf, _countof(buf), v);
		return buf;
	}

	template <uint N, class T>
	std::wstring to_wstring(const vec<N, T>& v)
	{
		wchar_t buf[32 * N];
		fmt(buf, _countof(buf), v);
		return buf;
	}

	inline std::wstring to_wstring(const quat& v)
	{
		wchar_t buf[32 * 4];
		fmt(buf, _countof(buf), v);
		return buf;
	}

	template <class T, class = typename std::enable_if<std::is_integral<T>::value>::type>
	inline std::wstring to_hex_wstring(T v, bool zero_fill = true)
	{
		std::wstringstream ss;
		if (zero_fill)
			ss << std::setfill(L'0') << std::setw(sizeof(T) * 2);
		ss << std::hex << v;
		return ss.str();
	}

	template <class CH>
	inline uint64 from_hex_string(const std::basic_string<CH>& str)
	{
		uint64 ret;
		std::basic_stringstream<CH> ss;
		ss << std::hex << str;
		ss >> ret;
		return ret;
	}

	template <class T>
	T sto(const std::string& s);
	template <class T>
	T sto(const char* s);

	template <class T>
	T sto(const std::wstring& s);
	template <class T>
	T sto(const wchar_t* s);

	template <>
	inline char sto<char>(const std::string& s)
	{
		char ret;
		try { ret = std::stoi(s); }
		catch (...) { ret = 0; }
		return ret;
	}

	template <>
	inline char sto<char>(const char* s)
	{
		return sto<char>(std::string(s));
	}

	template <>
	inline uchar sto<uchar>(const std::string& s)
	{
		uchar ret;
		try { ret = std::stoi(s); }
		catch (...) { ret = 0; }
		return ret;
	}

	template <>
	inline uchar sto<uchar>(const char* s)
	{
		return sto<uchar>(std::string(s));
	}

	template <>
	inline wchar_t sto<wchar_t>(const std::string& s)
	{
		wchar_t ret;
		try { ret = std::stoi(s); }
		catch (...) { ret = 0; }
		return ret;
	}

	template <>
	inline wchar_t sto<wchar_t>(const char* s)
	{
		return sto<wchar_t>(std::string(s));
	}

	template <>
	inline int sto<int>(const std::string& s)
	{
		int ret;
		try { ret = std::stoi(s); }
		catch (...) { ret = 0; }
		return ret;
	}

	template <>
	inline int sto<int>(const char* s)
	{
		return sto<int>(std::string(s));
	}

	template <>
	inline uint sto<uint>(const std::string& s)
	{
		uint ret;
		try { ret = std::stoi(s); }
		catch (...) { ret = 0; }
		return ret;
	}

	template <>
	inline uint sto<uint>(const char* s)
	{
		return sto<uint>(std::string(s));
	}

	template <>
	inline int64 sto<int64>(const std::string& s)
	{
		int64 ret;
		try { ret = std::stoull(s); }
		catch (...) { ret = 0; }
		return ret;
	}

	template <>
	inline int64 sto<int64>(const char* s)
	{
		return sto<int64>(std::string(s));
	}

	template <>
	inline uint64 sto<uint64>(const std::string& s)
	{
		uint64 ret;
		try { ret = std::stoull(s); }
		catch (...) { ret = 0; }
		return ret;
	}

	template <>
	inline uint64 sto<uint64>(const char* s)
	{
		return sto<uint64>(std::string(s));
	}

	template <>
	inline float sto<float>(const std::string& s)
	{
		float ret;
		try { ret = std::stof(s); }
		catch (...) { ret = 0; }
		return ret;
	}

	template <>
	inline float sto<float>(const char* s)
	{
		return sto<float>(std::string(s));
	}

	template <>
	inline char sto<char>(const std::wstring& s)
	{
		char ret;
		try { ret = std::stoi(s); }
		catch (...) { ret = 0; }
		return ret;
	}

	template <>
	inline char sto<char>(const wchar_t* s)
	{
		return sto<char>(std::wstring(s));
	}

	template <>
	inline uchar sto<uchar>(const std::wstring& s)
	{
		uchar ret;
		try { ret = std::stoi(s); }
		catch (...) { ret = 0; }
		return ret;
	}

	template <>
	inline uchar sto<uchar>(const wchar_t* s)
	{
		return sto<uchar>(std::wstring(s));
	}

	template <>
	inline wchar_t sto<wchar_t>(const std::wstring& s)
	{
		wchar_t ret;
		try { ret = std::stoi(s); }
		catch (...) { ret = 0; }
		return ret;
	}

	template <>
	inline wchar_t sto<wchar_t>(const wchar_t* s)
	{
		return sto<wchar_t>(std::wstring(s));
	}

	template <>
	inline int sto<int>(const std::wstring& s)
	{
		int ret;
		try { ret = std::stoi(s); }
		catch (...) { ret = 0; }
		return ret;
	}

	template <>
	inline int sto<int>(const wchar_t* s)
	{
		return sto<int>(std::wstring(s));
	}

	template <>
	inline uint sto<uint>(const std::wstring& s)
	{
		uint ret;
		try { ret = std::stoi(s); }
		catch (...) { ret = 0; }
		return ret;
	}

	template <>
	inline uint sto<uint>(const wchar_t* s)
	{
		return sto<uint>(std::wstring(s));
	}

	template <>
	inline int64 sto<int64>(const std::wstring& s)
	{
		int64 ret;
		try { ret = std::stoull(s); }
		catch (...) { ret = 0; }
		return ret;
	}

	template <>
	inline int64 sto<int64>(const wchar_t* s)
	{
		return sto<int64>(std::wstring(s));
	}

	template <>
	inline uint64 sto<uint64>(const std::wstring& s)
	{
		uint64 ret;
		try { ret = std::stoull(s); }
		catch (...) { ret = 0; }
		return ret;
	}

	template <>
	inline uint64 sto<uint64>(const wchar_t* s)
	{
		return sto<uint64>(std::wstring(s));
	}

	template <>
	inline float sto<float>(const std::wstring& s)
	{
		float ret;
		try { ret = std::stof(s); }
		catch (...) { ret = 0; }
		return ret;
	}

	template <>
	inline float sto<float>(const wchar_t* s)
	{
		return sto<float>(std::wstring(s));
	}

	template <>
	inline cvec2 sto<cvec2>(const char* s)
	{
		ivec2 ret;
		sscanf(s, "%d,%d", &ret.x, &ret.y);
		return cvec2(ret);
	}

	template <>
	inline cvec3 sto<cvec3>(const char* s)
	{
		ivec3 ret;
		sscanf(s, "%d,%d,%d", &ret.x, &ret.y, &ret.z);
		return cvec3(ret);
	}

	template <>
	inline cvec4 sto<cvec4>(const char* s)
	{
		ivec4 ret;
		sscanf(s, "%d,%d,%d,%d", &ret.x, &ret.y, &ret.z, &ret.w);
		return cvec4(ret);
	}

	template <>
	inline ivec2 sto<ivec2>(const char* s)
	{
		ivec2 ret;
		sscanf(s, "%d,%d", &ret.x, &ret.y);
		return ret;
	}

	template <>
	inline ivec3 sto<ivec3>(const char* s)
	{
		ivec3 ret;
		sscanf(s, "%d,%d,%d", &ret.x, &ret.y, &ret.z);
		return ret;
	}

	template <>
	inline ivec4 sto<ivec4>(const char* s)
	{
		ivec4 ret;
		sscanf(s, "%d,%d,%d,%d", &ret.x, &ret.y, &ret.z, &ret.w);
		return ret;
	}

	template <>
	inline uvec2 sto<uvec2>(const char* s)
	{
		uvec2 ret;
		sscanf(s, "%u,%u", &ret.x, &ret.y);
		return ret;
	}

	template <>
	inline uvec3 sto<uvec3>(const char* s)
	{
		uvec3 ret;
		sscanf(s, "%u,%u,%u", &ret.x, &ret.y, &ret.z);
		return ret;
	}

	template <>
	inline uvec4 sto<uvec4>(const char* s)
	{
		uvec4 ret;
		sscanf(s, "%u,%u,%u,%u", &ret.x, &ret.y, &ret.z, &ret.w);
		return ret;
	}

	template <>
	inline vec2 sto<vec2>(const char* s)
	{
		vec2 ret;
		sscanf(s, "%f,%f", &ret.x, &ret.y);
		return ret;
	}

	template <>
	inline vec3 sto<vec3>(const char* s)
	{
		vec3 ret;
		sscanf(s, "%f,%f,%f", &ret.x, &ret.y, &ret.z);
		return ret;
	}

	template <>
	inline vec4 sto<vec4>(const char* s)
	{
		vec4 ret;
		sscanf(s, "%f,%f,%f,%f", &ret.x, &ret.y, &ret.z, &ret.w);
		return ret;
	}

	template <>
	inline quat sto<quat>(const char* s)
	{
		quat ret;
		sscanf(s, "%f,%f,%f,%f", &ret.w, &ret.x, &ret.y, &ret.z);
		return ret;
	}

	template <>
	inline cvec2 sto<cvec2>(const wchar_t* s)
	{
		ivec2 ret;
		swscanf(s, L"%d,%d", &ret.x, &ret.y);
		return cvec2(ret);
	}

	template <>
	inline cvec3 sto<cvec3>(const wchar_t* s)
	{
		ivec3 ret;
		swscanf(s, L"%d,%d,%d", &ret.x, &ret.y, &ret.z);
		return cvec3(ret);
	}

	template <>
	inline cvec4 sto<cvec4>(const wchar_t* s)
	{
		ivec4 ret;
		swscanf(s, L"%d,%d,%d,%d", &ret.x, &ret.y, &ret.z, &ret.w);
		return cvec4(ret);
	}

	template <>
	inline ivec2 sto<ivec2>(const wchar_t* s)
	{
		ivec2 ret;
		swscanf(s, L"%d,%d", &ret.x, &ret.y);
		return ret;
	}

	template <>
	inline ivec3 sto<ivec3>(const wchar_t* s)
	{
		ivec3 ret;
		swscanf(s, L"%d,%d,%d", &ret.x, &ret.y, &ret.z);
		return ret;
	}

	template <>
	inline ivec4 sto<ivec4>(const wchar_t* s)
	{
		ivec4 ret;
		swscanf(s, L"%d,%d,%d,%d", &ret.x, &ret.y, &ret.z, &ret.w);
		return ret;
	}

	template <>
	inline uvec2 sto<uvec2>(const wchar_t* s)
	{
		uvec2 ret;
		swscanf(s, L"%u,%u", &ret.x, &ret.y);
		return ret;
	}

	template <>
	inline uvec3 sto<uvec3>(const wchar_t* s)
	{
		uvec3 ret;
		swscanf(s, L"%u,%u,%u", &ret.x, &ret.y, &ret.z);
		return ret;
	}

	template <>
	inline uvec4 sto<uvec4>(const wchar_t* s)
	{
		uvec4 ret;
		swscanf(s, L"%u,%u,%u,%u", &ret.x, &ret.y, &ret.z, &ret.w);
		return ret;
	}

	template <>
	inline vec2 sto<vec2>(const wchar_t* s)
	{
		vec2 ret;
		swscanf(s, L"%f,%f", &ret.x, &ret.y);
		return ret;
	}

	template <>
	inline vec3 sto<vec3>(const wchar_t* s)
	{
		vec3 ret;
		swscanf(s, L"%f,%f,%f", &ret.x, &ret.y, &ret.z);
		return ret;
	}

	template <>
	inline vec4 sto<vec4>(const wchar_t* s)
	{
		vec4 ret;
		swscanf(s, L"%f,%f,%f,%f", &ret.x, &ret.y, &ret.z, &ret.w);
		return ret;
	}

	template <>
	inline quat sto<quat>(const wchar_t* s)
	{
		quat ret;
		swscanf(s, L"%f,%f,%f,%f", &ret.w, &ret.x, &ret.y, &ret.z);
		return ret;
	}

	template <class CH>
	struct StrUtils
	{
		static void ltrim(std::basic_string<CH>& s)
		{
			s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](char ch) {
				return !std::isspace(ch);
			}));
		}

		static void rtrim(std::basic_string<CH>& s)
		{
			s.erase(std::find_if(s.rbegin(), s.rend(), [](char ch) {
				return !std::isspace(ch);
			}).base(), s.end());
		}

		static void trim(std::basic_string<CH>& s)
		{
			ltrim(s);
			rtrim(s);
		}

		static void remove_ch(std::basic_string<CH>& str, CH ch = ' ')
		{
			str.erase(std::remove(str.begin(), str.end(), ch), str.end());
		}

		static void remove_spaces(std::basic_string<CH>& str)
		{
			str.erase(std::remove_if(str.begin(), str.end(), [](char ch) {
				return std::isspace(ch);
			}), str.end());
		}

		static std::vector<std::basic_string<CH>> split_with_spaces(const std::basic_string<CH>& str)
		{
			std::basic_istringstream<CH> iss(str);
			std::vector<std::basic_string<CH>> ret;

			std::basic_string<CH> s;
			while (iss >> s)
				ret.push_back(s);

			return ret;
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

		static void cut_head_if(std::basic_string<CH>& str, const std::basic_string<CH>& head)
		{
			if (str.starts_with(head))
				str = str.substr(head.size());
		}

		static void cut_tail_if(std::basic_string<CH>& str, const std::basic_string<CH>& tail)
		{
			if (str.ends_with(tail))
				str = str.substr(0, str.size() - tail.size());
		}

		static void replace_all(std::basic_string<CH>& str, const std::basic_string<CH>& from, const std::basic_string<CH>& to)
		{
			if (from.empty())
				return;
			auto start_pos = 0;
			while ((start_pos = str.find(from, start_pos)) != std::basic_string<CH>::npos)
			{
				str.replace(start_pos, from.length(), to);
				start_pos += to.length();
			}
		}
	};

	using SUS = StrUtils<char>;
	using SUW = StrUtils<wchar_t>;

	inline std::string sfmt(const char* fmt, ...)
	{
		char buf[1024];
		va_list ap;
		va_start(ap, &fmt);
		vsprintf(buf, fmt, ap);
		va_end(ap);
		return buf;
	}

	inline std::wstring wfmt(const wchar_t* fmt, ...)
	{
		wchar_t buf[1024];
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
		if (ext == L".obj" || ext == L".dae")
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

	inline int64 get_file_length(std::ifstream& f)
	{
		f.seekg(0, std::ios::end);
		auto s = f.tellg();
		f.seekg(0, std::ios::beg);
		return s;
	}

	inline std::string get_file_content(const std::filesystem::path& filename)
	{
		std::ifstream file(filename, std::ios::binary);
		if (!file.good())
			return "";

		auto length = get_file_length(file);
		std::string ret;
		ret.resize(length);
		file.read(ret.data(), length);
		file.close();
		return ret;
	}

	inline bool read_b(std::ifstream& f)
	{
		char ch;
		f.read(&ch, sizeof(char));
		return ch == 1;
	}

	inline int read_i(std::ifstream& f)
	{
		int v;
		f.read((char*)&v, sizeof(int));
		return v;
	}

	inline uint read_u(std::ifstream& f)
	{
		uint v;
		f.read((char*)&v, sizeof(uint));
		return v;
	}

	inline void read_s(std::ifstream& f, std::string& v)
	{
		v.resize(read_u(f));
		f.read(v.data(), v.size());
	}

	inline void read_fn(std::ifstream& f, std::filesystem::path& fn)
	{
		std::string str;
		read_s(f, str);
		fn = str;
	}

	template <class T>
	void read_t(std::ifstream& f, T& v)
	{
		f.read((char*)&v, sizeof(T));
	}

	template <class T>
	void read_v(std::ifstream& f, std::vector<T>& v)
	{
		v.resize(read_u(f));
		f.read((char*)v.data(), v.size() * sizeof(T));
	}

	inline void write_b(std::ofstream& f, bool b)
	{
		char ch = b ? 1 : 0;
		f.write(&ch, sizeof(char));
	}

	inline void write_i(std::ofstream& f, int v)
	{
		f.write((char*)&v, sizeof(int));
	}

	inline void write_u(std::ofstream& f, uint v)
	{
		f.write((char*)&v, sizeof(uint));
	}

	inline void write_s(std::ofstream& f, const std::string& v)
	{
		write_u(f, v.size());
		f.write(v.c_str(), v.size());
	}

	template <class T>
	void write_t(std::ofstream& f, const T& v)
	{
		f.write((char*)&v, sizeof(T));
	}

	template <class T>
	void write_v(std::ofstream& f, const std::vector<T>& v)
	{
		write_u(f, v.size());
		f.write((char*)v.data(), v.size() * sizeof(T));
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
				SUS::trim(line);
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
					SUS::trim(entry.value);
					if (std::regex_search(entry.value, res, reg_quot))
						entry.value = res[1].str();

					ret.sections.back().entries.push_back(entry);
				}
			}
		}
		file.close();

		return ret;
	}
}

