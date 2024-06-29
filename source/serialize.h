#pragma once

#include "math.h"

#include <regex>
#include <locale>
#include <codecvt>

#include <sstream>
#include <fstream>
#include <iomanip>

namespace flame
{
	template<typename T>
	std::string str(T v)
	{
		return std::to_string(v);
	}

	template<std::integral T>
	inline std::string str_hex(T v, bool zero_fill = true)
	{
		char buf[32];
		if (zero_fill)
		{
			if constexpr (sizeof(T) == 4)
				sprintf(buf, "%08X", v);
			else if constexpr (sizeof(T) == 8)
				sprintf(buf, "%016llX", v);
		}
		else
		{
			if constexpr (sizeof(T) == 4)
				sprintf(buf, "%X", v);
			else if constexpr (sizeof(T) == 8)
				sprintf(buf, "%llX", v);
		}
		return buf;
	}

	template<std::floating_point T>
	std::string str(T v)
	{
		char buf[32];
		if constexpr (sizeof(T) == 4)
			sprintf(buf, "%.4f", v);
		else if constexpr (sizeof(T) == 8)
			sprintf(buf, "%.4lf", v);
		return buf;
	}

	template<pointer_type T>
	std::string str(T v)
	{
		return "0x" + str_hex((uint64)v);
	}

	template<typename T>
	std::string str(uint N, T* v)
	{
		std::string ret;
		ret += str(v[0]);
		for (auto i = 1; i < N; i++)
		{
			ret += ',';
			ret += str(v[i]);
		}
		return ret;
	}

	template<uint N, typename T>
	std::string str(T* v)
	{
		std::string ret;
		ret += str(v[0]);
		for (auto i = 1; i < N; i++)
		{
			ret += ',';
			ret += str(v[i]);
		}
		return ret;
	}

	template<uint N, typename T>
	std::string str(const vec<N, T>& v)
	{
		std::string ret;
		ret += str(v[0]);
		for (auto i = 1; i < N; i++)
		{
			ret += ',';
			ret += str(v[i]);
		}
		return ret;
	}

	template<uint C, uint R, typename T>
	std::string str(const mat<C, R, T>& v)
	{
		std::string ret;
		for (auto i = 0; i < C; i++)
		{
			for (auto j = 0; j < R; j++)
			{
				if (i > 0 || j > 0)
					ret += ',';
				ret += str(v[i][j]);
			}
		}
		return ret;
	}

	template<typename T>
	std::wstring wstr(T v)
	{
		return std::to_wstring(v);
	}

	template<std::integral T>
	inline std::wstring wstr_hex(T v, bool zero_fill = true)
	{
		wchar_t buf[32];
		if (zero_fill)
		{
			if constexpr (sizeof(T) == 4)
				swprintf(buf, L"%08X", v);
			else if constexpr (sizeof(T) == 8)
				swprintf(buf, L"%016llX", v);
		}
		else
		{
			if constexpr (sizeof(T) == 4)
				swprintf(buf, L"%X", v);
			else if constexpr (sizeof(T) == 8)
				swprintf(buf, L"%llX", v);
		}
		return buf;
	}

	template<std::floating_point T>
	std::wstring wstr(T v)
	{
		wchar_t buf[32];
		if constexpr (sizeof(T) == 4)
			swprintf(buf, L"%.4f", v);
		else if constexpr (sizeof(T) == 8)
			swprintf(buf, L"%.4lf", v);
		return buf;
	}

	template<pointer_type T>
	std::wstring wstr(T v)
	{
		return L"0x" + wstr_hex((uint64)v);
	}

	template<typename T>
	std::wstring wstr(uint N, T* v)
	{
		std::wstring ret;
		ret += wstr(v[0]);
		for (auto i = 1; i < N; i++)
		{
			ret += L',';
			ret += wstr(v[i]);
		}
		return ret;
	}

	template<uint N, typename T>
	std::wstring wstr(T* v)
	{
		std::wstring ret;
		ret += wstr(v[0]);
		for (auto i = 1; i < N; i++)
		{
			ret += L',';
			ret += wstr(v[i]);
		}
		return ret;
	}

