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
	template <std::floating_point T>
	std::string to_string(T v)
	{
		std::ostringstream ss;
		ss.precision(6);
		ss << v;
		return ss.str();
	}

	template <class T>
	std::string to_string(T v)
	{
		return std::to_string(v);
	}

	template <uint N, class T>
	std::string to_string(const vec<N, T>& v)
	{
		std::string ret;
		ret += to_string(v[0]);
		for (auto i = 1; i < N; i++)
		{
			ret += ",";
			ret += to_string(v[i]);
		}
		return ret;
	}

	template <uint C, uint R, class T>
	std::string to_string(const mat<C, R, T>& v)
	{
		std::string ret;
		for (auto i = 0; i < C; i++)
		{
			for (auto j = 0; j < R; j++)
			{
				if (i > 0 || j > 0)
					ret += ",";
				ret += to_string(v[i][j]);
			}
		}
		return ret;
	}

	template <std::integral T>
	inline std::string to_hex_string(T v, bool zero_fill = true)
	{
		std::ostringstream ss;
		if (zero_fill)
			ss << std::setfill('0') << std::setw(sizeof(T) * 2); 
		ss << std::hex << v;
		return ss.str();
	}


	template <std::floating_point T>
	std::wstring to_wstring(T v)
	{
		std::wostringstream ss;
		ss.precision(6);
		ss << v;
		return ss.str();
	}

	template <class T>
	std::wstring to_wstring(T v)
	{
		return std::to_wstring(v);
	}

	template <uint N, class T>
	std::wstring to_wstring(const vec<N, T>& v)
	{
		std::wstring ret;
		ret += to_wstring(v[0]);
		for (auto i = 1; i < N; i++)
		{
			ret += L",";
			ret += to_wstring(v[i]);
		}
		return ret;
	}

	template <std::integral T>
	inline std::wstring to_hex_wstring(T v, bool zero_fill = true)
	{
		std::wostringstream ss;
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

	template <int_type T, class CH>
	inline T sto(const std::basic_string<CH>& s)
	{
		T ret;
		try { ret = std::stoi(s); }
		catch (...) { ret = 0; }
		return ret;
	}

	template <int_type T, class CH>
	inline T sto(const CH* s)
	{
		return sto<T, CH>(std::basic_string<CH>(s));
	}

	template <uint_type T, class CH>
	inline T sto(const std::basic_string<CH>& s)
	{
		T ret;
		try { ret = std::stoul(s); }
		catch (...) { ret = 0; }
		return ret;
	}

	template <uint_type T, class CH>
	inline T sto(const CH* s)
	{
		return sto<T, CH>(std::basic_string<CH>(s));
	}

	template <int64_type T, class CH>
	inline T sto(const std::basic_string<CH>& s)
	{
		T ret;
		try { ret = std::stoll(s); }
		catch (...) { ret = 0; }
		return ret;
	}

	template <int64_type T, class CH>
	inline T sto(const CH* s)
	{
		return sto<T, CH>(std::basic_string<CH>(s));
	}

	template <uint64_type T, class CH>
	inline T sto(const std::basic_string<CH>& s)
	{
		T ret;
		try { ret = std::stoull(s); }
		catch (...) { ret = 0; }
		return ret;
	}

	template <uint64_type T, class CH>
	inline T sto(const CH* s)
	{
		return sto<T, CH>(std::basic_string<CH>(s));
	}

	template <std::floating_point T, class CH>
	inline T sto(const std::basic_string<CH>& s)
	{
		T ret;
		try { ret = std::stof(s); }
		catch (...) { ret = 0; }
		return ret;
	}

	template <std::floating_point T, class CH>
	inline T sto(const CH* s)
	{
		return sto<T, CH>(std::basic_string<CH>(s));
	}

	template <uint N, class T, class CH>
	inline vec<N, T> sto(const std::basic_string<CH>& s)
	{
		vec<N, T> ret;
		std::basic_istringstream ss(s);
		std::string token;
		for (auto i = 0; i < N; i++)
		{
			std::getline(ss, token, ',');
			ret[i] = sto<T, CH>(token);
		}
		return ret;
	}

	template <uint C, uint R, class T, class CH>
	inline mat<C, R, T> sto(const std::basic_string<CH>& s)
	{
		mat<C, R, T> ret;
		std::basic_istringstream ss(s);
		std::string token;
		for (auto i = 0; i < C; i++)
		{
			for (auto j = 0; j < R; j++)
			{
				std::getline(ss, token, ',');
				ret[i][j] = sto<T, CH>(token);
			}
		}
		return ret;
	}

	template <uint C, uint R, class T, class CH>
	inline mat<C, R, T> sto(const CH* s)
	{
		return sto<C, R, T, CH>(std::basic_string<CH>(s));
	}

	template <class CH>
	struct StrUtils
	{
		static uint indent_length(const std::basic_string<CH>& s)
		{
			return std::find_if(s.begin(), s.end(), [](char sh) {
				return !std::isspace(sh);
			}) - s.begin();
		}

		static void ltrim(std::basic_string<CH>& s)
		{
			s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](char sh) {
				return !std::isspace(sh);
			}));
		}

		static std::string get_ltrimed(const std::basic_string<CH>& s)
		{
			auto ret = s;
			ltrim(ret);
			return ret;
		}

		static void rtrim(std::basic_string<CH>& s)
		{
			s.erase(std::find_if(s.rbegin(), s.rend(), [](char sh) {
				return !std::isspace(sh);
			}).base(), s.end());
		}

		static std::string get_rtrimed(const std::basic_string<CH>& s)
		{
			auto ret = s;
			rtrim(ret);
			return ret;
		}

		static void trim(std::basic_string<CH>& s)
		{
			ltrim(s);
			rtrim(s);
		}

		static std::string get_trimed(const std::basic_string<CH>& s)
		{
			auto ret = s;
			trim(ret);
			return ret;
		}
		
		static void replace_char(std::basic_string<CH>& str, CH from, CH to)
		{
			for (auto& sh : str)
			{
				if (sh == from)
					sh = to;
			}
		}

		static void remove_char(std::basic_string<CH>& str, CH sh = ' ')
		{
			str.erase(std::remove(str.begin(), str.end(), sh), str.end());
		}

		static void remove_spaces(std::basic_string<CH>& str)
		{
			str.erase(std::remove_if(str.begin(), str.end(), [](char sh) {
				return std::isspace(sh);
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

		static std::basic_string<CH> get_tail(const std::basic_string<CH>& str, uint off, uint len)
		{
			return str.substr(str.size() - len - off, len);
		}

		static bool remove_head(std::basic_string<CH>& str, const std::basic_string<CH>& head)
		{
			if (str.starts_with(head))
			{
				str = str.substr(head.size());
				return true;
			}
			return false;
		}

		static bool remove_tail(std::basic_string<CH>& str, const std::basic_string<CH>& tail)
		{
			if (str.ends_with(tail))
			{
				str = str.substr(0, str.size() - tail.size());
				return true;
			}
			return false;
		}

		static bool remove_both_ends(std::basic_string<CH>& str, const std::basic_string<CH>& head, const std::basic_string<CH>& tail)
		{
			if (str.starts_with(head) && str.ends_with(tail))
			{
				str = str.substr(head.size(), str.size() - head.size() - tail.size());
				return true;
			}
			return false;
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

	inline bool is_text_file(const std::filesystem::path& ext)
	{
		if (ext == L".txt" ||
			ext == L".h" || ext == L".c" || ext == L".cpp" || ext == L".hpp" || ext == L".cxx" || ext == L".inl" ||
			ext == L".glsl" || ext == L".vert" || ext == L".tesc" || ext == L".tese" || ext == L".geom" || ext == L".frag" || ext == L".hlsl" ||
			ext == L".xml" || ext == L".json" || ext == L".ini" || ext == L".log" ||
			ext == L".htm" || ext == L".html" || ext == L".css" ||
			ext == L".sln" || ext == L".vcxproj")
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
		if (ext == L".obj" || ext == L".dae")
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
		char sh;
		f.read(&sh, sizeof(char));
		return sh == 1;
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
		char sh = b ? 1 : 0;
		f.write(&sh, sizeof(char));
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

		inline std::string form_content()
		{
			std::string ret;
			for (auto& l : lines)
				ret += l + "\n";
			return ret;
		}
	};

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

