#include "../../graphics/image.h"
#include "../../graphics/canvas.h"
#include "element_private.h"
#include "movie_private.h"

namespace flame
{
	cMoviePrivate::~cMoviePrivate()
	{
		element->drawers.remove("movie"_h);
	}

	void cMoviePrivate::on_init()
	{
		element->drawers.add([this](graphics::CanvasPtr canvas) {
			if (!images.empty())
			{
				if (play_index != -1)
				{
					auto& image = images[play_index];
					if (!element->tilted)
						canvas->draw_image(image.view, element->global_pos0(), element->global_pos1(), image.uvs, tint_col);
					else
					{
						vec2 pts[4];
						element->fill_pts(pts);
						vec2 uvs[4];
						uvs[0] = vec2(image.uvs.x, image.uvs.y);
						uvs[1] = vec2(image.uvs.z, image.uvs.y);
						uvs[2] = vec2(image.uvs.z, image.uvs.w);
						uvs[3] = vec2(image.uvs.x, image.uvs.w);
						canvas->draw_image_polygon(image.view, vec2(0.f), pts, uvs, tint_col);
					}

					time += delta_time;
					if (time >= speed)
					{
						time = 0.f;
						play_index++;
						if (play_index >= images.size())
							play_index = 0;
					}
				}
			}
		}, "movie"_h);
	}

	void cMoviePrivate::set_tint_col(const cvec4& col)
	{
		if (tint_col == col)
			return;
		tint_col = col;
		element->mark_drawing_dirty();
		data_changed("tint_col"_h);
	}

	struct cMovieCreate : cMovie::Create
	{
		cMoviePtr operator()(EntityPtr) override
		{
			return new cMoviePrivate();
		}
	}cMovie_create;
	cMovie::Create& cMovie::create = cMovie_create;
}
