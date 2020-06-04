#include <flame/serialize.h>
#include <flame/foundation/bitmap.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

namespace flame
{
	struct BitmapPrivate : Bitmap
	{
		uint width;
		uint height;
		uint channel;
		uint byte_per_channel;
		uint pitch;
		uchar* data;
		uint size;
		bool srgb;

		~BitmapPrivate() 
		{ 
			delete[] data; 
		}

		void release() override { delete this; }
		uint get_width() const override { return width; }
		uint get_height() const override { return height; }
		uint get_channel() const override { return channel; }
		uint get_byte_per_channel() const override { return byte_per_channel; }
		uint get_pitch() const override { return pitch; }
		uchar* get_data() const override { return data; }
		uint get_size() const override { return size; }
		bool get_srgb() const override { return srgb; }

		void add_alpha_channel() override
		{
			assert(channel == 3 && byte_per_channel == 1);
			if (channel != 3 || byte_per_channel != 1)
				return;

			auto new_data = new uchar[width * height * 4];
			auto dst = new_data;
			for (auto j = 0; j < height; j++)
			{
				auto src = data + j * pitch;
				for (auto i = 0; i < width; i++)
				{
					*dst++ = *src++;
					*dst++ = *src++;
					*dst++ = *src++;
					*dst++ = 255;
				}
			}
			channel = 4;
			pitch = image_pitch(width * channel * byte_per_channel);
			size = pitch * height;
			delete[]data;
			data = new_data;
		}

		void Bitmap::swap_channel(uint ch1, uint ch2)
		{
			assert(byte_per_channel == 1 && ch1 < channel && ch2 < channel);
			if (byte_per_channel != 1 || ch1 >= channel || ch2 >= channel)
				return;

			for (auto j = 0; j < height; j++)
			{
				auto line = data + j * pitch;
				for (auto i = 0; i < width; i++)
				{
					auto p = line + i * channel;
					std::swap(p[ch1], p[ch2]);
				}
			}
		}

		void copy_to(Bitmap* _dst, uint w, uint h, uint src_x, uint src_y, uint _dst_x, uint _dst_y, bool border) override
		{
			auto dst = (BitmapPrivate*)_dst;
			auto b1 = border ? 1 : 0;
			auto b2 = b1 << 1;
			assert(byte_per_channel == 1 &&
				channel == dst->channel &&
				byte_per_channel == dst->byte_per_channel &&
				src_x + w < width&& src_y + h < height&&
				_dst_x + w + b2 < dst->width && _dst_y + h + b2 < dst->height);
			if (byte_per_channel != 1 ||
				channel != dst->channel ||
				byte_per_channel != dst->byte_per_channel ||
				src_x + w >= width || src_y + h >= height ||
				_dst_x + w + b2 >= dst->width || _dst_y + h + b2 >= dst->height)
				return;

			auto dst_x = _dst_x + b1;
			auto dst_y = _dst_y + b2;
			auto dst_data = dst->data;
			auto dst_pitch = dst->pitch;
			for (auto i = 0; i < h; i++)
			{
				auto src_line = data + (src_y + i) * pitch + src_x * channel;
				auto dst_line = dst_data + (dst_y + i) * dst_pitch + dst_x * channel;
				memcpy(dst_line, src_line, w * channel);
			}

			if (border)
			{
				memcpy(dst->data + (dst_y - 1) * dst_pitch + dst_x * channel, data + src_y * pitch + src_x * channel, w * channel); // top line
				memcpy(dst->data + (dst_y + h) * dst_pitch + dst_x * channel, data + (src_y + h - 1) * pitch + src_x * channel, w * channel); // bottom line
				for (auto i = 0; i < h; i++)
					memcpy(dst->data + (dst_y + i) * dst_pitch + (dst_x - 1) * channel, data + (src_y + i) * pitch + src_x * channel, channel); // left line
				for (auto i = 0; i < h; i++)
					memcpy(dst->data + (dst_y + i) * dst_pitch + (dst_x + w) * channel, data + (src_y + i) * pitch + (src_x + w - 1) * channel, channel); // left line

				memcpy(dst->data + (dst_y - 1) * dst_pitch + (dst_x - 1) * channel, data + src_y * pitch + src_x * channel, channel); // left top corner
				memcpy(dst->data + (dst_y - 1) * dst_pitch + (dst_x + w) * channel, data + src_y * pitch + (src_x + w - 1) * channel, channel); // right top corner
				memcpy(dst->data + (dst_y + h) * dst_pitch + (dst_x - 1) * channel, data + (src_y + h - 1) * pitch + src_x * channel, channel); // left bottom corner
				memcpy(dst->data + (dst_y + h) * dst_pitch + (dst_x + w) * channel, data + (src_y + h - 1) * pitch + (src_x + w - 1) * channel, channel); // right bottom corner
			}
		}

