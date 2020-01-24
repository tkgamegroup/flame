#pragma once

#include <flame/math.h>

#include <pugixml.hpp>

#include <stdarg.h>
#include <regex>
#include <locale>
#include <codecvt>
#include <filesystem>
#include <sstream>
#include <iomanip>
#include <fstream>

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
	bool sstartswith(const std::basic_string<CH>& str, const std::basic_string<CH>& oth)
	{
		return str.size() >= oth.size() && str.compare(0, oth.size(), oth) == 0;
	}

	template<typename CH>
	bool sendswith(const std::basic_string<CH>& str, const std::basic_string<CH>& oth)
	{
		return str.size() >= oth.size() && str.compare(str.size() - oth.size(), oth.size(), oth) == 0;
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

	template<class T>
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

	template<class T>
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

	inline longlong get_file_length(std::ifstream& f)
	{
		f.seekg(0, std::ios::end);
		auto s = f.tellg();
		f.seekg(0, std::ios::beg);
		return s;
	}

	inline std::pair<std::unique_ptr<char[]>, longlong> get_file_content(const std::wstring& filename)
	{
#ifdef FLAME_WINDOWS
		std::ifstream file(filename, std::ios::binary);
#else
		auto utf8_filename = w2s(filename);
		std::ifstream file(utf8_filename, std::ios::binary);
#endif
		if (!file.good())
			return std::make_pair(nullptr, 0);

		auto length = get_file_length(file);
		auto data = new char[length + 1];
		file.read(data, length);
		data[length] = 0;
		return std::make_pair(std::unique_ptr<char[]>(data), length);
	}

	inline std::string get_file_string(const std::wstring& filename)
	{
		auto content = get_file_content(filename);
		if (content.first)
			return std::string(content.first.get(), content.first.get() + content.second);
		return std::string();
	}

	// SHA1 is from https://github.com/vog/sha1

	struct SHA1
	{
		uint digest[5];
		std::string buffer;
		ulonglong transforms;

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
			ulonglong total_bits = (transforms * BLOCK_BYTES + buffer.size()) * 8;

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
			result.resize(sizeof(uint) * array_size(digest));
			auto dst = &result[0];
			for (auto i = 0; i < array_size(digest); i++)
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
			for (auto i = 0; i < array_size(digest); i++)
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

		static void transform(uint digest[], uint block[BLOCK_INTS], ulonglong& transforms)
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

	inline uint data_size(uint type_hash)
	{
		switch (type_hash)
		{
		case FLAME_CHASH("bool"):
			return sizeof(bool);
		case FLAME_CHASH("int"):
		case FLAME_CHASH("uint"):
			return sizeof(int);
		case FLAME_CHASH("Vec(1+int)"):
		case FLAME_CHASH("Vec(1+uint)"):
			return sizeof(Vec1i);
		case FLAME_CHASH("Vec(2+int)"):
		case FLAME_CHASH("Vec(2+uint)"):
			return sizeof(Vec2i);
		case FLAME_CHASH("Vec(3+int)"):
		case FLAME_CHASH("Vec(3+uint)"):
			return sizeof(Vec3i);
		case FLAME_CHASH("Vec(4+int)"):
		case FLAME_CHASH("Vec(4+uint)"):
			return sizeof(Vec4i);
		case FLAME_CHASH("longlong"):
		case FLAME_CHASH("ulonglong"):
			return sizeof(longlong);
		case FLAME_CHASH("float"):
			return sizeof(float);
		case FLAME_CHASH("Vec(1+float)"):
			return sizeof(Vec1f);
		case FLAME_CHASH("Vec(2+float)"):
			return sizeof(Vec2f);
		case FLAME_CHASH("Vec(3+float)"):
			return sizeof(Vec3f);
		case FLAME_CHASH("Vec(4+float)"):
			return sizeof(Vec4f);
		case FLAME_CHASH("uchar"):
			return sizeof(uchar);
		case FLAME_CHASH("Vec(1+uchar)"):
			return sizeof(Vec1c);
		case FLAME_CHASH("Vec(2+uchar)"):
			return sizeof(Vec2c);
		case FLAME_CHASH("Vec(3+uchar)"):
			return sizeof(Vec3c);
		case FLAME_CHASH("Vec(4+uchar)"):
			return sizeof(Vec4c);
		case FLAME_CHASH("StringA"):
			return sizeof(StringA);
		case FLAME_CHASH("StringW"):
			return sizeof(StringW);
		default:
			assert(0);
		}
	}

	inline void data_copy(uint type_hash, const void* src, void* dst, uint size = 0)
	{
		switch (type_hash)
		{
		case FLAME_CHASH("StringA"):
			*(StringA*)dst = *(StringA*)src;
			return;
		case FLAME_CHASH("StringW"):
			*(StringW*)dst = *(StringW*)src;
			return;
		}

		memcpy(dst, src, size ? size : data_size(type_hash));
	}

	inline void data_dtor(uint type_hash, void* p)
	{
		switch (type_hash)
		{
		case FLAME_CHASH("StringA"):
			((StringA*)p)->~String();
			return;
		case FLAME_CHASH("StringW"):
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
			case FLAME_CHASH("bool"):
				return *(bool*)src ? "1" : "0";
			case FLAME_CHASH("int"):
				return std::to_string(*(int*)src);
			case FLAME_CHASH("Vec(1+int)"):
				return to_string(*(Vec1i*)src);
			case FLAME_CHASH("Vec(2+int)"):
				return to_string(*(Vec2i*)src);
			case FLAME_CHASH("Vec(3+int)"):
				return to_string(*(Vec3i*)src);
			case FLAME_CHASH("Vec(4+int)"):
				return to_string(*(Vec4i*)src);
			case FLAME_CHASH("uint"):
				return std::to_string(*(uint*)src);
			case FLAME_CHASH("Vec(1+uint)"):
				return to_string(*(Vec1u*)src);
			case FLAME_CHASH("Vec(2+uint)"):
				return to_string(*(Vec2u*)src);
			case FLAME_CHASH("Vec(3+uint)"):
				return to_string(*(Vec3u*)src);
			case FLAME_CHASH("Vec(4+uint)"):
				return to_string(*(Vec4u*)src);
			case FLAME_CHASH("ulonglong"):
				return std::to_string(*(ulonglong*)src);
			case FLAME_CHASH("float"):
				return to_string(*(float*)src, precision);
			case FLAME_CHASH("Vec(1+float)"):
				return to_string(*(Vec1f*)src, precision);
			case FLAME_CHASH("Vec(2+float)"):
				return to_string(*(Vec2f*)src, precision);
			case FLAME_CHASH("Vec(3+float)"):
				return to_string(*(Vec3f*)src, precision);
			case FLAME_CHASH("Vec(4+float)"):
				return to_string(*(Vec4f*)src, precision);
			case FLAME_CHASH("uchar"):
				return std::to_string(*(uchar*)src);
			case FLAME_CHASH("Vec(1+uchar)"):
				return to_string(*(Vec1c*)src);
			case FLAME_CHASH("Vec(2+uchar)"):
				return to_string(*(Vec2c*)src);
			case FLAME_CHASH("Vec(3+uchar)"):
				return to_string(*(Vec3c*)src);
			case FLAME_CHASH("Vec(4+uchar)"):
				return to_string(*(Vec4c*)src);
			case FLAME_CHASH("std::string"):
				return *(std::string*)src;
			case FLAME_CHASH("std::wstring"):
				return w2s(*(std::wstring*)src);
			case FLAME_CHASH("StringA"):
				return ((StringA*)src)->str();
			case FLAME_CHASH("StringW"):
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
			case FLAME_CHASH("bool"):
				*(bool*)dst = (src != "0");
				break;
			case FLAME_CHASH("int"):
				*(int*)dst = std::stoi(src);
				break;
			case FLAME_CHASH("Vec(1+int)"):
				*(Vec1u*)dst = std::stoi(src.c_str());
				break;
			case FLAME_CHASH("Vec(2+int)"):
				*(Vec2u*)dst = stoi2(src.c_str());
				break;
			case FLAME_CHASH("Vec(3+int)"):
				*(Vec3u*)dst = stoi3(src.c_str());
				break;
			case FLAME_CHASH("Vec(4+int)"):
				*(Vec4u*)dst = stoi4(src.c_str());
				break;
			case FLAME_CHASH("uint"):
				*(uint*)dst = std::stoul(src);
				break;
			case FLAME_CHASH("Vec(1+uint)"):
				*(Vec1u*)dst = std::stoul(src.c_str());
				break;
			case FLAME_CHASH("Vec(2+uint)"):
				*(Vec2u*)dst = stou2(src.c_str());
				break;
			case FLAME_CHASH("Vec(3+uint)"):
				*(Vec3u*)dst = stou3(src.c_str());
				break;
			case FLAME_CHASH("Vec(4+uint)"):
				*(Vec4u*)dst = stou4(src.c_str());
				break;
			case FLAME_CHASH("ulonglong"):
				*(ulonglong*)dst = std::stoull(src);
				break;
			case FLAME_CHASH("float"):
				*(float*)dst = std::stof(src.c_str());
				break;
			case FLAME_CHASH("Vec(1+float)"):
				*(Vec1f*)dst = std::stof(src.c_str());
				break;
			case FLAME_CHASH("Vec(2+float)"):
				*(Vec2f*)dst = stof2(src.c_str());
				break;
			case FLAME_CHASH("Vec(3+float)"):
				*(Vec3f*)dst = stof3(src.c_str());
				break;
			case FLAME_CHASH("Vec(4+float)"):
				*(Vec4f*)dst = stof4(src.c_str());
				break;
			case FLAME_CHASH("uchar"):
				*(uchar*)dst = std::stoul(src);
				break;
			case FLAME_CHASH("Vec(1+uchar)"):
				*(Vec1c*)dst = std::stoul(src.c_str());
				break;
			case FLAME_CHASH("Vec(2+uchar)"):
				*(Vec2c*)dst = stoc2(src.c_str());
				break;
			case FLAME_CHASH("Vec(3+uchar)"):
				*(Vec3c*)dst = stoc3(src.c_str());
				break;
			case FLAME_CHASH("Vec(4+uchar)"):
				*(Vec4c*)dst = stoc4(src.c_str());
				break;
			case FLAME_CHASH("StringA"):
				*(StringA*)dst = src;
				break;
			case FLAME_CHASH("StringW"):
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
}

