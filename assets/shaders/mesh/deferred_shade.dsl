#ifdef MAKE_DSL
#define DEFERRED_SHADE_SET 0
#endif

layout (set = DEFERRED_SHADE_SET, binding = 0) uniform sampler2D image0;
layout (set = DEFERRED_SHADE_SET, binding = 1) uniform sampler2D image1;