		void save_to_file(const wchar_t* filename) override
		{
			auto ext = std::filesystem::path(filename).extension();

			if (ext == L".png")
				stbi_write_png(w2s(filename).c_str(), width, height, channel, data, pitch);
			else if (ext == L".bmp")
				stbi_write_bmp(w2s(filename).c_str(), width, height, channel, data);
		}
	};

	Bitmap* Bitmap::create(uint width, uint height, uint channel, uint byte_per_channel, uchar* data)
	{
		auto b = new BitmapPrivate;
		b->width = width;
		b->height = height;
		b->channel = channel;
		b->byte_per_channel = byte_per_channel;
		auto pitch = image_pitch(width * channel * byte_per_channel);
		b->pitch = pitch;
		auto size = pitch * height;
		b->size = size;
		b->data = new uchar[size];
		if (!data)
			memset(b->data, 0, size);
		else
			memcpy(b->data, data, size);
		b->srgb = false;
		return b;
	}

	Bitmap* Bitmap::create_from_file(const wchar_t* filename)
	{
		auto file = _wfopen(filename, L"rb");
		if (!file)
			return nullptr;

		int cx, cy, channel;
		auto data = stbi_load_from_file(file, &cx, &cy, &channel, 4);
		if (!data)
			return nullptr;
		auto b = Bitmap::create(cx, cy, 4, 1, data);
		stbi_image_free(data);
		fclose(file);
		return b;
	}

	void pack_atlas(uint input_count, const wchar_t* const* inputs, const wchar_t* output, bool border)
	{
		auto output_path = std::filesystem::path(output);

		struct Tile
		{
			std::string id;
			Bitmap* b;
			Vec2i pos;
		};
		std::vector<Tile> tiles;
		for (auto i = 0; i < input_count; i++)
		{
			Tile t;
			t.id = std::filesystem::path(inputs[i]).filename().string();
			t.b = Bitmap::create_from_file(inputs[i]);
			t.pos = Vec2i(-1);
			tiles.push_back(t);
		}
		std::sort(tiles.begin(), tiles.end(), [](const Tile& a, const Tile& b) {
			return max(a.b->size.x(), a.b->size.y()) > max(b.b->size.x(), b.b->size.y());
		});

		auto size = Vec2u(512);
		auto tree = std::make_unique<BinPackNode>(size);

		for (auto& t : tiles)
		{
			auto n = tree->find(t.b->size + Vec2i(border ? 2 : 0));
			if (n)
				t.pos = n->pos;
		}

		size = 0;
		for (auto& t : tiles)
		{
			size.x() = max(t.pos.x() + t.b->size.x() + (border ? 1 : 0), size.x());
			size.y() = max(t.pos.y() + t.b->size.y() + (border ? 1 : 0) + 1, size.y());
		}

		auto b = Bitmap::create(size, 4, 32);
		for (auto& t : tiles)
		{
			if (t.pos >= 0)
				t.b->copy_to(b, Vec2u(0), t.b->size, Vec2u(t.pos), border);
		}

		Bitmap::save_to_file(b, output_path.c_str());

		auto atlas_path = output_path;
		atlas_path.replace_extension(L".atlas");
		std::ofstream atlas_file(atlas_path);
		atlas_file << "image = \"" << output_path.filename().string() << "\"\n";
		atlas_file << "border = " << (border ? "1" : "0") << "\n";
		atlas_file << "\n[tiles]\n";
		for (auto& t : tiles)
		{
			atlas_file << t.id + " " + to_string(Vec4u(Vec2u(t.pos) + (border ? 1U : 0U), t.b->size)) + "\n";
			Bitmap::destroy(t.b);
		}
		atlas_file.close();
	}
}
