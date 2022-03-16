#define SAMPLE_COUNT 64
#define NOISE_CX 4
#define NOISE_CY 4

layout (set = SET, binding = 0) uniform SampleLocations
{
	vec4 sample_locations[SAMPLE_COUNT];
};

layout (set = SET, binding = 1) uniform SampleNoises
{
	vec4 sample_noises[NOISE_CX * NOISE_CY];
};

layout (set = SET, binding = 2) uniform sampler2D img_nor_rou;
layout (set = SET, binding = 3) uniform sampler2D img_dep;