	template<uint N, typename T>
	std::wstring wstr(const vec<N, T>& v)
	{
		std::wstring ret;
		ret += wstr(v[0]);
		for (auto i = 1; i < N; i++)
		{
			ret += L',';
			ret += wstr(v[i]);
		}
		return ret;
	}

	template<uint C, uint R, typename T>
	std::wstring wstr(const mat<C, R, T>& v)
	{
		std::wstring ret;
		for (auto i = 0; i < C; i++)
		{
			for (auto j = 0; j < R; j++)
			{
				if (i > 0 || j > 0)
					ret += L',';
				ret += wstr(v[i][j]);
			}
		}
		return ret;
	}

	template<std::unsigned_integral T, class CH>
	inline T s2u_hex(const std::basic_string<CH>& s)
	{
		T ret;
		std::basic_stringstream<CH> ss;
		ss << std::hex << s;
		ss >> ret;
		return ret;
	}

	template<std::unsigned_integral T, class CH>
	inline T s2u_hex(const CH* s)
	{
		return s2u_hex<T, CH>(std::basic_string<CH>(s));
	}

	template<int_type T, class CH>
	inline T s2t(const std::basic_string<CH>& s)
	{
		T ret;
		try { ret = std::stoi(s); }
		catch (...) { ret = 0; }
		return ret;
	}

	template<int_type T, class CH>
	inline T s2t(const CH* s)
	{
		return s2t<T, CH>(std::basic_string<CH>(s));
	}

	template<uint_type T, class CH>
	inline T s2t(const std::basic_string<CH>& s)
	{
		T ret;
		try { ret = std::stoul(s); }
		catch (...) { ret = 0; }
		return ret;
	}

	template<uint_type T, class CH>
	inline T s2t(const CH* s)
	{
		return s2t<T, CH>(std::basic_string<CH>(s));
	}

	template<int64_type T, class CH>
	inline T s2t(const std::basic_string<CH>& s)
	{
		T ret;
		try { ret = std::stoll(s); }
		catch (...) { ret = 0; }
		return ret;
	}

	template<int64_type T, class CH>
	inline T s2t(const CH* s)
	{
		return s2t<T, CH>(std::basic_string<CH>(s));
	}

	template<uint64_type T, class CH>
	inline T s2t(const std::basic_string<CH>& s)
	{
		T ret;
		try { ret = std::stoull(s); }
		catch (...) { ret = 0; }
		return ret;
	}

	template<uint64_type T, class CH>
	inline T s2t(const CH* s)
	{
		return s2t<T, CH>(std::basic_string<CH>(s));
	}

	template<std::floating_point T, class CH>
	inline T s2t(const std::basic_string<CH>& s)
	{
		T ret;
		try { ret = std::stof(s); }
		catch (...) { ret = 0; }
		return ret;
	}

	template<std::floating_point T, class CH>
	inline T s2t(const CH* s)
	{
		return s2t<T, CH>(std::basic_string<CH>(s));
	}

	template<uint N, typename T, class CH>
	inline vec<N, T> s2t(const std::basic_string<CH>& s)
	{
		vec<N, T> ret;
		std::basic_istringstream ss(s);
		std::string token;
		for (auto i = 0; i < N; i++)
		{
			std::getline(ss, token, ',');
			ret[i] = s2t<T, CH>(token);
		}
		return ret;
	}

	template<uint N, typename T, class CH>
	inline vec<N, T> s2t(const CH* s)
	{
		return s2t<N, T, CH>(std::basic_string<CH>(s));
	}

	template<uint C, uint R, typename T, class CH>
	inline mat<C, R, T> s2t(const std::basic_string<CH>& s)
	{
		mat<C, R, T> ret;
		std::basic_istringstream ss(s);
		std::string token;
		for (auto i = 0; i < C; i++)
		{
			for (auto j = 0; j < R; j++)
			{
				std::getline(ss, token, ',');
				ret[i][j] = s2t<T, CH>(token);
			}
		}
		return ret;
	}

