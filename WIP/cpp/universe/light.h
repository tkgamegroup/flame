#include <flame/math.h>
namespace flame
{
	namespace ThreeDWorld
	{
		struct PointLight
		{
			Vec3 pos;
			Vec3 color;

			long long last_show_frame[2];
			int show_idx;
		};
	}
}
