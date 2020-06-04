#pragma once

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
		virtual void save_to_file(const wchar_t* filename) = 0;

		FLAME_FOUNDATION_EXPORTS static Bitmap* create(uint width, uint height, uint channel, uint bype_per_channel = 1, uchar* data = nullptr);
		FLAME_FOUNDATION_EXPORTS static Bitmap* create_from_file(const wchar_t* filename);
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

	FLAME_FOUNDATION_EXPORTS void pack_atlas(uint input_count, const wchar_t* const* inputs, const wchar_t* output, bool border);
}
