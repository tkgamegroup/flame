#include <flame/foundation/foundation.h>
#include <flame/foundation/system.h>
#include <flame/foundation/typeinfo.h>
#include <flame/graphics/noise.h>

using namespace flame;

int main(int argc, char** args) 
{
	for (auto x = 0; x < 10; x++)
	{
		for (auto y = 0; y < 10; y++)
		{
			auto uv = vec2(x / 100.f, y / 100.f);
			auto v = graphics::perlin_noise(uv);
			printf("uv: %s, %f\n", str(uv).c_str(), v);
		}
	}
	return 0;
}

