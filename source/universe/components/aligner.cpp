#include <flame/universe/components/element.h>
#include <flame/universe/components/aligner.h>

namespace flame
{
	struct cAlignerPrivate: cAligner
	{
		cAlignerPrivate()
		{
			element = nullptr;

			x_align_ = AlignxFree;
			y_align_ = AlignyFree;
			width_policy_ = SizeFixed;
			min_width_ = -1.f;
			width_factor_ = 1.f;
			height_policy_ = SizeFixed;
			min_height_ = -1.f;
			height_factor_ = 1.f;
			using_padding_ = false;
		}

		void on_component_added(Component* c) override
		{
			if (c->name_hash == cH("cElement"))
			{
				element = (cElement*)c;
				if (min_width_ < 0.f && width_policy_ == SizeGreedy)
					min_width_ = element->size_.x();
				if (min_height_ < 0.f && height_policy_ == SizeGreedy)
					min_height_ = element->size_.y();
			}
		}

		Component* copy() override
		{
			auto copy = new cAlignerPrivate();

			copy->x_align_ = x_align_;
			copy->y_align_ = y_align_;
			copy->width_policy_ = width_policy_;
			copy->min_width_ = min_width_;
			copy->width_factor_ = width_factor_;
			copy->height_policy_ = height_policy_;
			copy->min_height_ = min_height_;
			copy->height_factor_ = height_factor_;
			copy->using_padding_ = using_padding_;

			return copy;
		}
	};

	cAligner* cAligner::create()
	{
		return new cAlignerPrivate();
	}

	void cAligner::set_x_align(Alignx a, void* sender)
	{
		if (a == x_align_)
			return;
		x_align_ = a;
		data_changed(cH("x_align"), sender);
	}

	void cAligner::set_y_align(Aligny a, void* sender)
	{
		if (a == y_align_)
			return;
		y_align_ = a;
		data_changed(cH("y_align"), sender);
	}

	void cAligner::set_width_policy(SizePolicy p, void* sender)
	{
		if (p == width_policy_)
			return;
		width_policy_ = p;
		data_changed(cH("width_policy"), sender);
	}

	void cAligner::set_min_width(float w, void* sender)
	{
		if (w == min_width_)
			return;
		min_width_ = w;
		data_changed(cH("min_width"), sender);
	}

	void cAligner::set_width_factor(float f, void* sender)
	{
		if (f == width_factor_)
			return;
		width_factor_ = f;
		data_changed(cH("width_factor"), sender);
	}

	void cAligner::set_height_policy(SizePolicy p, void* sender)
	{
		if (p == height_policy_)
			return;
		height_policy_ = p;
		data_changed(cH("height_policy"), sender);
	}

	void cAligner::set_min_height(float h, void* sender)
	{
		if (h == min_height_)
			return;
		min_height_ = h;
		data_changed(cH("min_height"), sender);
	}

	void cAligner::set_height_factor(float f, void* sender)
	{
		if (f == height_factor_)
			return;
		height_factor_ = f;
		data_changed(cH("height_factor"), sender);
	}

	void cAligner::set_using_padding(bool v, void* sender)
	{
		if (v == using_padding_)
			return;
		using_padding_ = v;
		data_changed(cH("using_padding"), sender);
	}

}
