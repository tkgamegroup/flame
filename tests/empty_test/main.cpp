#include <flame/math.h>

int main(int argc, char** args) 
{
	auto get_quat = [](const vec3& e)->quat {
		auto make_quat = [&](int _1, int _2, int _3) {
			vec3 axis[3] = {
				vec3(1.f, 0.f, 0.f),
				vec3(0.f, 1.f, 0.f),
				vec3(0.f, 0.f, 1.f)
			};

			return
				angleAxis(radians((float)e[_3]), axis[_3])
				* angleAxis(radians((float)e[_2]), axis[_2])
				* angleAxis(radians((float)e[_1]), axis[_1]);
		};

		return make_quat(0, 1, 2);
	};

	auto wtf = get_quat(vec3(55.5390549f, -48.7939224f, -0.494006157));

	return 0;
}

