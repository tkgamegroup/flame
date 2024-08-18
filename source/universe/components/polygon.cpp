#include "../../graphics/image.h"
#include "../../graphics/canvas.h"
#include "element_private.h"
#include "polygon_private.h"

namespace flame
{
	cPolygonPrivate::~cPolygonPrivate()
	{
		element->drawers.remove("polygon"_h);
	}

	void cPolygonPrivate::on_init()
	{
		element->drawers.add([this](graphics::CanvasPtr canvas) {
			if (image)
			{
				auto lvs = 1U;
				if (sampler && sampler->linear_mipmap)
					lvs = image->n_levels;
				canvas->draw_image_polygon(image->get_view({ 0, lvs, 0, 1 }), element->global_pos(), { pts, num_pts }, { uvs, num_pts }, color, sampler);
			}
		}, "polygon"_h);
	}

	struct cPolygonCreate : cPolygon::Create
	{
		cPolygonPtr operator()(EntityPtr) override
		{
			return new cPolygonPrivate();
		}
	}cPolygon_create;
	cPolygon::Create& cPolygon::create = cPolygon_create;
}
