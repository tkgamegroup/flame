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

		virtual void add_alpha_channel() = 0;
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
		Vec2u pos;
		Vec2u size;
		std::unique_ptr<BinPackNode> right;
		std::unique_ptr<BinPackNode> bottom;

		BinPackNode(const Vec2u& size) :
			used(false),
			pos(0),
			size(size)
		{
		}

		BinPackNode* find(const Vec2u& _size)
		{
			if (!used && size >= _size)
			{
				used = true;
				right.reset(new BinPackNode(Vec2u(size.x() - _size.x(), _size.y())));
				right->pos = pos + Vec2u(_size.x(), 0);
				bottom.reset(new BinPackNode(Vec2u(size.x(), size.y() - _size.y())));
				bottom->pos = pos + Vec2u(0, _size.y());
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
		Bitmap* b;
		Vec2i pos;
	};

	inline void bin_pack(const Vec2u& size, const std::vector<std::filesystem::path>& inputs, const std::filesystem::path& output, bool border, const std::function<void(const std::vector<BinPackTile>& tiles)>& callback)
	{
		auto b1 = border ? 1U : 0U;
		auto b2 = b1 << 1;

		std::vector<BinPackTile> tiles;
		for (auto& i : inputs)
		{
			BinPackTile t;
			t.id = i.filename().stem().string();
			t.b = Bitmap::create(i.c_str());
			t.pos = Vec2i(-1);
			tiles.push_back(t);
		}
		std::sort(tiles.begin(), tiles.end(), [](const auto& a, const auto& b) {
			return max(a.b->get_width(), a.b->get_height()) > max(b.b->get_width(), b.b->get_height());
		});

		auto tree = std::make_unique<BinPackNode>(size);

		for (auto& t : tiles)
		{
			auto n = tree->find(Vec2u(t.b->get_width(), t.b->get_height()) + b2);
			if (n)
				t.pos = n->pos;
		}

		auto _size = Vec2u(0);
		for (auto& t : tiles)
		{
			_size.x() = max(t.pos.x() + t.b->get_width() + b1, _size.x());
			_size.y() = max(t.pos.y() + t.b->get_height() + b1, _size.y());
		}

		auto b = Bitmap::create(_size.x(), _size.y(), 4);
		for (auto& t : tiles)
		{
			if (t.pos >= 0)
				t.b->copy_to(b, t.b->get_width(), t.b->get_height(), 0, 0, t.pos.x(), t.pos.y(), border);
		}

		b->save(output.c_str());

		callback(tiles);

		for (auto& t : tiles)
			t.b->release();
	}
}
