#pragma once

#include "image.h"

namespace flame
{
	namespace graphics
	{
		enum TextureType
		{
			TextureImage,
			TexturePerlin,
			TextureVoronoi
		};

		struct Texture
		{
			TextureType type = TextureImage;
			ImagePtr image = nullptr;
		};

		template<typename T>
		inline T triplanar_sample(const vec3& normal, const vec3& coord, 
			const std::function<T(const vec2& uv)>& sampler)
		{
			T ret = T(0);
			vec3 blending = abs(normal);
			blending = normalize(max(blending, vec3(0.00001f)));
			blending /= blending.x + blending.y + blending.z;
			if (blending.x > 0)
			{
				vec2 uv = coord.yz;
				ret += sampler(uv) * blending.x;
			}
			if (blending.y > 0)
			{
				vec2 uv = coord.xz;
				ret += sampler(uv) * blending.y;
			}
			if (blending.z > 0)
			{
				vec2 uv = coord.xy;
				ret += sampler(uv) * blending.z;
			}
			return ret;
		}
	}
}