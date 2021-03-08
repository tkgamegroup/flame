#pragma once

#include <flame/serialize.h>
#include <flame/foundation/foundation.h>

namespace flame
{
	struct Bitmap
	{
		virtual void release() = 0;

		virtual uint get_width() const = 0;
		virtual uint get_height() const = 0;
		virtual uint get_channel() const = 0;
		virtual uint get_byte_per_channel() const = 0;
		virtual uint get_pitch() const = 0;
		virtual uchar* get_data() const = 0;
		virtual uint get_size() const = 0;
		virtual bool get_srgb() const = 0;

		virtual void swap_channel(uint ch1, uint ch2) = 0;
		virtual void copy_to(Bitmap* dst, uint width, uint height, uint src_x = 0, uint src_y = 0, uint dst_x = 0, uint dst_y = 0, bool border = false) = 0;
		virtual void srgb_to_linear() = 0;
		virtual void save(const wchar_t* filename) = 0;

		FLAME_FOUNDATION_EXPORTS static Bitmap* create(uint width, uint height, uint channel = 4, uint byte_per_channel = 1, uchar* data = nullptr);
		FLAME_FOUNDATION_EXPORTS static Bitmap* create(const wchar_t* filename);
	};

	struct BinPackNode
	{
		bool used;
		uvec2 pos;
		uvec2 size;
		std::unique_ptr<BinPackNode> right;
		std::unique_ptr<BinPackNode> bottom;

		BinPackNode(const uvec2& size) :
			used(false),
			pos(0),
			size(size)
		{
		}

		BinPackNode* find(const uvec2& _size)
		{
			if (!used && size.x >= _size.x && size.y >= size.y)
			{
				used = true;
				right.reset(new BinPackNode(uvec2(size.x - _size.x, _size.y)));
				right->pos = pos + uvec2(_size.x, 0);
				bottom.reset(new BinPackNode(uvec2(size.x, size.y - _size.y)));
				bottom->pos = pos + uvec2(0, _size.y);
				return this;
			}
			if (!right || !bottom)
				return nullptr;
			auto n1 = right->find(_size);
			if (n1)
				return n1;
			auto n2 = bottom->find(_size);
			if (n2)
				return n2;
			return nullptr;
		}
	};

	struct BinPackTile
	{
		std::string id;
		FlmPtr<Bitmap> bmp;
		ivec2 pos;
	};

	inline void bin_pack(const uvec2& size, const std::vector<std::filesystem::path>& inputs, const std::filesystem::path& output, bool border, const std::function<void(const std::vector<BinPackTile>& tiles)>& callback)
	{
		auto b1 = border ? 1U : 0U;
		auto b2 = b1 << 1;

		std::vector<BinPackTile> tiles;
		for (auto& i : inputs)
		{
			BinPackTile t;
			t.id = i.filename().stem().string();
			t.bmp.reset(Bitmap::create(i.c_str()));
			t.pos = ivec2(-1);
			tiles.push_back(std::move(t));
		}
		std::sort(tiles.begin(), tiles.end(), [](const auto& a, const auto& b) {
			return max(a.bmp->get_width(), a.bmp->get_height()) > max(b.bmp->get_width(), b.bmp->get_height());
		});

		auto tree = std::make_unique<BinPackNode>(size);

		for (auto& t : tiles)
		{
			auto n = tree->find(uvec2(t.bmp->get_width(), t.bmp->get_height()) + b2);
			if (n)
				t.pos = n->pos;
		}

		auto _size = uvec2(0);
		for (auto& t : tiles)
		{
			_size.x = max(t.pos.x + t.bmp->get_width() + b1, _size.x);
			_size.y = max(t.pos.y + t.bmp->get_height() + b1, _size.y);
		}

		auto b = Bitmap::create(_size.x, _size.y, 4);
		for (auto& t : tiles)
		{
			if (t.pos.x >= 0 && t.pos.y >= 0)
				t.bmp->copy_to(b, t.bmp->get_width(), t.bmp->get_height(), 0, 0, t.pos.x, t.pos.y, border);
		}

		b->save(output.c_str());

		callback(tiles);
	}
}