	template<uint C, uint R, typename T, class CH>
	inline mat<C, R, T> s2t(const CH* s)
	{
		return s2t<C, R, T, CH>(std::basic_string<CH>(s));
	}

	template<class CH>
	struct StrUtils
	{
		static void to_lower(std::basic_string<CH>& s)
		{
			std::transform(s.begin(), s.end(), s.begin(), [](uint c) {
				return std::tolower(c);
				});
		}

		static std::basic_string<CH> get_lowered(std::basic_string_view<CH> s)
		{
			auto ret = std::basic_string<CH>(s);
			to_lower(ret);
			return ret;
		}

		static void to_upper(std::basic_string<CH>& s)
		{
			std::transform(s.begin(), s.end(), s.begin(), [](uint c) {
				return std::toupper(c);
				});
		}

		static std::basic_string<CH> get_uppered(std::basic_string_view<CH> s)
		{
			auto ret = std::basic_string<CH>(s);
			to_upper(ret);
			return ret;
		}

		static void ltrim(std::basic_string<CH>& s)
		{
			s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](char ch) {
				return !std::isspace(ch);
				}));
		}

		static void ltrim(std::basic_string<CH>& s, CH t)
		{
			s.erase(s.begin(), std::find_if(s.begin(), s.end(), [t](char ch) {
				return ch == t;
				}));
		}

		static std::basic_string<CH> get_ltrimed(std::basic_string_view<CH> s)
		{
			auto ret = std::basic_string<CH>(s);
			ltrim(ret);
			return ret;
		}

		static void rtrim(std::basic_string<CH>& s)
		{
			s.erase(std::find_if(s.rbegin(), s.rend(), [](char ch) {
				return !std::isspace(ch);
				}).base(), s.end());
		}

		static void rtrim(std::basic_string<CH>& s, CH t)
		{
			s.erase(std::find_if(s.rbegin(), s.rend(), [t](char ch) {
				return ch == t;
				}).base(), s.end());
		}

		static std::basic_string<CH> get_rtrimed(std::basic_string_view<CH> s)
		{
			auto ret = std::basic_string<CH>(s);
			rtrim(ret);
			return ret;
		}

		static void trim(std::basic_string<CH>& s)
		{
			ltrim(s);
			rtrim(s);
		}

		static std::basic_string<CH> get_trimed(std::basic_string_view<CH> s)
		{
			auto ret = std::basic_string<CH>(s);
			trim(ret);
			return ret;
		}

		static void replace_char(std::basic_string<CH>& str, CH from, CH to)
		{
			for (auto& ch : str)
			{
				if (ch == from)
					ch = to;
			}
		}

		static void strip_char(std::basic_string<CH>& str, CH ch = ' ')
		{
			str.erase(std::remove(str.begin(), str.end(), ch), str.end());
		}

		static void strip_after(std::basic_string<CH>& str, CH ch)
		{
			if (auto pos = str.find(ch); pos != std::basic_string<CH>::npos)
				str.erase(str.begin() + pos, str.end());
		}

		static std::basic_string<CH> get_tail(const std::basic_string<CH>& str, uint off, uint len)
		{
			return str.substr(str.size() - len - off, len);
		}

		static bool match_head_tail(const std::basic_string<CH>& str, std::basic_string_view<CH> head, std::basic_string_view<CH> tail)
		{
			return str.starts_with(head) && str.ends_with(tail);
		}

		static bool strip_head_if(std::basic_string<CH>& str, std::basic_string_view<CH> head)
		{
			if (str.starts_with(head))
			{
				str = str.substr(head.size());
				return true;
			}
			return false;
		}

		static bool strip_tail_if(std::basic_string<CH>& str, std::basic_string_view<CH> tail)
		{
			if (str.ends_with(tail))
			{
				str = str.substr(0, str.size() - tail.size());
				return true;
			}
			return false;
		}

		static bool strip_head_tail_if(std::basic_string<CH>& str, std::basic_string_view<CH> head, std::basic_string_view<CH> tail)
		{
			if (str.starts_with(head) && str.ends_with(tail))
			{
				str = str.substr(head.size(), str.size() - head.size() - tail.size());
				return true;
			}
			return false;
		}

		static bool match_case_insensitive(std::basic_string_view<CH> a, std::basic_string_view<CH> b)
		{
			return std::equal(a.begin(), a.end(), b.begin(), b.end(), [](CH a, CH b) {
				return std::tolower(a) == std::tolower(b);
				});
		}

		static bool find_case_insensitive(std::basic_string_view<CH> str, std::basic_string_view<CH> token)
		{
			auto it = std::search(
				str.begin(), str.end(),
				token.begin(), token.end(),
				[](CH ch1, CH ch2) { return std::toupper(ch1) == std::toupper(ch2); }
			);
			return (it != str.end());
		}

		static void replace_all(std::basic_string<CH>& str, std::basic_string_view<CH> from, std::basic_string_view<CH> to)
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

		static std::vector<std::basic_string<CH>> to_string_vector(const std::vector<std::basic_string_view<CH>>& views)
		{
			std::vector<std::basic_string<CH>> ret;
			for (auto view : views)
				ret.push_back(std::basic_string<CH>(view));
			return ret;
		}

		static std::vector<std::basic_string_view<CH>> split(std::basic_string_view<CH> str, CH delimiter = ' ')
		{
			std::vector<std::basic_string_view<CH>> ret;

			std::size_t prev = 0, pos;
			while ((pos = str.find_first_of(delimiter, prev)) != std::basic_string<CH>::npos)
			{
				if (pos > prev)
					ret.emplace_back(str.begin() + prev, str.begin() + pos);
				else if (pos == prev)
					ret.emplace_back();
				prev = pos + 1;
			}
			if (prev < str.length())
				ret.emplace_back(str.begin() + prev, str.end());

			return ret;
		}

		static std::vector<std::basic_string_view<CH>> split_dbnull(const CH* str)
		{
			std::vector<std::basic_string_view<CH>> ret;
			auto p = str;
			while (*p)
			{
				ret.push_back(std::basic_string_view<CH>(p));
				p += ret.back().size() + 1;
			}
			return ret;
		}

		static std::vector<std::basic_string_view<CH>> split_quot(std::basic_string_view<CH> str, CH quot = '\"', CH delimiter = ' ')
		{
			std::vector<std::basic_string_view<CH>> ret;
			std::vector<int> in_quot_delimiters;
			auto in_quot = false;

			for (auto i = 0; i < str.size(); i++)
			{
				auto ch = str[i];
				if (in_quot && ch == delimiter)
					in_quot_delimiters.push_back(i);
				else if (ch == quot)
					in_quot = !in_quot;

			}

			{
				std::size_t prev = 0, pos;
				while ((pos = str.find_first_of(delimiter, prev)) != std::basic_string<CH>::npos)
				{
					if (std::find(in_quot_delimiters.begin(), in_quot_delimiters.end(), pos) != in_quot_delimiters.end())
					{
						prev = pos + 1;
						continue;
					}
					if (pos > prev)
						ret.emplace_back(str.begin() + prev, str.begin() + pos);
					prev = pos + 1;
				}
				if (prev < str.length())
					ret.emplace_back(str.begin() + prev, str.end());
			}

			return ret;
		}

		static inline int first_parentheses_token_pos(std::basic_string_view<CH> str, CH left_parenthesis, CH right_parenthesis, CH delimiter)
		{
			auto p = 0, lv = 0;
			while (p < str.size())
			{
				auto ch = str[p];
				if (ch == delimiter && lv == 0)
					return p;
				else if (p == (int)str.size() - 1 && lv == 0)
					return p + 1;
				else if (ch == left_parenthesis)
					lv++;
				else if (ch == right_parenthesis)
					lv--;
				p++;
			}
			return -1;
		}

		static std::vector<std::basic_string_view<CH>> split_parentheses(std::basic_string_view<CH> str, CH left_parenthesis, CH right_parenthesis, CH delimiter = ' ')
		{
			std::vector<std::basic_string_view<CH>> ret;
			auto off = 0;
			while (off < str.size())
			{
				auto n = first_parentheses_token_pos({ str.begin() + off, str.end() }, left_parenthesis, right_parenthesis, delimiter);
				if (n == -1)
				{
					ret.push_back(str.substr(off));
					break;
				}
				ret.push_back(str.substr(off, n));
				off += n + 1;
			}
			return ret;
		}

		static uint indent_length(const std::basic_string<CH>& s)
		{
			return std::find_if(s.begin(), s.end(), [](char ch) {
				return !std::isspace(ch);
				}) - s.begin();
		}
	};

	using SUS = StrUtils<char>;
	using SUW = StrUtils<wchar_t>;

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

	inline std::filesystem::path replace_ext(const std::filesystem::path& path, const std::filesystem::path& ext)
	{
		auto ret = path;
		ret.replace_extension(ext);
		return ret;
	}

	inline std::filesystem::path replace_fn(const std::filesystem::path& path, const std::wstring& fmt)
	{
		auto ret = path;
		auto fn = ret.filename().stem().wstring();
		fn = std::vformat(fmt, std::make_wformat_args(fn));
		fn += ret.extension().wstring();
		ret.replace_filename(fn);
		return ret;
	}

	enum FileType
	{
		FileTypeUnknown,
		FileTypeFolder,
		FileTypeText,
		FileTypeImage,
		FileTypeModel
	};

	inline bool is_text_file(const std::filesystem::path& ext)
	{
		if (ext == L".txt" ||
			ext == L".h" || ext == L".c" || ext == L".cpp" || ext == L".hpp" || ext == L".cxx" || ext == L".inl" ||
			ext == L".glsl" || ext == L".vert" || ext == L".tesc" || ext == L".tese" || ext == L".geom" || ext == L".frag" || ext == L".hlsl" ||
			ext == L".rp" || ext == L".dsl" || ext == L".pll" || ext == L".pipeline" || ext == L".res" ||
			ext == L".xml" || ext == L".json" || ext == L".ini" || ext == L".log" ||
			ext == L".prefab" || ext == L".fmat" || ext == L".fmod" || ext == L".fani" ||
			ext == L".htm" || ext == L".html" || ext == L".css")
			return true;
		return false;
	}

	inline bool is_image_file(const std::filesystem::path& ext)
	{
		if (ext == L".bmp" || ext == L".jpg" || ext == L".jpeg" || ext == L".png" || ext == L".gif" ||
			ext == L".tga" || ext == L".dds" || ext == L".ktx")
			return true;
		return false;
	}

	inline bool is_model_file(const std::filesystem::path& ext)
	{
		if (ext == L".obj" || ext == L".dae" || ext == L".gltf" || ext == L".glb" || ext == L".fbx")
			return true;
		return false;
	}

	inline bool is_audio_file(const std::filesystem::path& ext)
	{
		if (ext == L".wav" || ext == L".ogg")
			return true;
		return false;
	}

	inline FileType get_file_type(const std::filesystem::path& ext)
	{
		if (is_text_file(ext))
			return FileTypeText;
		if (is_image_file(ext))
			return FileTypeImage;
		if (is_model_file(ext))
			return FileTypeModel;
		return FileTypeUnknown;
	}

	inline uint64 get_file_length(const std::filesystem::path& filename)
	{
		std::ifstream file(filename, std::ios::binary);
		if (!file.good())
			return 0;
		file.seekg(0, std::ios::end);
		return file.tellg();
	}

	inline std::string get_file_content(const std::filesystem::path& filename)
	{
		std::string ret;
		std::ifstream file(filename, std::ios::binary);
		if (file.good())
		{
			file.seekg(0, std::ios::end);
			auto length = file.tellg();
			file.seekg(0, std::ios::beg);
			ret.resize(length);
			file.read(ret.data(), length);
			ret[length] = 0;
			file.close();
		}
		return ret;
	}

	inline std::vector<std::string> get_file_lines(const std::filesystem::path& filename)
	{
		std::vector<std::string> ret;
		std::ifstream file(filename);
		if (file.good())
		{
			std::string line;
			while (!file.eof())
			{
				std::getline(file, line);
				ret.push_back(line);
			}
			file.close();
		}
		return ret;
	}

	inline std::vector<std::filesystem::path> glob_files(const std::filesystem::path& dir, const std::filesystem::path& ext)
	{
		std::vector<std::filesystem::path> ret;
		for (auto& it : std::filesystem::directory_iterator(dir))
		{
			if (it.path().extension() == ext)
				ret.push_back(it.path());
		}
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

	template<typename T>
	void read_t(std::ifstream& f, T& v)
	{
		f.read((char*)&v, sizeof(T));
	}

	template<typename T>
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

	inline void write_s(std::ofstream& f, std::string_view v)
	{
		write_u(f, v.size());
		f.write(v.data(), v.size());
	}

	template<typename T>
	void write_t(std::ofstream& f, const T& v)
	{
		f.write((char*)&v, sizeof(T));
	}

	template<typename T>
	void write_v(std::ofstream& f, const std::vector<T>& v)
	{
		write_u(f, v.size());
		f.write((char*)v.data(), v.size() * sizeof(T));
	}

	struct LineReader
	{
		std::istream& stream;
		std::vector<std::string> lines;
		int anchor = -1;

		LineReader(std::istream& stream) :
			stream(stream)
		{
		}

		inline std::string& line(int off = 0)
		{
			return lines[anchor + off];
		}

		inline bool next_line()
		{
			if (anchor + 1 >= (int)lines.size())
				return false;
			anchor++;
			return !line().empty();
		}

		inline bool read_block(const std::string& beg_mark, const std::string& end_mark = "[EMPTY_LINE]")
		{
			lines.clear();
			anchor = -1;

			std::string line;
			if (!beg_mark.empty())
			{
				while (true)
				{
					if (stream.eof())
						return false;
					std::getline(stream, line);
					if (SUS::get_ltrimed(line).starts_with(beg_mark))
						break;
				}
			}
			if (!end_mark.empty())
			{
				auto exit_when_empty = end_mark == "[EMPTY_LINE]";
				while (true)
				{
					if (stream.eof())
						return false;
					std::getline(stream, line);
					auto str = SUS::get_ltrimed(line);
					if (str.empty() && exit_when_empty)
						return true;
					if (str.starts_with(end_mark))
						return true;
					lines.push_back(line);
				}
			}
		}

		inline std::string to_string()
		{
			std::string ret;
			for (auto& l : lines)
				ret += l + "\n";
			return ret;
		}
	};

	struct INI_Entry
	{
		uint key_hash;
		std::string key;
		std::vector<std::string> values;
	};

	struct INI_Section
	{
		std::string name;
		std::vector<INI_Entry> entries;
	};

	struct INI_File
	{
		std::vector<INI_Section> sections;

		const std::vector<INI_Entry>& get_section_entries(std::string_view name)
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

					static std::regex reg(R"(^([\*\w]+)\s*=(.*))");
					std::smatch res;
					INI_Entry entry;
					if (std::regex_search(line, res, reg))
					{
						entry.key = res[1].str();
						entry.key_hash = sh(entry.key.c_str());
						line = res[2].str();
					}
					while (line.ends_with('\\'))
					{
						line.pop_back();
						std::string line2;
						std::getline(file, line2);
						line += line2;
					}
					SUS::trim(line);
					entry.values = SUS::to_string_vector(SUS::split_quot(line));
					for (auto& t : entry.values)
					{
						if (t.size() > 2 && t.front() == '\"' && t.back() == '\"')
							t = std::string(t.begin() + 1, t.end() - 1);
					}

					ret.sections.back().entries.push_back(entry);
				}
			}
		}
		file.close();

		return ret;
	}

	template<class CH>
	std::basic_string<CH> get_display_name(const std::basic_string<CH>& name)
	{
		std::basic_string<CH> ret;
		ret += toupper(name[0]);
		for (auto i = 1; i < name.size(); i++)
		{
			if (name[i] == '_')
			{
				ret += ' ';
				ret += toupper(name[i + 1]);
				i++;
			}
			else if (isupper(name[i]) && isalpha(name[i - 1]))
			{
				ret += ' ';
				ret += name[i];
			}
			else
				ret += name[i];
		}
		return ret;
	}
}

