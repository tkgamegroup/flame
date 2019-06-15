#pragma once

#include <flame/graphics/image.h>
#include <flame/UI/widget.h>

struct Tag;

struct Pic
{
	std::wstring filename;
	std::vector<Tag*> tags;
	flame::graphics::Image *img_thumbnail;
	flame::UI::Image *w_img;

	Pic();
	bool has_tag(Tag *tag);
	void remove_tag(Tag *tag);
	void get_tags_from_filename();
	void make_filename_from_tags();
};

void delete_pic(Pic *p);

extern std::vector<std::unique_ptr<Pic>> pics;

void load_pics();
