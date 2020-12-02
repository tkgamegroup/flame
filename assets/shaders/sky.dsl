#ifdef MAKE_DSL
#define SKY_SET 0
#endif

layout (set = SKY_SET, binding = 0) uniform samplerCube sky_box;
layout (set = SKY_SET, binding = 1) uniform samplerCube sky_irr;
layout (set = SKY_SET, binding = 2) uniform samplerCube sky_rad;
